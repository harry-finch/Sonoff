#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "ArduinoOTA.h"

// Définition des entrées/sorties
#define PIN_LED 13
#define PIN_RELAI 12
#define PIN_BOUTON 0

// Adresse IP partielle du module
#define IP_MODULE 254

// Page html
const char index_html[] PROGMEM = R"=====(
<!doctype html>
<html lang="fr">
    <head>
        <meta charset="utf-8">
        <meta name="viewport" content="width=device-width, initial-scale=1">
        <title>Relais SONOFF</title>
        <style>
          body {
            padding: 10px;
            color: #DDDDDD;
            background-color: #333333;
          }
          h1 { font: 2em Arial; }

          .bouton {
              font: 3em Arial; color: #DDDDDD; text-align: center; text-decoration: none;
              background-color: blueviolet;
              border: 1px; border-radius: 3px;
              padding: 5px 0px; display: inline-block; margin: 4px 2px;
              cursor: pointer;
              width: 100%;
          }
        </style>
    </head>
    <body>
        <h1 id="etatSonoffBasic">Etat du module : %ETAT_SONOFF%</h1>

        <form action="/switch%BTN_ACTION%">
          <input class="bouton" type="submit" value="%BTN_LABEL%">
        </form>
    </body>
</html>
)=====";

const char * SSID = "Livebox-7990";
const char * PASSWORD = "fHH5t5nYGhLvQJPK7v";

// Gestion des événements du WiFi
// Lorsque la connexion vient d'aboutir
void onConnected(const WiFiEventStationModeConnected& event);
// Lorsque l'adresse IP est attribuée
void onGotIP(const WiFiEventStationModeGotIP& event);

// Objet WebServer
ESP8266WebServer serverWeb(80);

// Fonctions du serveur Web
void handleRoot() {
  String temp(reinterpret_cast<const __FlashStringHelper *> (index_html));

  if (digitalRead(PIN_RELAI) == HIGH) {
    temp.replace("%ETAT_SONOFF%", "ON");
    temp.replace("%BTN_LABEL%", "Eteindre");
    temp.replace("%BTN_ACTION%", "Off");
  } else {
    temp.replace("%ETAT_SONOFF%", "OFF"); 
    temp.replace("%BTN_LABEL%", "Allumer");
    temp.replace("%BTN_ACTION%", "On");
  }

  serverWeb.send(200, "text/html", temp);
}

void switchOn() {
  digitalWrite(PIN_RELAI, HIGH);
  serverWeb.sendHeader("Location","/");
  serverWeb.send(303); 
}

void switchOff() {
  digitalWrite(PIN_RELAI, LOW);
  serverWeb.sendHeader("Location","/");
  serverWeb.send(303); 
}

void APOff() {
  WiFi.enableAP(false);
  serverWeb.send(200, "text/plain", "Access Point Off.");
}

void setup() {
  // Liste des IP pour une configuration en IP Fixe
  IPAddress ip(192, 168, 1, IP_MODULE);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 1, 1);

  // Mise en place d'une liaison série
  Serial.begin(9600L);
  delay(100);

  // Configuration des entrées/sorties
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RELAI, OUTPUT);
  pinMode(PIN_BOUTON, INPUT_PULLUP);

  // Module OFF par défaut
  digitalWrite(PIN_RELAI, LOW);

  // Mode de connexion
  WiFi.softAP("Module Sonoff Basic R2");
  WiFi.mode(WIFI_AP_STA);
 
  // Démarrage de la connexion
  WiFi.config(ip, gateway, subnet, dns);
  WiFi.begin(SSID, PASSWORD);

  static WiFiEventHandler onConnectedHandler = WiFi.onStationModeConnected(onConnected);
  static WiFiEventHandler onGotIPHandler = WiFi.onStationModeGotIP(onGotIP);

  // Mise en place du serveur WebServer
  serverWeb.on("/switchOn", switchOn);
  serverWeb.on("/switchOff", switchOff);
  serverWeb.on("/APOff", APOff);
  serverWeb.on("/", handleRoot);
  serverWeb.on("/index.html", handleRoot);
  serverWeb.begin();

  // Nom pour l'OTA
  ArduinoOTA.setHostname("Module Sonoff Basic R2");
}

void loop() {
  // Si l'objet est connecté au réseau, on effectue les tâches qui doivent l'être dans ce cas
  if (WiFi.isConnected()) {
    digitalWrite(PIN_LED, LOW); // LED active à l'état bas
    ArduinoOTA.begin();
  }
  else {
    digitalWrite(PIN_LED, HIGH);
  }
  ArduinoOTA.handle();
  serverWeb.handleClient();
}

void onConnected(const WiFiEventStationModeConnected& event) {
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
}

void onGotIP(const WiFiEventStationModeGotIP& event) {
  Serial.println("Adresse IP : " + WiFi.localIP().toString());
  Serial.println("Passerelle IP : " + WiFi.gatewayIP().toString());
  Serial.println("DNS IP : " + WiFi.dnsIP().toString());
  Serial.print("Puissance de réception : ");
  Serial.println(WiFi.RSSI());
}
