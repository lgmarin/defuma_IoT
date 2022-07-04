#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsync_WiFiManager.h>
#include <ESP8266mDNS.h>
#include <SPI.h>
#include <TM1637Display.h>

#include "wifi_mgr.h"
#include "read_temp.h"
#include "file_utils.h"

// USING GPIO PINS FOR ESP12 Compatibility!
// D# Pins correspond to nodeMCU V1.2 pins
// TM1637 4d Display Pins
// SCLK -> D2 - GPIO4
// DIO  -> D1 - GPIO5
int tm_DIO = 5;
int tm_CLK = 4;

TM1637Display display(tm_CLK, tm_DIO);

// Define main pins for MAX6675
// SCLK -> D5 - GPIO14
// MISO -> D6 - GPIO12
// CS   -> D7 - GPIO13
int max_CS = 13;
float temperature = 0;

// Buzzer PIN
// BUZZ -> D0 - GPIO16
int BUZZ = 16;

String temp_low = "20";
String temp_high = "26";
String last_temperature;

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

// Open WebServer connection at PORT 80
AsyncWebServer server(80);
DNSServer dnsServer;

// String processor to be used to parse data to the web browser
String processor(const String& var){
  if(var == "TEMPERATURE"){
    return last_temperature;
  }
  else if(var == "THRESHOLD_MAX"){
    return temp_high;
  }
  else if(var == "THRESHOLD_MIN"){
    return temp_low;
  }
  return String();
}

String config_processor(const String& var){
  if(var == "MODE"){
    switch (WiFi.getMode())
    {
    case WIFI_STA:
      return "Station";
      break;
    case WIFI_AP:
      return "Access Point";
      break;
    case WIFI_AP_STA:
      return "Access Point + Station";
      break;          
    default:
      return "Disconnected";
      break;
    }
  }
  else if(var == "SSID"){
    return WiFi.SSID();
  }
  else if(var == "IP"){
    return WiFi.localIP().toString();
  }
  return String();
}

// Configure time interval between readings - 1 second
unsigned long previousMillis = 0;
const long interval = 1000;

void setup(){
  Serial.begin(9600);
  delay(1000);

 //Intiate SPI transaction
  SPI.begin();

  // Init LittleFS
  initFS();

  // Prepare Display and Wait for MAX initialization
  uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setBrightness(0x0f);
  display.setSegments(data);

  //Set max_CS Pin as OUTPUT and set to HIGH to Intiate MAX 6675
  pinMode(max_CS, OUTPUT);
  digitalWrite(max_CS, HIGH);

  // Set Buzzer PIN as Output
  pinMode(BUZZ, OUTPUT);

 // AsynWifiManager Block BEGIN

  ESPAsync_WiFiManager ESPAsync_wifiManager(&server, &dnsServer, "defuma_iot");
  
  Serial.print("\n[INFO]: Verify if there is some saved credentials...");
  bool initialConfig = false;
  bool configDataLoaded = false;

  if (loadWifiCred())
  {
      configDataLoaded = true;
      ESPAsync_wifiManager.setConfigPortalTimeout(30);
      Serial.print(F("\n[INFO]:Got stored Credentials. Timeout 30s for Config Portal"));
  }
  else
  {
      // Enter CP only if no stored SSID on flash and file 
      Serial.print(F("\n[INFO]:Open Config Portal without Timeout: No stored Credentials."));
      initialConfig = true;
  }

  Serial.print("\n[INFO]:Starting configuration portal @ "); Serial.print("192.168.4.1");

  digitalWrite(LED_BUILTIN, LOW); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

  // Starts an access point
  if (!ESPAsync_wifiManager.startConfigPortal("defuma_IOT")) //(const char *) ssid.c_str())
    Serial.print(F("\n[INFO]: Configuration portal time out. No Wifi Connected..."));
  else
  {
    Serial.print(F("\n[INFO]: WiFi connected!"));
  }  

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print(F("\n[INFO]: Connected. Local IP: ")); Serial.print(WiFi.localIP());
  } else
  {
    Serial.print(F("\n[ERROR]: WiFi not connected, with status: ")); Serial.print(ESPAsync_wifiManager.getStatus(WiFi.status()));
    ESPAsync_wifiManager.startConfigPortal("defuma_IOT"); //(const char *) ssid.c_str())
  }

  // Only clear then save data if CP entered and with new valid Credentials
  if (String(ESPAsync_wifiManager.getSSID(0)) != "" && String(ESPAsync_wifiManager.getPW(0)) != "")
  {
    Serial.print(F("\n[INFO]: WiFi connected! Saving config..."));
    String SSID = ESPAsync_wifiManager.getSSID(0);
    String PW = ESPAsync_wifiManager.getPW(0);
    Serial.println(SSID);
    storeWifiCred(SSID, PW);   // Store data in struct      
    initialConfig = true;
  }

  digitalWrite(LED_BUILTIN, HIGH); // Turn led off as we are not in configuration mode.

  if (!initialConfig)
  {
    // Load stored data, the addAP ready for MultiWiFi reconnection
    if (!configDataLoaded)
      loadWifiCred();
    if ( WiFi.status() != WL_CONNECTED ) 
    {
      Serial.print(F("\n[INFO]: MultiWiFi Configuration..."));
      connectMultiWifi();
    }
  }

  if(loadThresholdConfig()) { //Load Initial configuration data
    temp_low = String(APP_config.temp_min);
    temp_high = String(APP_config.temp_max);
  }

  // AsynWifiManager Block END

  display.clear();

  // Configure Server Async calls
  server.serveStatic("/", LittleFS, "/");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html", false, processor);
  });

  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/config.html", "text/html", false, config_processor);
  });

  server.on("/delete-cfg", HTTP_GET, [](AsyncWebServerRequest *request){
    if(removeThresholdConfig())
      request->send_P(200, "text/plain", "success");
    request->send_P(200, "text/plain", "error");
  });

  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("threshold_max") && request->hasParam("threshold_min")) {
      temp_high = request->getParam("threshold_max")->value();
      temp_low = request->getParam("threshold_min")->value();
    }

    Serial.print("\n[INFO]: Set threshold_max:");
    Serial.print(temp_high);
    Serial.print("\n[INFO]: Set threshold_min:");
    Serial.print(temp_low);

    storeThresholdConfig(temp_high, temp_low);

    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(temperature).c_str());
  });

  server.onNotFound(notFound);
  server.begin();

  if (!MDNS.begin("defumaiot")) {
    Serial.print(F("\n[ERROR]: MultiWiFi Configuration..."));
  }
  // Add Web Server service to mDNS
  MDNS.addService("http", "tcp", 80);
  Serial.print(F("\n[INFO]: mDNS service started. Go to http://defumaiot.local"));
}

void loop(){
  // Use millis() to generate a counter and ditch delay()
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    //Read Temperature Data
    temperature = readTemperatureC(max_CS);

    if (temperature == NAN)
    {
      Serial.print(F("Error: No Thermocouple connected!"));
    } 
    else
    {
      display.showNumberDec(temperature);
    }

    // Keep track of the last temperature read
    last_temperature = String(temperature);
    
    if(temperature > temp_high.toFloat() && temperature != NAN){
      tone(BUZZ, 523, 800);
    }
    else if(temperature < temp_low.toFloat() && temperature != NAN){
      tone(BUZZ, 1000, 200);
    }
    else {
      noTone(BUZZ);
    }
  }

  MDNS.update();
}