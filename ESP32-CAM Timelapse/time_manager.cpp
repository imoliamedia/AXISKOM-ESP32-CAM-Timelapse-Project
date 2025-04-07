#include "time_manager.h"

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;  // GMT+1 (Centraal-Europese tijd)
const int daylightOffset_sec = 3600;  // Zomertijd correctie

// Time setup
void setupTimeSync() {
  // Configureer de tijdserver
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  // Probeer tijd te synchroniseren
  if (syncTimeNTP()) {
    timeInitialized = true;
    Serial.println("Tijd gesynchroniseerd");
  } else {
    Serial.println("Tijd synchronisatie mislukt");
  }
}

// Synchroniseer tijd via NTP
bool syncTimeNTP() {
  time_t now;
  struct tm timeinfo;
  
  // Probeer tijd te synchroniseren
  int retry = 0;
  const int maxRetries = 5;
  
  while (retry < maxRetries) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    delay(1000);
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    if (timeinfo.tm_year > (2020 - 1900)) {
      // Tijd is succesvol gesynchroniseerd
      Serial.printf("Huidige tijd: %02d:%02d:%02d %02d/%02d/%04d\n", 
                   timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
                   timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
      lastNTPSync = millis();
      return true;
    }
    
    Serial.println("NTP synchronisatie poging mislukt, opnieuw proberen...");
    retry++;
    delay(1000);
  }
  
  return false;
}

// Controleer of het binnen de ingestelde daguren is
bool isDay() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  return (timeinfo.tm_hour >= dayStartHour && timeinfo.tm_hour < dayEndHour);
}