# Defuma IoT

## _An ESP8266 based smoker controller and data acquisition system_
[![ESP8266](https://img.shields.io/badge/ESP-8266-blue.svg)](https://github.com/esp8266/esp8266-wiki)
[![C++](https://img.shields.io/badge/C-++-blue.svg)]()
[![PlatformIO](https://img.shields.io/badge/Platform-IO-blue.svg)](https://platformio.org/)

![CHURRASQUEIRA CONTROLE REMOTO](/img/tapegandofogo.jpg "TÁ PEGANDO FOGO BICHO - CHURRASQUEIRA CONTROLE REMOTO")

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

## References

* [NodeMCU and ESP pinout and Setup](https://www.mischianti.org/2021/05/08/esp12-esp07-esp8266-flash-pinout-specs-and-arduino-ide-configuration-6/)
* [TM1637 Connection](https://www.electroniclinic.com/tm1637-a-4-digit-7-segment-display-with-arduino/)
* [Read MAX6675 with the SPI Library](https://arduinodiy.wordpress.com/2019/12/06/using-a-max6675-temperature-sensor-without-a-library/)
* [Buzzer with NodeMCU](https://www.geekering.com/categories/embedded-sytems/esp8266/ricardocarreira/esp8266-nodemcu-make-some-noise-with-buzzers/)
* [Control Threshold with an webserver](https://microcontrollerslab.com/esp32-esp8266-thermostat-web-server-control-output-temperature-threshold/)
* [Wifi Manager - Configure Wifi Connection](https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password/)
* [Async WifiManager](https://randomnerdtutorials.com/esp8266-nodemcu-wi-fi-manager-asyncwebserver/)
* [Plot Sensor Data](https://randomnerdtutorials.com/esp32-esp8266-plot-chart-web-server/)
