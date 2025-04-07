#include "sd_card.h"

// Initialiseer de SD-kaart
bool initSDCard() {
  // SD-kaart initialiseren in 1-bit modus
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD-kaart initialisatie mislukt!");
    sdCardAvailable = false;
    return false;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Geen SD-kaart gedetecteerd");
    sdCardAvailable = false;
    return false;
  }
  
  // SD-kaart capaciteit controleren
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD-kaart gedetecteerd. Type: %d, Grootte: %lluMB\n", cardType, cardSize);
  
  // Verzeker dat de kaart toegankelijk is
  if (cardSize == 0) {
    Serial.println("SD-kaart heeft 0 capaciteit - waarschijnlijk corrupte mount");
    sdCardAvailable = false;
    return false;
  }
  
  // Maak een hoofdmap voor de timelapse als die nog niet bestaat
  if (!SD_MMC.exists("/timelapse")) {
    Serial.println("Timelapse map niet gevonden, wordt aangemaakt");
    SD_MMC.mkdir("/timelapse");
  }
  
  sdCardAvailable = true;
  return true;
}

// Maak een map aan voor de huidige dag als die nog niet bestaat
bool createDayFolder() {
  if (!sdCardAvailable) return false;
  
  // Huidige datum ophalen
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  // Map pad aanmaken in het formaat "/timelapse/DD-MM-YYYY"
  sprintf(folderPath, "/timelapse/%02d-%02d-%04d", 
          timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  
  // Controleer of de map al bestaat
  if (!SD_MMC.exists(folderPath)) {
    Serial.printf("Map voor vandaag wordt aangemaakt: %s\n", folderPath);
    if (!SD_MMC.mkdir(folderPath)) {
      Serial.println("Fout bij aanmaken van dagmap");
      return false;
    }
  }
  
  return true;
}

// Recursief verwijderen van een map en alle inhoud
void removeDir(String path) {
  if (!sdCardAvailable) return;
  
  File dir = SD_MMC.open(path);
  if (!dir || !dir.isDirectory()) return;
  
  File file = dir.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      // Recursief verwijderen van submappen
      removeDir(String(file.path()));
    } else {
      // Verwijder bestand
      SD_MMC.remove(String(file.path()));
    }
    file = dir.openNextFile();
  }
  
  // Nu de map zelf verwijderen
  SD_MMC.rmdir(path);
}

// Bestandsgrootte formatteren naar leesbare vorm (B, KB, MB)
String formatFileSize(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0, 1) + " KB";
  } else {
    return String(bytes / (1024.0 * 1024.0), 1) + " MB";
  }
}