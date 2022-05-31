# Defuma IoT

## _An ESP8266 based smoker controller and data acquisition system_
[![ESP8266](https://img.shields.io/badge/ESP-8266-blue.svg)](https://github.com/esp8266/esp8266-wiki)
[![C++](https://img.shields.io/badge/C-++-blue.svg)]()
[![PlatformIO](https://img.shields.io/badge/Platform-IO-blue.svg)](https://platformio.org/)

![CHURRASQUEIRA CONTROLE REMOTO](/imgs/tapegandofogo.jpg "TÁ PEGANDO FOGO BICHO - CHURRASQUEIRA CONTROLE REMOTO")

This software was developed with the intention to help to produce a delicious, tender and with an amazing smoked flavored meat! What better way than to make a Nerd happy than with some long smoking process with some friends, some beer and some technology involved, why not?

To acquire a good quality smoked meat, the amount of smoke produced, and mainly the temperature should be constantly controlled (temperature should not be too low and specially, not too high) during the long 12 hours (or more, depending on the cut) to succeed.

The idea here is to have a controller that will display the smoker temperature with a buzzer to signalize the low and high temperature drops. The base system will be developed using an ESP8266 board, that allows for a Wifi Access Point connection that will expose a Web Server that will also display the temperature and allow to define the temperature thresholds.


## Objectives

Development of a data monitoring for a meat smoker:

* Inclusion of a Display and Buzzer to notify the user
* Allow for WiFi connection to visualize the data using a smartphone
* Simple construction and low power
* Future upgrades may include a servo to control the air intake (thus, controlling the smoker temperature)

## Peripherals

* NodeMCU V1.2 ESP8266 Development Board
    * The final project will be built using a ESP12F board.
* MAX6675 Type K Thermocouple Module
* TM1637 4 Digit 8 Segment Display
* Passive Buzzer

## Libraries

* SPI (Arduino IDE Default Libraries) - For reading the MAX6675 data through the SPI Bus.
* TM1637 by Smougenot - For displaying data in the display.
