#include "LittleFS.h"
#include <wifi_mgr.h>

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

bool loadConfigData(const char *filename)
{
    File file = LittleFS.open(filename, "r");
    Serial.println(F("Loading Config File..."));

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

void saveConfigData(const char *filename)
{
    File file = LittleFS.open(filename, "w");
    Serial.println(F("Saving Config File..."));

    if (file)
    {
        WM_config.checksum = calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) );

        file.write((uint8_t*) &WM_config, sizeof(WM_config));
        file.write((uint8_t*) &WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
        file.write((uint8_t*) &Thr_config, sizeof(Thr_config));

        file.close();
        Serial.println(F("Config File Saved!"));
    }
    else
    {
        Serial.println(F("Saving Config File Failed!"));
    }
}