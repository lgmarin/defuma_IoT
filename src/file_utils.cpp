#include "wifi_mgr.h"
#include <LittleFS.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;

const char* wifi_config_file = "/wifi_cfg.dat";
const char* config_file = "/config.dat";

WM_Config         WM_config;
APP_Config        APP_config;

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

bool loadConfigData(void *str_Config, size_t size, char* filename)
{
  File file = LittleFS.open(filename, "r");
  memset(str_Config, 0, size);

  if (file)
  {
    file.readBytes((char *) str_Config, size);
    file.close();
    Serial.println(F("Config File Loaded!"));
    return true;
  }
  else
  {
    Serial.println(F("ERROR: Loading Config File Failed!"));
    return false;
  }
}

bool saveConfigData(void *str_Config, size_t size, char* filename)
{
  File file = LittleFS.open(filename, "w");

  if (file)
  {
    file.write((uint8_t*) str_Config, size);
    file.close();

    Serial.println(F("Config File Saved!"));
    return true;
  }
  else
  {
    Serial.println(F("ERROR: Saving Config File Failed!"));
    return false;
  }
}

bool removeConfigData(char* filename)
{
  if (LittleFS.exists(filename))
  {
    if (LittleFS.remove(filename))
    {
      Serial.println(F("File removed!"));
      return true;
    }
    Serial.println(F("ERROR: File couldn't be removed!"));
    return false;
  }
  Serial.println(F("ERROR: File couldn't be removed!"));
  return false;
}

void storeWifiCred(String SSID, String password)
{
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
  //Calculate checksum and save credentials
  WM_config.checksum = calcChecksum((uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum));
  saveConfigData(&WM_config, sizeof(WM_config), (char*) wifi_config_file);
}

bool loadWifiCred()
{
  if(loadConfigData(&WM_config, sizeof(WM_config), (char*) wifi_config_file))
  {
    Serial.println(F("Wifi Config File Read. Checksum check..."));
    if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) )
    {
      Serial.println(F("Wifi Credentials checksum wrong!"));
      return false;
    }
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        Serial.print("Add SSID: ");
        Serial.println(WM_config.WiFi_Creds[i].wifi_ssid);
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
    Serial.println(F("\nWiFi connected!"));
    Serial.println("\nSSID: "); Serial.print(WiFi.SSID()); 
    Serial.println("\nRSSI= "); Serial.print(WiFi.RSSI());
    Serial.println("\nIP address: "), Serial.print(WiFi.localIP());
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

bool loadThresholdConfig()
{
  if(loadConfigData(&APP_config, sizeof(APP_config), (char*) config_file))
  {
    Serial.print(F("\nApp Config File Read."));
    Serial.print(APP_config.temp_max);
    Serial.print(APP_config.temp_min);
    return true;
  }
  return false;
}

bool storeThresholdConfig(String t_max, String t_min)
{
  memset(&APP_config, 0, sizeof(APP_config));
  
  if (strlen(t_max.c_str()) < sizeof(APP_config.temp_max) - 1)
    strcpy(APP_config.temp_max, t_max.c_str());
  else
    strncpy(APP_config.temp_max, t_max.c_str(), sizeof(APP_config.temp_max) - 1);

  if (strlen(t_min.c_str()) < sizeof(APP_config.temp_min) - 1)
    strcpy(APP_config.temp_min, t_min.c_str());
  else
    strncpy(APP_config.temp_min, t_min.c_str(), sizeof(APP_config.temp_min) - 1);  

  // Don't permit NULL values
  if ( (String(APP_config.temp_max) != "") && (strlen(APP_config.temp_min) >= MIN_AP_PASSWORD_SIZE) )
    return false;

  if( saveConfigData(&WM_config, sizeof(WM_config), (char*) wifi_config_file) )
    return true;

  return false;
}