/****************************************************************************************************************************
  Async_ConfigOnStartup.ino
  For ESP8266 / ESP32 boards
  ESPAsync_WiFiManager is a library for the ESP8266/Arduino platform, using (ESP)AsyncWebServer to enable easy
  configuration and reconfiguration of WiFi credentials using a Captive Portal.
  Modified from 
  1. Tzapu               (https://github.com/tzapu/WiFiManager)
  2. Ken Taylor          (https://github.com/kentaylor)
  3. Alan Steremberg     (https://github.com/alanswx/ESPAsyncWiFiManager)
  4. Khoi Hoang          (https://github.com/khoih-prog/ESP_WiFiManager)
  Built by Khoi Hoang https://github.com/khoih-prog/ESPAsync_WiFiManager
  Licensed under MIT license
 *****************************************************************************************************************************/
/****************************************************************************************************************************
   This example will open a configuration portal for 60 seconds when first powered up if the boards has stored WiFi Credentials.
   Otherwise, it'll stay indefinitely in ConfigPortal until getting WiFi Credentials and connecting to WiFi
   ConfigOnSwitch is a a bettter example for most situations but this has the advantage
   that no pins or buttons are required on the ESP32/ESP8266 device at the cost of delaying
   the user sketch for the period that the configuration portal is open.
   Also in this example a password is required to connect to the configuration portal
   network. This is inconvenient but means that only those who know the password or those
   already connected to the target WiFi network can access the configuration portal and
   the WiFi network credentials will be sent from the browser over an encrypted connection and
   can not be read by observers.
 *****************************************************************************************************************************/
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>

// From v1.1.0
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#include <LittleFS.h>
FS* filesystem      =   &LittleFS;
#define FileFS          LittleFS
#define FS_Name         "LittleFS"

#define ESP_getChipId()   (ESP.getChipId())

#define LED_ON      LOW
#define LED_OFF     HIGH

// SSID and PW for Config Portal
String ssid = "ESP_" + String(ESP_getChipId(), HEX);
//const char* password = "your_password";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

// From v1.1.1
// You only need to format the filesystem once
//#define FORMAT_FILESYSTEM       true
#define FORMAT_FILESYSTEM         false

#define MIN_AP_PASSWORD_SIZE    8

#define SSID_MAX_LEN            32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN            64

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
} WiFi_Credentials;

typedef struct
{
  int temp_max;
  int temp_min;
} Thr_Config;

#define NUM_WIFI_CREDENTIALS      2

// Assuming max 49 chars
#define TZNAME_MAX_LEN            50
#define TIMEZONE_MAX_LEN          50

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
  char TZ_Name[TZNAME_MAX_LEN];     // "America/Toronto"
  char TZ[TIMEZONE_MAX_LEN];        // "EST5EDT,M3.2.0,M11.1.0"
  uint16_t checksum;
} WM_Config;

WM_Config         WM_config;
Thr_Config        Thr_config;

const char* config_file = "/wifi_cred.dat";
//////

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;

// Use false if you don't like to display Available Pages in Information Page of Config Portal
// Comment out or use true to display Available Pages in Information Page of Config Portal
// Must be placed before #include <ESP_WiFiManager.h>
#define USE_AVAILABLE_PAGES     true

// From v1.0.10 to permit disable/enable StaticIP configuration in Config Portal from sketch. Valid only if DHCP is used.
// You'll loose the feature of dynamically changing from DHCP to static IP, or vice versa
// You have to explicitly specify false to disable the feature.
//#define USE_STATIC_IP_CONFIG_IN_CP          false

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP     false

// Just use enough to save memory. On ESP8266, can cause blank ConfigPortal screen
// if using too much memory
#define USING_AMERICA       true


// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP          false

// New in v1.0.11
#define USING_CORS_FEATURE          false

////////////////////////////////////////////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
  // Force DHCP to be true
  #if defined(USE_DHCP_IP)
    #undef USE_DHCP_IP
  #endif
  #define USE_DHCP_IP     true
#else
  // You can select DHCP or Static IP here
  #define USE_DHCP_IP     true
  //#define USE_DHCP_IP     false
#endif

#if ( USE_DHCP_IP )
  // Use DHCP
  
  #if (_ESPASYNC_WIFIMGR_LOGLEVEL_ > 3)
    #warning Using DHCP IP
  #endif
  
  IPAddress stationIP   = IPAddress(0, 0, 0, 0);
  IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
  IPAddress netMask     = IPAddress(255, 255, 255, 0);
  
