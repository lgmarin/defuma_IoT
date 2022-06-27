#ifndef file_utils_h
#define file_utils_h

void initFS();
bool loadConfigData();
void saveConfigData();
void storeWifiCred();
bool loadWifiCred();
uint8_t connectMultiWifi();

#endif