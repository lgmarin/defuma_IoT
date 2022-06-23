#include "LittleFS.h"
#include <wifi_mgr.h>

const char* config_file = "/configuration.dat";

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw  [PASS_MAX_LEN];
} WiFi_Credentials;

typedef struct
{
  char temp_max[3];
  char temp_min[3];
} Thr_Config;

typedef struct
{
  WiFi_Credentials  WiFi_Creds [NUM_WIFI_CREDENTIALS];
  char TZ_Name[TZNAME_MAX_LEN];     // "America/Toronto"
  char TZ[TIMEZONE_MAX_LEN];        // "EST5EDT,M3.2.0,M11.1.0"
  uint16_t checksum;
} WM_Config;

WM_Config         WM_config;
Thr_Config        Thr_config;

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
  File file = LittleFS.open(config_file, "r");
  Serial.println(F("Loading Config File..."));

  // Load Wifi Credentials and IP Configuration
  memset((void *) &WM_config,       0, sizeof(WM_config));
  memset((void *) &Thr_config,      0, sizeof(Thr_config));

  if (file)
  {
    file.readBytes((char *) &WM_config,   sizeof(WM_config));
    file.readBytes((char *) &Thr_config, sizeof(Thr_config));

    file.close();
    Serial.println(F("Config File Read. Checksum check..."));

    if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) )
    {
      Serial.println(F("Wifi Credentials checksum wrong!"));
      return false;
    }
    Serial.println(F("Config File Loaded!"));
    return true;
  }
  else
  {
    Serial.println(F("Loading Config File Failed!"));
    return false;
  }
}

void saveConfigData()
{
  File file = LittleFS.open(config_file, "w");
  Serial.println(F("Saving Config File..."));

  if (file)
  {
    WM_config.checksum = calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) );

    file.write((uint8_t*) &WM_config, sizeof(WM_config));
    file.write((uint8_t*) &Thr_config, sizeof(Thr_config));

    file.close();
    Serial.println(F("Config File Saved!"));
  }
  else
  {
      Serial.println(F("Saving Config File Failed!"));
  }
}