#else
    // Use static IP
    
    #if (_ESPASYNC_WIFIMGR_LOGLEVEL_ > 3)
        #warning Using static IP
    #endif
  
    IPAddress stationIP   = IPAddress(192, 168, 2, 186);
  
    IPAddress gatewayIP   = IPAddress(192, 168, 2, 1);
    IPAddress netMask     = IPAddress(255, 255, 255, 0);
#endif

////////////////////////////////////////////


#define USE_CONFIGURABLE_DNS      false

#include <ESPAsync_WiFiManager.h>               //https://github.com/khoih-prog/ESPAsync_WiFiManager

#define HTTP_PORT     80

// Onboard LED I/O pin on NodeMCU board
const int PIN_LED = 2; // D4 on NodeMCU and WeMos. GPIO2/ADC12 of ESP32. Controls the onboard LED.

WiFi_STA_IPConfig WM_STA_IPconfig;

void initSTAIPConfigStruct(WiFi_STA_IPConfig &in_WM_STA_IPconfig)
{
  in_WM_STA_IPconfig._sta_static_ip   = stationIP;
  in_WM_STA_IPconfig._sta_static_gw   = gatewayIP;
  in_WM_STA_IPconfig._sta_static_sn   = netMask;
}

void configWiFi(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
    // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
    WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn);
}

///////////////////////////////////////////

uint8_t connectMultiWiFi()
{
    // For ESP8266, this better be 2200 to enable connect the 1st time
    #define WIFI_MULTI_1ST_CONNECT_WAITING_MS             2200L
    #define WIFI_MULTI_CONNECT_WAITING_MS                  500L

    uint8_t status;

    //WiFi.mode(WIFI_STA);

    LOGERROR(F("ConnectMultiWiFi with :"));

    if ( (Router_SSID != "") && (Router_Pass != "") )
    {
        LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass );
        LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass );
        wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
    }

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
        // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
        if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
        {
            LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
        }
    }

    LOGERROR(F("Connecting MultiWifi..."));

    //WiFi.mode(WIFI_STA);

    #if !USE_DHCP_IP
        // New in v1.4.0
        configWiFi(WM_STA_IPconfig);
        //////
    #endif

    int i = 0;
    status = wifiMulti.run();
    delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

    while ( ( i++ < 20 ) && ( status != WL_CONNECTED ) )
    {
        status = WiFi.status();

        if ( status == WL_CONNECTED )
            break;
        else
            delay(WIFI_MULTI_CONNECT_WAITING_MS);
        }

        if ( status == WL_CONNECTED )
        {
            LOGERROR1(F("WiFi connected after time: "), i);
            LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
            LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP() );
        }
        else
        {
            LOGERROR(F("WiFi not connected"));   
            ESP.reset();
    }

return status;
}

void toggleLED()
{
    //toggle state
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void heartBeatPrint()
{
    static int num = 1;

    if (WiFi.status() == WL_CONNECTED)
        Serial.print(F("H"));        // H means connected to WiFi
    else
        Serial.print(F("F"));        // F means not connected to WiFi

    if (num == 80)
    {
        Serial.println();
        num = 1;
    }
    else if (num++ % 10 == 0)
    {
        Serial.print(F(" "));
    }
}

void check_WiFi()
{
    if ( (WiFi.status() != WL_CONNECTED) )
    {
        Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
        connectMultiWiFi();
    }
}  

void check_status()
{
    static float checkstatus_timeout  = 0;
    static float LEDstatus_timeout    = 0;
    static float checkwifi_timeout    = 0;

    static float current_millis;

    #define WIFICHECK_INTERVAL    1000L

    #define HEARTBEAT_INTERVAL    10000L

    #define LED_INTERVAL          2000L

    current_millis = millis();

    // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
    if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
    {
        check_WiFi();
        checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
    }

    if ((current_millis > LEDstatus_timeout) || (LEDstatus_timeout == 0))
    {
        // Toggle LED at LED_INTERVAL = 2s
        toggleLED();
        LEDstatus_timeout = current_millis + LED_INTERVAL;
    }

    // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
    if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
    {
        heartBeatPrint();
        checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
    }
}

int calcChecksum(uint8_t* address, uint16_t sizeToCalc)
{
    uint16_t checkSum = 0;

    for (uint16_t index = 0; index < sizeToCalc; index++)
    {
        checkSum += * ( ( (byte*) address ) + index);
    }

    return checkSum;
}

bool loadConfigData(const char *filename)
{
    File file = FileFS.open(filename, "r");
    LOGERROR(F("Loading Config File..."));

    // Load Wifi Credentials and IP Configuration
    memset((void *) &WM_config,       0, sizeof(WM_config));
    memset((void *) &WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
    memset((void *) &Thr_config,      0, sizeof(Thr_config));

    if (file)
    {
        file.readBytes((char *) &WM_config,   sizeof(WM_config));
        file.readBytes((char *) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
        file.readBytes((char *) &Thr_config, sizeof(Thr_config));

        file.close();
        LOGERROR(F("Config File Read. Checksum check..."));

        if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) )
        {
            LOGERROR(F("Wifi Credentials checksum wrong!"));
            return false;
        }
        LOGERROR(F("Config File Loaded!"));
        return true;
    }
    else
    {
        LOGERROR(F("Loading Config File Failed!"));
        return false;
    }
}

void saveConfigData(const char *filename)
{
    File file = FileFS.open(filename, "w");
    LOGERROR(F("Saving Config File..."));

    if (file)
    {
        WM_config.checksum = calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) );

        file.write((uint8_t*) &WM_config, sizeof(WM_config));
        file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
        file.write((uint8_t*) &Thr_config, sizeof(Thr_config));

        file.close();
        LOGERROR(F("Config File Saved!"));
    }
    else
    {
        LOGERROR(F("Saving Config File Failed!"));
    }
}

