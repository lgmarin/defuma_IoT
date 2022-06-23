#include <wifi_mgr.h>

// SSID and PW for Config Portal
String ssid = "ESP_" + String(ESP.getChipId(), HEX);
//const char* password = "your_password";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

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

const char* config_file = "/wifi_cred.dat";
//////

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;