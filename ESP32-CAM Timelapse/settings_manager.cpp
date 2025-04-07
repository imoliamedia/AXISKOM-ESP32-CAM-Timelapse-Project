#include "settings_manager.h"
#include "camera.h"

// Standaardwaarden voor instellingen
int photoInterval = 5;    // Standaard 5 minuten tussen foto's
int dayStartHour = 8;     // Start om 8:00
int dayEndHour = 20;      // Eindigt om 20:00
int jpegQuality = 10;     // Hoge kwaliteit

// Bereken een eenvoudige checksum voor instellingen validatie
uint32_t calculateChecksum(TimelapseSavedSettings* settings) {
  return settings->photoInterval + settings->dayStartHour + settings->dayEndHour + 
         settings->jpegQuality + SETTINGS_CHECKSUM;
}

// Laad instellingen uit flash (EEPROM emulatie)
void loadSettings() {
  EEPROM.begin(sizeof(TimelapseSavedSettings));
  
  TimelapseSavedSettings savedSettings;
  EEPROM.get(0, savedSettings);
  
  // Controleer of gegevens geldig zijn met checksum
  if (calculateChecksum(&savedSettings) == savedSettings.checksum) {
    photoInterval = savedSettings.photoInterval;
    dayStartHour = savedSettings.dayStartHour;
    dayEndHour = savedSettings.dayEndHour;
    jpegQuality = savedSettings.jpegQuality;
    Serial.println("Instellingen geladen uit flash");
  } else {
    Serial.println("Geen geldige instellingen gevonden in flash, standaardwaarden worden gebruikt");
  }
  
  EEPROM.end();
}

// Sla instellingen op in flash (EEPROM emulatie)
void saveSettings() {
  // Preferenties in EEPROM opslaan
  EEPROM.begin(sizeof(TimelapseSavedSettings));
  
  TimelapseSavedSettings savedSettings;
  savedSettings.photoInterval = photoInterval;
  savedSettings.dayStartHour = dayStartHour;
  savedSettings.dayEndHour = dayEndHour;
  savedSettings.jpegQuality = jpegQuality;
  savedSettings.checksum = calculateChecksum(&savedSettings);
  
  // Debug info printen
  Serial.println("Opslaan van instellingen:");
  Serial.println("photoInterval: " + String(photoInterval));
  Serial.println("dayStartHour: " + String(dayStartHour));
  Serial.println("dayEndHour: " + String(dayEndHour));
  Serial.println("jpegQuality: " + String(jpegQuality));
  
  EEPROM.put(0, savedSettings);
  bool success = EEPROM.commit();
  Serial.println("EEPROM commit resultaat: " + String(success ? "Succesvol" : "Mislukt"));
  EEPROM.end();
  
  // Update camera instellingen
  updateCameraSettings();
  
  Serial.println("Instellingen opgeslagen in flash");
}