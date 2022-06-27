#include <wifi_mgr.h>
#include <file_utils.h>
#include <Arduino.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;

void StartWifiManager(){

// put your setup code here, to run once:
//     initialize the LED digital pin as an output.
//     pinMode(LED_BUILTIN, OUTPUT);

//     Serial.begin(115200);
//     while (!Serial);

//     delay(200);

//     Serial.print(F("\nStarting Async_ConfigOnStartup using ")); Serial.print(FS_Name);
//     Serial.print(F(" on ")); Serial.println(ARDUINO_BOARD);
//     Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);

//     Serial.setDebugOutput(false);

//     if (FORMAT_FILESYSTEM) 
//         FileFS.format();

//     Format FileFS if not yet
//     if (!FileFS.begin())
//         {
//         FileFS.format();

//         Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));

//         if (!FileFS.begin())
//         {     
//             prevents debug info from the library to hide err message.
//             delay(100);
//             Serial.println(F("LittleFS failed!. Please use SPIFFS or EEPROM. Stay forever"));
//             while (true)
//             {
//             delay(1);
//             }
//         }
//     }

    unsigned long startedAt = millis();

    //Local intialization. Once its business is done, there is no need to keep it around
    // AsyncWebServer webServer(HTTP_PORT);
    // DNSServer dnsServer;
    // ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "AsyncConfigOnStartup");    

    // Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
    bool initialConfig = false;

    //Check if there is stored WiFi router/password credentials.
    //If not found, device will remain in configuration mode until switched off via webserver.
    Serial.println(F("Opening configuration portal."));

    bool configDataLoaded = false;

    if (loadConfigData())
    {
        configDataLoaded = true;
        
        ESPAsync_wifiManager.setConfigPortalTimeout(60); //If no access point name has been previously entered disable timeout.
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

    // Only clear then save data if CP entered and with new valid Credentials
    // No CP => stored getSSID() = ""
    if (String(ESPAsync_wifiManager.getSSID(0)) != "")
    {
        storeWifiCred();   // Store data in struct      
        saveConfigData();

        initialConfig = true;
    }

    digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.

    startedAt = millis();

    if (!initialConfig)
    {
        // Load stored data, the addAP ready for MultiWiFi reconnection
        if (!configDataLoaded)
        loadConfigData();

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
        
        //connectMultiWiFi();
        }
    }

    Serial.print(F("After waiting "));
    Serial.print((float) (millis() - startedAt) / 1000L);
    Serial.print(F(" secs more in setup(), connection result is "));

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print(F("connected. Local IP: "));
        Serial.println(WiFi.localIP());
    }
    else
        Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));
}