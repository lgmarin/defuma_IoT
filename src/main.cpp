#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsync_WiFiManager.h>
#include <SPI.h>
#include <TM1637Display.h>

#include <read_temp.h>
#include <file_utils.h>
#include <wifi_mgr.h>

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

// Configure time interval between readings - 1 second
unsigned long previousMillis = 0;
const long interval = 1000;

void setup(){
  Serial.begin(9600);
  delay(200);

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

  ESPAsync_WiFiManager ESPAsync_wifiManager(&server, &dnsServer, "defuma_iot");
  //ESPAsync_wifiManager.resetSettings();   //reset saved settings
  
  Serial.println("Verify if there is some saved credentials...");
  bool initialConfig = false;
  bool configDataLoaded = false;

  if (loadConfigData())
  {
      configDataLoaded = true;
      ESPAsync_wifiManager.setConfigPortalTimeout(60);
      Serial.println(F("Got stored Credentials. Timeout 60s for Config Portal"));
  }
  else
  {
      // Enter CP only if no stored SSID on flash and file 
      Serial.println(F("Open Config Portal without Timeout: No stored Credentials."));
      initialConfig = true;
  }

  Serial.print(F("Starting configuration portal @ "));
  Serial.print(F("192.168.4.1"));

  digitalWrite(LED_BUILTIN, LED_ON); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

  // Starts an access point
  if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str()))
      Serial.println(F("Not connected to WiFi but continuing anyway."));
  else
  {
      Serial.println(F("WiFi connected...yeey :)"));
  }  

  if (WiFi.status() == WL_CONNECTED)
  {
      Serial.print(F("Connected. Local IP: "));
      Serial.println(WiFi.localIP());
  }
  else
  {
      Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));
      Serial.println("Can't connect! Entering WiFi config mode...");
      Serial.println("Restart board...");
      ESPAsync_wifiManager.startConfigPortal("defuma_iot");
  }

  // Only clear then save data if CP entered and with new valid Credentials
  if (String(ESPAsync_wifiManager.getSSID(0)) != "")
  {
    storeWifiCred();   // Store data in struct      
    //saveConfigData();

    initialConfig = true;
  }

  digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.

  if (!initialConfig)
  {
      // Load stored data, the addAP ready for MultiWiFi reconnection
      if (!configDataLoaded)


      for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
      {
        // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
        if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
        {
            LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
            wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
        }
      }

      if ( WiFi.status() != WL_CONNECTED ) 
      {
        Serial.println(F("ConnectMultiWiFi in setup"));
        connectMultiWiFi();
      }
  }  

  display.clear();

  // Configure Server Async calls
  server.serveStatic("/", LittleFS, "/");


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html", false, processor);
  });

  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    if (request->hasParam("threshold_max") && request->hasParam("threshold_min")) {
      temp_high = request->getParam("threshold_max")->value();
      temp_low = request->getParam("threshold_min")->value();
    }

    Serial.println("Set threshold_max");
    Serial.println(temp_high);
    Serial.println("Set threshold_min");
    Serial.println(temp_low);

    request->send(200, "text/html", "HTTP GET request sent to your ESP.<br><a href=\"/\">Return to Home Page</a>");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(temperature).c_str());
  });

  server.onNotFound(notFound);
  server.begin();    
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
      Serial.print("Error: No Thermocouple connected!");
    } 
    else
    {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
      display.showNumberDec(temperature);
    }

    // Keep track of the last temperature read
    last_temperature = String(temperature);
    
    if(temperature > temp_high.toFloat() && temperature != NAN){
      String message = String("Temperature is too high. Current temperature: ") + 
                            String(temperature) + String("C");
      Serial.println(message);
      tone(BUZZ, 523, 800);
    }
    else if(temperature < temp_low.toFloat() && temperature != NAN){
      String message = String("Temperature is too low. Current temperature: ") + 
                            String(temperature) + String(" C");
      Serial.println(message);
      tone(BUZZ, 1000, 200);
    }
    else {
      noTone(BUZZ);
    }
  }
}