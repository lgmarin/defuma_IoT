#include <wifi_mgr.h>

// SSID and PW for Config Portal
const str ssid = "defuma_iot";
//const char* password = "your_password";

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

WM_Config         WM_config;
Thr_Config        Thr_config;

const char* config_file = "/wifi_cred.dat";
//////

// Indicates whether ESP has WiFi credentials saved from previous session, or double reset detected
bool initialConfig = false;