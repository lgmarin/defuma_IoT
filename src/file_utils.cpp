#include "wifi_mgr.h"
#include <LittleFS.h>
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti wifiMulti;

WM_Config         WM_config;
APP_Config        APP_config;

const char* wifi_config_file = "/wifi_cfg.dat";
const char* config_file = "/config.dat";

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.print("\n[ERROR]: An error has occurred while mounting LittleFS");
  }
  else{
    Serial.print("\n[INFO]: LittleFS mounted successfully");
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
    return true;
  }
  else
  {
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
    return true;
  }
  else
  {
    return false;
  }
}

bool removeConfigData(char* filename)
{
  if (LittleFS.exists(filename))
  {
    if (LittleFS.remove(filename))
    {
      Serial.print(F("\n[INFO]: File removed!"));
      return true;
    }
    Serial.print(F("[ERROR]: File couldn't be removed! Removal error."));
    return false;
  }
  Serial.print(F("[ERROR]: File couldn't be removed! Not found."));
  return false;
}

bool storeWifiCred(String SSID, String password)
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
      Serial.println(F("[ERROR]: Invalid SSID or Password!"));
      return false;
    }
  }
  //Calculate checksum and save credentials
  WM_config.checksum = calcChecksum((uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum));
  if (saveConfigData(&WM_config, sizeof(WM_config), (char*) wifi_config_file))
  {
    Serial.print(F("\n[INFO]: Wifi Credentials file saved!"));
    return true;
  }

  Serial.print(F("\n[ERROR]: Could not store Wifi Config File"));
  return false;
}

bool loadWifiCred()
{
  if(loadConfigData(&WM_config, sizeof(WM_config), (char*) wifi_config_file))
  {
    if ( WM_config.checksum != calcChecksum( (uint8_t*) &WM_config, sizeof(WM_config) - sizeof(WM_config.checksum) ) )
    {
      Serial.print(F("\n[ERROR]: Wifi Credentials checksum wrong!"));
      return false;
    }
    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ( (String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE) )
      {
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }
    return true;
    Serial.print(F("\n[INFO]: Wifi Config File Read. Checksum ok."));
  }
  else
  {
    Serial.print(F("\n[ERROR]: Could not read Wifi Config File."));
    return false;
  }
}

void connectMultiWifi()
{
  #define WIFI_MULTI_1ST_CONNECT_WAITING_MS             2200L
  #define WIFI_MULTI_CONNECT_WAITING_MS                  500L

  uint8_t status;

  Serial.println(F("\n[INFO]: Connecting MultiWifi..."));

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
    Serial.println(F("\n[INFO] WiFi connected!"));
    Serial.print("\nSSID: "); Serial.print(WiFi.SSID()); 
    Serial.print("\nRSSI= "); Serial.print(WiFi.RSSI());
    Serial.print("\nIP address: "), Serial.print(WiFi.localIP());
  }
  else
  {
    Serial.println(F("\n[EROR]: No WiFi connected, reseting ESP!")); 
    ESP.reset();
  }
}

void checkWifiStatus()
{
  static float checkwifi_timeout = 0;
  static float current_millis;

  current_millis = millis();

  if ((WiFi.status() != WL_CONNECTED))
  {
    // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
    if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
    {
      checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;      
      Serial.println(F("\n[ERROR]: WiFi lost. Call connectMultiWiFi in loop"));
      connectMultiWifi();
    } 
  }
}

bool loadThresholdConfig()
{
  if(loadConfigData(&APP_config, sizeof(APP_config), (char*) config_file))
  {
    Serial.print(F("\n[INFO]: App Config File Read."));
    Serial.print(F("\nt_min")); Serial.print(String(APP_config.temp_min));
    Serial.print(F("\nt_max")); Serial.print(String(APP_config.temp_max));
    return true;
  }
  Serial.print(F("\n[ERROR]: Could not read App Config File"));
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

  Serial.print(F("\n[INFO]: App Config File Store."));
  Serial.print(F("\nt_min")); Serial.print(String(APP_config.temp_min));
  Serial.print(F("\nt_max")); Serial.print(String(APP_config.temp_max));


  // Don't permit NULL values
  if (String(APP_config.temp_max) == ""){
    Serial.print(F("\n[ERROR]: Invalid null Threshold value."));
    return false;
  }

  if(saveConfigData(&WM_config, sizeof(WM_config), (char*) config_file)){
    Serial.print(F("\n[INFO]: APP Configuration saved!"));
    return true;
  }

  Serial.print(F("\n[ERROR]: Could not save APP Configuration."));
  return false;
}

bool removeThresholdConfig()
{
  if(removeConfigData((char*) config_file))
    return true;

  return false;
}

bool removeWifiCred()
{
  if(removeConfigData((char*) wifi_config_file))
    return true;

  return false;
}