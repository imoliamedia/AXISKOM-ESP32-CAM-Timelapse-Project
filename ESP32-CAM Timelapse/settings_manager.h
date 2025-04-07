#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include "config.h"

// Configuratie-instellingen
extern int photoInterval;  // Tijd tussen foto's in minuten
extern int dayStartHour;   // Start tijdstip voor foto's
extern int dayEndHour;     // Eind tijdstip voor foto's
extern int jpegQuality;    // JPEG kwaliteit (0-63)

// Functie voor instellingenbeheer
void loadSettings();
void saveSettings();
uint32_t calculateChecksum(TimelapseSavedSettings* settings);

#endif // SETTINGS_MANAGER_H