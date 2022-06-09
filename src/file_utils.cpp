#include "LittleFS.h"

#define FORMAT_FILESYSTEM         false
#define SSID_MAX_LEN              32
#define PASS_MAX_LEN              64
#define NUM_WIFI_CREDENTIALS      2
#define CONFIG_FILENAME           F("/wifi_cred.dat")

// Use false to disable NTP config. Advisable when using Cellphone, Tablet to access Config Portal.
// See Issue 23: On Android phone ConfigPortal is unresponsive (https://github.com/khoih-prog/ESP_WiFiManager/issues/23)
#define USE_ESP_WIFIMANAGER_NTP     true

// Just use enough to save memory. On ESP8266, can cause blank ConfigPortal screen
// if using too much memory
#define USING_AFRICA        false
#define USING_AMERICA       true
#define USING_ANTARCTICA    false
#define USING_ASIA          false
#define USING_ATLANTIC      false
#define USING_AUSTRALIA     false
#define USING_EUROPE        false
#define USING_INDIAN        false
#define USING_PACIFIC       false
#define USING_ETC_GMT       false

// Use true to enable CloudFlare NTP service. System can hang if you don't have Internet access while accessing CloudFlare
// See Issue #21: CloudFlare link in the default portal (https://github.com/khoih-prog/ESP_WiFiManager/issues/21)
#define USE_CLOUDFLARE_NTP          false

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
}  WiFi_Credentials;

typedef struct
{
  String wifi_ssid;
  String wifi_pw;
}  WiFi_Credentials_String;

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
  uint16_t checksum;
} WM_Config;

WM_Config WM_config;


// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
    Serial.println("LittleFS mounted successfully");
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

bool loadConfigData()
{
  File file = LittleFS.open(CONFIG_FILENAME, "r");
  Serial.println(F("LoadWiFiCfgFile "));

  memset((void *) &WM_config,       0, sizeof(WM_config));

  // New in v1.4.0
  //memset((void *) &WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
  //////

  if (file)
  {
    file.readBytes((char *) &WM_config,   sizeof(WM_config));

    // New in v1.4.0
    //file.readBytes((char *) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    Serial.println(F("OK"));

    if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) )
    {
      Serial.println(F("WM_config checksum wrong"));
      
      return false;
    }
    
    // New in v1.4.0
    //displayIPConfigStruct(WM_STA_IPconfig);
    //////

    return true;
  }
  else
  {
    Serial.println(F("failed"));

    return false;
  }
}

void saveConfigData()
{
  File file = LittleFS.open(CONFIG_FILENAME, "w");
  Serial.println(F("SaveWiFiCfgFile "));

  if (file)
  {
    WM_config.checksum = calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) );
    
    file.write((uint8_t*) &WM_config, sizeof(WM_config));

    //displayIPConfigStruct(WM_STA_IPconfig);

    // New in v1.4.0
    //file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    Serial.println(F("OK"));
  }
  else
  {
    Serial.println(F("failed"));
  }
}
