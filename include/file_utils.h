#ifndef file_utils_h
#define file_utils_h

void initFS();
String readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);

#endif