void setup()
{
    // put your setup code here, to run once:
    // initialize the LED digital pin as an output.
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial);

    delay(200);

    Serial.print(F("\nStarting Async_ConfigOnStartup using ")); Serial.print(FS_Name);
    Serial.print(F(" on ")); Serial.println(ARDUINO_BOARD);
    Serial.println(ESP_ASYNC_WIFIMANAGER_VERSION);

    Serial.setDebugOutput(false);

    if (FORMAT_FILESYSTEM) 
        FileFS.format();

    // Format FileFS if not yet
    if (!FileFS.begin())
        {
        FileFS.format();

        Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));

        if (!FileFS.begin())
        {     
            // prevents debug info from the library to hide err message.
            delay(100);
            Serial.println(F("LittleFS failed!. Please use SPIFFS or EEPROM. Stay forever"));
            while (true)
            {
            delay(1);
            }
        }
    }

    unsigned long startedAt = millis();

    // New in v1.4.0
    initSTAIPConfigStruct(WM_STA_IPconfig);
    //////

    //Local intialization. Once its business is done, there is no need to keep it around
    AsyncWebServer webServer(HTTP_PORT);
    DNSServer dnsServer;
    ESPAsync_WiFiManager ESPAsync_wifiManager(&webServer, &dnsServer, "AsyncConfigOnStartup");

    #if !USE_DHCP_IP    
        // Set (static IP, Gateway, Subnetmask, DNS1 and DNS2) or (IP, Gateway, Subnetmask). New in v1.0.5
        // New in v1.4.0
        ESPAsync_wifiManager.setSTAStaticIPConfig(WM_STA_IPconfig);
        //////
    #endif

    // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
    // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
    // Have to create a new function to store in EEPROM/SPIFFS for this purpose
    Router_SSID = ESPAsync_wifiManager.WiFi_SSID();
    Router_Pass = ESPAsync_wifiManager.WiFi_Pass();

    //Check if there is stored WiFi router/password credentials.
    //If not found, device will remain in configuration mode until switched off via webserver.
    Serial.println(F("Opening configuration portal."));

    bool configDataLoaded = false;

    // From v1.1.0, Don't permit NULL password
    if ( (Router_SSID != "") && (Router_Pass != "") )
    {
        LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
        wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
        
        ESPAsync_wifiManager.setConfigPortalTimeout(120); //If no access point name has been previously entered disable timeout.
        Serial.println(F("Got ESP Self-Stored Credentials. Timeout 120s for Config Portal"));
    }

    if (loadConfigData(config_file))
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

    // SSID to uppercase
    ssid.toUpperCase();

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
    if ( String(ESPAsync_wifiManager.getSSID(0)) != "" && String(ESPAsync_wifiManager.getSSID(1)) != "" )
    {
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
                LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw );
                wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
            }
        }

        // New in v1.4.0
        ESPAsync_wifiManager.getSTAStaticIPConfig(WM_STA_IPconfig);
        //////
        
        saveConfigData(config_file);

        initialConfig = true;
    }

    digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.

    startedAt = millis();

    if (!initialConfig)
    {
        // Load stored data, the addAP ready for MultiWiFi reconnection
        if (!configDataLoaded)
        loadConfigData(config_file);

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

void loop()
{
  // put your main code here, to run repeatedly
  check_status();
}
