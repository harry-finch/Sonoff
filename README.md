# Sonoff hack

## The project

The original idea and tutorial for this project can be found [in this video](https://www.youtube.com/watch?v=y9pcBowAaXw) by [Eric Peronnin](https://www.youtube.com/channel/UCe3v5cVACw-5BKQOcwUaM8w) on how to hack the Sonoff relay modules.

The tutorial explains how to upload a new program to the Sonoff. The program creates a simple webserver you can then access on your phone's browser to control the relay switch.

You'll find the [original program here](http://geii.eu/index.php?option=com_content&view=article&id=244&Itemid=955).

## Revamp

I used the original program for a while but wanted to be able to control the module from anywhere, so I decided to switch to using a Telegram Bot. For this I used [Brian Lough's library](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) and one of its example files.

## Improvements

- Added a programmable timer command
