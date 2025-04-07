#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"

extern const char* ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// Status variabelen voor tijd
extern bool timeInitialized;
extern unsigned long lastNTPSync;

// Functies voor tijdbeheer
void setupTimeSync();
bool syncTimeNTP();
bool isDay();

#endif // TIME_MANAGER_H