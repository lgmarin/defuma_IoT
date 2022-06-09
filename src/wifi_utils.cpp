
String Router_SSID;
String Router_Pass;
String ssid = "defumat_IoT"
String password = "defuma"

void setup()
{
    //set led pin as output
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(TRIGGER_PIN, INPUT_PULLUP);
    pinMode(TRIGGER_PIN2, INPUT_PULLUP);

    Serial.begin(115200);
    while (!Serial);

    delay(200);

    digitalWrite(LED_BUILTIN, LED_ON); // turn the LED on by making the voltage LOW to tell us we are in configuration mode.

    //Local intialization. Once its business is done, there is no need to keep it around
    // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
    //ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer);
    // Use this to personalize DHCP hostname (RFC952 conformed)
    AsyncWebServer webServer(80);
    DNSServer dnsServer;
    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "AsyncConfigOnSwitch");

    // Use only to erase stored WiFi Credentials
    //resetSettings();
    //ESPAsync_wifiManager.resetSettings();

    // We can't use WiFi.SSID() in ESP32as it's only valid after connected.
    // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
    // Have to create a new function to store in EEPROM/SPIFFS for this purpose
    Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
    Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

    bool configDataLoaded = false;

    if (loadConfigData())
    {
        configDataLoaded = true;
        
        ESPAsync_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
        Serial.println(F("Got stored Credentials. Timeout 120s for Config Portal"));
    }
    else
    {
        // Enter CP only if no stored SSID on flash and file 
        Serial.println(F("Open Config Portal without Timeout: No stored Credentials."));
        initialConfig = true;
    }

    if (initialConfig)
    {
        Serial.print(F("Starting configuration portal"));

        digitalWrite(LED_BUILTIN, LED_ON); // Turn led on as we are in configuration mode.

        //sets timeout in seconds until configuration portal gets turned off.
        //If not specified device will remain in configuration mode until
        //switched off via webserver or device is restarted.
        //ESPAsync_wifiManager.setConfigPortalTimeout(600);

        // Starts an access point
        if (!ESPAsync_wifiManager.startConfigPortal((const char *) ssid.c_str(), password.c_str()))
            Serial.println(F("Not connected to WiFi but continuing anyway."));
        else
        {
            Serial.println(F("WiFi connected...yeey :)"));
        }

        // Stored  for later usage, from v1.1.0, but clear first
        memset(&WM_config, 0, sizeof(WM_config));
        
        for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
        {
        String tempSSID = ESPAsync_wifiManager.getSSID(i);
        String tempPW   = ESPAsync_wifiManager.getPW(i);
    
        if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
            strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
        else
            strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

        if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
            strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
        else
            strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);  

        // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
        if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
        {
            Serial.println(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
            wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
        }
        }

        saveConfigData();
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
        
        connectMultiWiFi();
        }
    }

    Serial.print(F("After waiting "));
    Serial.print((float) (millis() - startedAt) / 1000);
    Serial.print(F(" secs more in setup(), connection result is "));

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print(F("connected. Local IP: "));
        Serial.println(WiFi.localIP());
    }
    else
        Serial.println(ESPAsync_wifiManager.getStatus(WiFi.status()));
}