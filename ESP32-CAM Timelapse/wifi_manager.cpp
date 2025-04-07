#include "wifi_manager.h"

// WiFi-instellingen - Pas deze aan naar je eigen netwerk
const char* ssid = "JouwWiFiNaam";
const char* password = "JouwWiFiWachtwoord";

// WiFi-verbinding opzetten
void setupWiFi() {
  Serial.printf("Verbinden met WiFi netwerk: %s\n", ssid);
  
  WiFi.begin(ssid, password);
  
  // Wacht maximaal 20 seconden op WiFi-verbinding
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi verbonden. IP-adres: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("WiFi verbinding mislukt. Camera werkt offline door.");
  }
}