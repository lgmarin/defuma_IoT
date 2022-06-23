#ifndef file_utils_h
#define file_utils_h

void initFS();
bool loadConfigData(const char *filename);
void saveConfigData(const char *filename);

#endif