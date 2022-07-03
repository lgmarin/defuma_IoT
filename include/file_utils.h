#ifndef file_utils_h
#define file_utils_h

void initFS();
bool loadConfigData();
void saveConfigData();
void storeWifiCred(String SSID, String password);
bool loadWifiCred();
void connectMultiWifi();
void checkWifiStatus();
bool loadThresholdConfig();
bool storeThresholdConfig(String t_max, String t_min);

#endif