#include "config.h"

// Status variabelen
bool sdCardAvailable = false;
bool timeInitialized = false;
unsigned long lastPhotoTime = 0;
unsigned long lastNTPSync = 0;
char filePath[100];
char folderPath[50];