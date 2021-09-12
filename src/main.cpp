#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include "credentials.h"

const unsigned long BOT_MTBS = 1000;                            // mean time between scan messages
unsigned long bot_lasttime;                                     // last time messages' scan has been done

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 3600;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec);

const int ledPin = 13;
const int relayPin = 12;
int relayStatus = 0;

int startTimer = 8;
int endTimer = 22;
bool timerMode = false;

void handleNewMessages(int numNewMessages) {
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/on") {
      timerMode = false;
      digitalWrite(relayPin, HIGH);
      relayStatus = 1;
      bot.sendMessage(chat_id, "Light is ON", "");
    }

    if (text == "/off") {
      timerMode = false;
      relayStatus = 0;
      digitalWrite(relayPin, LOW); 
      bot.sendMessage(chat_id, "Light is OFF", "");
    }

    if (text == "/status") {
      if (relayStatus) bot.sendMessage(chat_id, "Light is ON", "");
      else bot.sendMessage(chat_id, "Light is OFF", "");

      if (timerMode) {
        bot.sendMessage(chat_id, "Timer is ON", "");
        char numstr[3];
        String timer = itoa(startTimer, numstr, 10);
        bot.sendMessage(chat_id, "Start time is: " + timer, "");
        timer = itoa(endTimer, numstr, 10);
        bot.sendMessage(chat_id, "End time is: " + timer, "");
      }
      else bot.sendMessage(chat_id, "Timer is OFF", "");
    }

    if(text.startsWith("/timer")) {
      int index1 = text.indexOf(' ');
      int index2 = text.indexOf(' ', index1+1);

      startTimer = text.substring(index1, index2).toInt();
      endTimer = text.substring(index2+1).toInt();
      timerMode = true;

      String message = "Timer is now set.\n\nLights will be on from " + text.substring(index1, index2) + " to " + text.substring(index2+1) + "\n";
      bot.sendMessage(chat_id, message, "");
    }

    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "/on : to switch the light ON\n";
      welcome += "/off : to switch the light OFF\n";
      welcome += "/status : returns current light status\n";
      welcome += "/timer [start time] [end time] : sets timer \n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  delay(10);
  digitalWrite(relayPin, LOW);
  digitalWrite(ledPin, HIGH);

  // attempt to connect to Wifi network:
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);       // get UTC time via NTP
  secured_client.setTrustAnchors(&cert);                          // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(ledPin, LOW);

  bot.sendMessage(CHAT_ID, "Sonoff connected", "");
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    timeClient.update();
    int currentTime = timeClient.getHours();
    
    if(timerMode && !relayStatus && (currentTime >= startTimer) && (currentTime < endTimer)) {
      digitalWrite(relayPin, HIGH); 
      relayStatus = 1;
      bot.sendMessage(CHAT_ID, "Light has now been turned ON by timer.", "");
    } else if(timerMode && relayStatus && (currentTime < startTimer) && (currentTime >= endTimer)) {
      digitalWrite(relayPin, LOW); 
      relayStatus = 0;
      bot.sendMessage(CHAT_ID, "Light has now been turned OFF by timer.", "");
    }

    bot_lasttime = millis();
  }
}