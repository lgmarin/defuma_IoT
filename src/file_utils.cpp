#include "wifi_mgr.h"
#include <LittleFS.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;

const char* config_file = "/configuration.dat";

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

void storeWifiCred(String SSID, String password)
{
  // Stored  for later usage, from v1.1.0, but clear first
  memset(&WM_config, 0, sizeof(WM_config));
  
  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
      String tempSSID = SSID;
      String tempPW   = password;

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
          Serial.println(F("Invalid SSID or Password!"));
      }
  }
  saveConfigData();
}

bool loadWifiCred()
{
  if(loadConfigData())
  {
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
          wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

void connectMultiWifi()
{
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS             2200L
  #define WIFI_MULTI_CONNECT_WAITING_MS                  500L

  uint8_t status;

  Serial.println(F("Connecting MultiWifi..."));

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
    Serial.println(F("WiFi connected!"));
    Serial.println("SSID: "); Serial.print(WiFi.SSID()); 
    Serial.println("RSSI= "); Serial.print(WiFi.RSSI());
    Serial.println("IP address: "), Serial.print(WiFi.localIP());
  }
  else
  {
    Serial.println(F("WiFi not connected, reseting ESP!")); 
    ESP.reset();
  }
}

void checkWifiStatus()
{
  static float checkwifi_timeout = 0;
  static float current_millis;

  current_millis = millis();

  if ( (WiFi.status() != WL_CONNECTED) )
  {
    // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
    if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
    {
      checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;      
      Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
      connectMultiWifi();
    } 
  }
}