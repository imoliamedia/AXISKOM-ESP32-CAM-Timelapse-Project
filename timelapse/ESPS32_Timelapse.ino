/**
 * ESP32-CAM Plantengroei Timelapse Project
 * 
 * Dit programma maakt automatisch timelapse foto's met een ESP32-CAM
 * en slaat deze op op een SD-kaart. De camera maakt alleen foto's tijdens de dag (8:00-20:00).
 * Bevat een eenvoudige webinterface om foto's te bekijken en individueel te downloaden.
 * 
 * INSTELLINGEN AANPASSEN:
 * - WiFi: Verander 'ssid' en 'password' naar je eigen WiFi-gegevens (regels 27-28)
 * - Foto interval: Pas 'photoInterval' aan om de tijd tussen foto's te wijzigen (regel 32)
 * - Dag/nacht cyclus: Wijzig 'dayStartHour' en 'dayEndHour' voor de actieve uren (regels 33-34)
 * - Beeldkwaliteit: Pas 'jpegQuality' aan voor betere/slechtere foto's (regel 35)
 * 
 * GEBRUIK:
 * - Upload de code naar je ESP32-CAM met Arduino IDE
 * - Plaats een geformatteerde SD-kaart in de ESP32-CAM
 * - Verbind met hetzelfde WiFi-netwerk
 * - Open een browser en ga naar het IP-adres dat in de seriële monitor wordt getoond
 * - Foto's kunnen individueel gedownload worden via de webinterface
 * 
 * Ontwikkeld voor en gedeeld door AXISKOM 
 * Een Nederlands kennisplatform voor zelfredzaamheid,
 * prepping, outdoor skills en zelfvoorzienend leven.
 * 
 * Meer info: https://axiskom.nl
 * 
 * Versie: 2.0 (April 2025)
 */

#include "config.h"
#include "camera.h"
#include "sd_card.h"
#include "wifi_manager.h"
#include "time_manager.h"
#include "web_server.h"
#include "web_handlers.h"  
#include "web_views.h"     
#include "web_utils.h"     
#include "settings_manager.h"

void setup() {
  // Start seriële communicatie
  Serial.begin(115200);
  Serial.println("\nESP32-CAM Plantengroei Timelapse Project");
  
  // Flash LED als uitgang instellen
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  // Extra stap voor SD-kaart herkenning
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  
  // Even wachten op SD-kaart stabilisatie
  delay(500);
  
  // Laad opgeslagen instellingen
  loadSettings();
  
  // Initialiseer camera
  if (!initCamera()) {
    Serial.println("Camera initialisatie mislukt");
    delay(5000);
    ESP.restart();
    return;
  }
  
  // SD-kaart initialiseren met herhaalde pogingen
  initSDCard();
  
  // WiFi verbinding opzetten
  setupWiFi();
  
  // NTP tijd synchroniseren
  setupTimeSync();
  
  // Start webserver
  startWebServer();
  
  Serial.println("Setup voltooid. Timelapse actief.");
}

void loop() {
  // Als tijd gesynchroniseerd is, controleer tijd voor foto's maken
  if (timeInitialized) {
    // Dagelijks de tijd synchroniseren
    unsigned long currentTime = millis();
    if (currentTime - lastNTPSync > NTP_SYNC_INTERVAL) {
      syncTimeNTP();
      lastNTPSync = currentTime;
    }
    
    // Controleer of het tijd is voor een nieuwe foto
    if (sdCardAvailable && isDay()) {
      if (currentTime - lastPhotoTime > (photoInterval * 60 * 1000)) {
        // Maak een dagmap als die nog niet bestaat
        if (createDayFolder()) {
          // Maak en sla een foto op
          if (takeSavePhoto()) {
            Serial.println("Foto succesvol gemaakt en opgeslagen");
          } else {
            Serial.println("Fout bij maken of opslaan van foto");
          }
        }
        lastPhotoTime = currentTime;
      }
    }
  }
  
  // Afhandelen van webserver verzoeken
  handleClientRequests();
  
  // Korte pauze om CPU-gebruik te verminderen
  delay(100);
}