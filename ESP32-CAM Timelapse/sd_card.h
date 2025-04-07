#ifndef SD_CARD_H
#define SD_CARD_H

#include "config.h"

// Functies voor SD-kaart beheer
bool initSDCard();
bool createDayFolder();
void removeDir(String path);
String formatFileSize(size_t bytes);

#endif // SD_CARD_H