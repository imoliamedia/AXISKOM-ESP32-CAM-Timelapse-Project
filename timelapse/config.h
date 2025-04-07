#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include "esp_camera.h"
#include "time.h"
#include "FS.h"
#include "SD_MMC.h"

// Pin definities voor ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Het LED-lampje op het bord
#define FLASH_LED_PIN 4

// WiFi-instellingen 
extern const char* ssid;
extern const char* password;

// NTP-server voor synchroniseren van de tijd
extern const char* ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

// Configuratie-instellingen 
extern int photoInterval;    // Tijd tussen foto's in minuten
extern int dayStartHour;     // Start tijdstip voor foto's (8:00)
extern int dayEndHour;       // Eind tijdstip voor foto's (20:00)
extern int jpegQuality;      // JPEG kwaliteit (0-63, lagere waarde = hogere kwaliteit)

// Buffer voor bestandspaden
extern char filePath[100];
extern char folderPath[50];

// Status variabelen
extern bool sdCardAvailable;
extern bool timeInitialized;
extern unsigned long lastPhotoTime;
extern unsigned long lastNTPSync;

// Constanten
#define NTP_SYNC_INTERVAL 86400000  // Eén keer per dag tijd synchroniseren
#define SETTINGS_CHECKSUM 0xABCD1234

// Structuur voor het opslaan van instellingen
struct TimelapseSavedSettings {
  int photoInterval;
  int dayStartHour;
  int dayEndHour;
  int jpegQuality;
  uint32_t checksum;
};

#endif // CONFIG_H