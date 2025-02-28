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
 * Versie: 1.0 (Februari 2025)
 */

#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"
#include "SD_MMC.h"
#include "time.h"
#include <WiFi.h>

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
const char* ssid = "JouwWiFiNaam";
const char* password = "JouwWiFiWachtwoord";

// NTP-server voor synchroniseren van de tijd
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;  // Pas aan naar jouw tijdzone (3600 = GMT+1)
const int daylightOffset_sec = 3600;  // Zomertijd correctie

// Configuratie-instellingen
int photoInterval = 5;  // Tijd tussen foto's in minuten (standaard 5)
int dayStartHour = 8;    // Start tijdstip voor foto's (8:00)
int dayEndHour = 20;     // Eind tijdstip voor foto's (20:00)
int jpegQuality = 10;    // JPEG kwaliteit (0-63, lagere waarde = hogere kwaliteit)
boolean sdCardAvailable = false;

// Buffer voor bestandspaden
char filePath[100];
char folderPath[50];

// Variabelen voor tijdsynchronisatie
unsigned long lastNTPSync = 0;
const unsigned long NTP_SYNC_INTERVAL = 86400000; // Eén keer per dag tijd synchroniseren

// Variabelen voor foto's maken
unsigned long lastPhotoTime = 0;
bool timeInitialized = false;

// Webserver instellen
WiFiServer server(80);

void setup() {
  // Seriële communicatie starten
  Serial.begin(115200);
  Serial.println("ESP32 Plantengroei Timelapse Project");
  
  // Flash LED als uitgang instellen
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  // Extra stap voor SD-kaart herkenning
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  
  // Even wachten op SD-kaart stabilisatie
  delay(500);
  
  // Initialisatie van de camera
  if (!initCamera()) {
    Serial.println("Camera initialisatie mislukt");
    delay(5000);
    ESP.restart();
    return;
  }
  
  // SD-kaart initialiseren met herhaalde pogingen
  for (int attempt = 1; attempt <= 3; attempt++) {
    Serial.printf("SD-kaart initialisatie poging %d/3...\n", attempt);
    sdCardAvailable = initSDCard();
    if (sdCardAvailable) {
      Serial.println("SD-kaart succesvol gedetecteerd!");
      break;
    } else {
      Serial.println("SD-kaart detectie mislukt, nieuwe poging...");
      SD_MMC.end();  // Sluit de huidige SD-kaart sessie
      delay(500);    // Wacht even
    }
  }
  
  if (!sdCardAvailable) {
    Serial.println("WAARSCHUWING: SD-kaart niet beschikbaar na meerdere pogingen");
  }
  
  // WiFi verbinding opzetten
  setupWiFi();
  
  // NTP tijd synchroniseren
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (syncTimeNTP()) {
    timeInitialized = true;
    Serial.println("Tijd gesynchroniseerd");
  } else {
    Serial.println("Tijd synchronisatie mislukt");
  }
  
  // Webserver starten
  server.begin();
  Serial.println("HTTP server gestart");
  
  Serial.println("Setup voltooid. Timelapse actief.");
}

void loop() {
  // Elke dag de tijd synchroniseren
  if (millis() - lastNTPSync > NTP_SYNC_INTERVAL) {
    syncTimeNTP();
    lastNTPSync = millis();
  }
  
  // Controleer of het tijd is voor een nieuwe foto
  unsigned long currentMillis = millis();
  if (timeInitialized && sdCardAvailable && isDay()) {
    if (currentMillis - lastPhotoTime > (photoInterval * 60 * 1000)) {
      // Maak een dagmap als die nog niet bestaat
      if (createDayFolder()) {
        // Maak en sla een foto op
        if (takeSavePhoto()) {
          Serial.println("Foto succesvol gemaakt en opgeslagen");
        } else {
          Serial.println("Fout bij maken of opslaan van foto");
        }
      }
      lastPhotoTime = currentMillis;
    }
  }
  
  // Afhandelen van webserver verzoeken
  handleClient();
  
  // Korte pauze om CPU-gebruik te verminderen
  delay(100);
}

// Initialiseer de camera met de juiste instellingen
bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // Initiële instellingen voor de camera
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA; // 1600x1200 voor hoge kwaliteit 
    config.jpeg_quality = jpegQuality;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera initialiseren
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialisatie mislukt met foutcode 0x%x", err);
    return false;
  }
  
  // Camera instellingen aanpassen voor betere kwaliteit
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);
  s->set_contrast(s, 0);
  s->set_saturation(s, 0);
  s->set_special_effect(s, 0);
  s->set_whitebal(s, 1);
  s->set_awb_gain(s, 1);
  s->set_wb_mode(s, 0);
  s->set_exposure_ctrl(s, 1);
  s->set_aec2(s, 0);
  s->set_gain_ctrl(s, 1);
  s->set_agc_gain(s, 0);
  s->set_gainceiling(s, (gainceiling_t)0);
  s->set_bpc(s, 0);
  s->set_wpc(s, 1);
  s->set_raw_gma(s, 1);
  s->set_lenc(s, 1);
  s->set_dcw(s, 1);
  
  return true;
}

bool initSDCard() {
  // SD-kaart initialiseren in 1-bit modus
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD-kaart initialisatie mislukt!");
    return false;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Geen SD-kaart gedetecteerd");
    return false;
  }
  
  // SD-kaart capaciteit controleren
  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD-kaart gedetecteerd. Type: %d, Grootte: %lluMB\n", cardType, cardSize);
  
  // Verzeker dat de kaart toegankelijk is
  if (cardSize == 0) {
    Serial.println("SD-kaart heeft 0 capaciteit - waarschijnlijk corrupte mount");
    return false;
  }
  
  // Maak een hoofdmap voor de timelapse als die nog niet bestaat
  if (!SD_MMC.exists("/timelapse")) {
    Serial.println("Timelapse map niet gevonden, wordt aangemaakt");
    SD_MMC.mkdir("/timelapse");
  }
  
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

// Maak een foto en sla deze op de SD-kaart op
bool takeSavePhoto() {
  if (!sdCardAvailable) return false;
  
  // Huidige tijd ophalen voor de bestandsnaam
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  // Foto maken
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Foto maken mislukt");
    return false;
  }
  
  // Flash LED aan voor foto (optioneel)
  // digitalWrite(FLASH_LED_PIN, HIGH);
  // delay(100);
  // digitalWrite(FLASH_LED_PIN, LOW);
  
  // Bestandsnaam maken met timestamp: HH-MM-SS.jpg
  sprintf(filePath, "%s/%02d-%02d-%02d.jpg", 
          folderPath, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  
  Serial.printf("Foto opslaan als: %s\n", filePath);
  
  // Bestand openen en schrijven
  File file = SD_MMC.open(filePath, FILE_WRITE);
  if (!file) {
    Serial.println("Bestand openen mislukt");
    esp_camera_fb_return(fb);
    return false;
  }
  
  // Foto naar bestand schrijven
  if (file.write(fb->buf, fb->len)) {
    Serial.printf("Bestand opgeslagen: %s (%u bytes)\n", filePath, fb->len);
  } else {
    Serial.println("Schrijven naar bestand mislukt");
    file.close();
    esp_camera_fb_return(fb);
    return false;
  }
  
  // Bestand sluiten en buffer vrijgeven
  file.close();
  esp_camera_fb_return(fb);
  
  return true;
}

// Controleer of het binnen de ingestelde daguren is
bool isDay() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  
  return (timeinfo.tm_hour >= dayStartHour && timeinfo.tm_hour < dayEndHour);
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

// Stuur een afbeelding direct naar de browser voor weergave
void sendImageFile(WiFiClient client, String filePath) {
  if (!sdCardAvailable || !SD_MMC.exists(filePath)) {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println();
    return;
  }
  
  File file = SD_MMC.open(filePath, FILE_READ);
  if (!file) {
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println();
    return;
  }
  
  // Bestandsgrootte bepalen
  size_t fileSize = file.size();
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.print("Content-Length: ");
  client.println(fileSize);
  client.println("Connection: close");
  client.println();
  
  // Bestand in chunks naar client sturen
  const size_t bufferSize = 1024;
  uint8_t buffer[bufferSize];
  size_t bytesRemaining = fileSize;
  
  while (bytesRemaining > 0) {
    size_t bytesToRead = min(bufferSize, bytesRemaining);
    file.read(buffer, bytesToRead);
    client.write(buffer, bytesToRead);
    bytesRemaining -= bytesToRead;
    yield(); // WiFi-stack tijd geven om te verwerken
  }
  
  file.close();
}

// Web-client verzoeken afhandelen
void handleClient() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  Serial.println("Nieuwe client verbonden");
  String currentLine = "";
  String header = "";
  
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime;
  const unsigned long timeoutTime = 10000; // tijdslimiet voor HTTP verzoeken (10 seconden voor grotere bestanden)
  
  while (client.connected() && currentTime - previousTime <= timeoutTime) {
    currentTime = millis();
    
    if (client.available()) {
      char c = client.read();
      header += c;
      
      if (c == '\n') {
        if (currentLine.length() == 0) {
          // HTTP headers zijn klaar
          
          // Verwerken van dag foto's bekijken
          if (header.indexOf("GET /day/") >= 0) {
            int startPos = header.indexOf("GET /day/") + 9;
            int endPos = header.indexOf(" ", startPos);
            String folderName = header.substring(startPos, endPos);
            String fullPath = "/timelapse/" + folderName;
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>Foto's van " + folderName + "</title>");
            client.println("<style>");
            client.println("body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }");
            client.println("h1, h2 { color: #333; }");
            client.println(".container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }");
            client.println(".btn { display: inline-block; padding: 10px 20px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 4px; margin: 5px; }");
            client.println(".btn-back { background-color: #607D8B; }");
            client.println(".photos { display: flex; flex-wrap: wrap; gap: 10px; margin-top: 20px; }");
            client.println(".photo-item { width: calc(33.333% - 10px); margin-bottom: 20px; box-shadow: 0 0 5px rgba(0,0,0,0.2); border-radius: 4px; overflow: hidden; }");
            client.println(".photo-item img { width: 100%; height: auto; display: block; }");
            client.println(".photo-info { padding: 10px; background-color: white; }");
            client.println(".photo-actions { display: flex; justify-content: space-between; margin-top: 10px; }");
            client.println("@media (max-width: 768px) { .photo-item { width: calc(50% - 10px); } }");
            client.println("@media (max-width: 480px) { .photo-item { width: 100%; } }");
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<div class=\"container\">");
            client.println("<h1>Foto's van " + folderName + "</h1>");
            client.println("<a href=\"/\" class=\"btn btn-back\">Terug naar overzicht</a>");
            
            if (sdCardAvailable) {
              File dir = SD_MMC.open(fullPath);
              if (dir && dir.isDirectory()) {
                client.println("<div class=\"photos\">");
                
                File file = dir.openNextFile();
                int photoCount = 0;
                
                while (file) {
                  if (!file.isDirectory()) {
                    photoCount++;
                    String fileName = String(file.name()).substring(String(file.name()).lastIndexOf('/') + 1);
                    String filePath = String(file.name());
                    
                    client.println("<div class=\"photo-item\">");
                    client.println("<img src=\"/view/timelapse/" + folderName + "/" + fileName + "\" alt=\"" + fileName + "\">");
                    client.println("<div class=\"photo-info\">");
                    client.println("<div>" + fileName + " (" + formatFileSize(file.size()) + ")</div>");
                    client.println("<div class=\"photo-actions\">");
                    client.println("<a href=\"/download/timelapse/" + folderName + "/" + fileName + "\" class=\"btn\">Download</a>");
                    client.println("</div>");
                    client.println("</div>");
                    client.println("</div>");
                  }
                  file = dir.openNextFile();
                }
                
                client.println("</div>");
                
                if (photoCount == 0) {
                  client.println("<p>Geen foto's gevonden in deze map.</p>");
                }
              } else {
                client.println("<p>Map niet gevonden of geen toegang.</p>");
              }
            } else {
              client.println("<p>SD-kaart niet beschikbaar.</p>");
            }
            
            client.println("</div>");
            client.println("</body></html>");
          }
          // Verwerken van individuele foto bekijken
          else if (header.indexOf("GET /view/") >= 0) {
          int startPos = header.indexOf("GET /view/") + 10;
          int endPos = header.indexOf(" ", startPos);
          String relativePath = header.substring(startPos, endPos);
          String filePath = "/" + relativePath;
  
          Serial.println("View aangevraagd voor bestand: " + filePath);
            
            sendImageFile(client, filePath);
          }
          // Verwerken van handmatige foto aanvraag
          else if (header.indexOf("GET /photo") >= 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            // HTML begin
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>Handmatige Foto</title>");
            client.println("<style>body { font-family: Arial, sans-serif; margin: 20px; }");
            client.println(".message { padding: 15px; margin-bottom: 15px; border-radius: 4px; }");
            client.println(".success { background-color: #d4edda; border-left: 6px solid #28a745; }");
            client.println(".error { background-color: #f8d7da; border-left: 6px solid #dc3545; }");
            client.println(".btn { padding: 10px 15px; text-decoration: none; margin: 5px; display: inline-block; border-radius: 4px; background-color: #4CAF50; color: white; }</style></head>");
            client.println("<body><h1>Handmatige Foto</h1>");
            
            // Maak dagmap en neem een foto
            if (sdCardAvailable) {
              if (createDayFolder()) {
                if (takeSavePhoto()) {
                  client.println("<div class=\"message success\">");
                  client.println("<h2>Succes!</h2>");
                  client.println("<p>Foto succesvol gemaakt en opgeslagen.</p>");
                  client.println("<p>Bestandspad: " + String(filePath) + "</p>");
                  client.println("</div>");
                } else {
                  client.println("<div class=\"message error\">");
                  client.println("<h2>Fout</h2>");
                  client.println("<p>Foto maken of opslaan is mislukt.</p>");
                  client.println("</div>");
                }
              } else {
                client.println("<div class=\"message error\">");
                client.println("<h2>Fout</h2>");
                client.println("<p>Kon de dagmap niet aanmaken.</p>");
                client.println("</div>");
              }
            } else {
              client.println("<div class=\"message error\">");
              client.println("<h2>Fout</h2>");
              client.println("<p>SD-kaart niet beschikbaar. Plaats een SD-kaart en herstart de camera.</p>");
              client.println("</div>");
            }
            
            client.println("<a href=\"/\" class=\"btn\">Terug naar het overzicht</a>");
            client.println("</body></html>");
          }
          // Verwerken van live stream aanvraag
          else if (header.indexOf("GET /stream") >= 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
            client.println("Connection: close");
            client.println();
            
            // Een livestream van 30 seconden (kan onderbroken worden door client)
            unsigned long streamStartTime = millis();
            while (client.connected() && (millis() - streamStartTime < 30000)) {
              // Foto maken
              camera_fb_t * fb = esp_camera_fb_get();
              if (!fb) {
                Serial.println("Camera frame capture mislukt");
                break;
              }
              
              client.println("--frame");
              client.println("Content-Type: image/jpeg");
              client.print("Content-Length: ");
              client.println(fb->len);
              client.println();
              client.write(fb->buf, fb->len);
              client.println();
              
              esp_camera_fb_return(fb);
              
              // Korte pauze tussen frames
              delay(100);
            }
          }
          // Verwerken van download aanvraag voor een specifiek bestand
          else if (header.indexOf("GET /download/") >= 0) {
          int startPos = header.indexOf("GET /download/") + 13;
          int endPos = header.indexOf(" ", startPos);
          String relativePath = header.substring(startPos, endPos);
          String filePath = "/" + relativePath;
          
          Serial.println("Download aangevraagd voor bestand: " + filePath);
            
            if (sdCardAvailable && SD_MMC.exists(filePath)) {
              File file = SD_MMC.open(filePath, FILE_READ);
              if (file) {
                // Bestandsgrootte bepalen
                size_t fileSize = file.size();
                
                // Bestandsnaam extraheren
                String fileName = filePath.substring(filePath.lastIndexOf('/') + 1);
                
                client.println("HTTP/1.1 200 OK");
                client.println("Content-Type: image/jpeg");
                client.print("Content-Length: ");
                client.println(fileSize);
                client.println("Content-Disposition: attachment; filename=\"" + fileName + "\"");
                client.println("Connection: close");
                client.println();
                
                // Bestand in chunks naar client sturen
                const size_t bufferSize = 1024;
                uint8_t buffer[bufferSize];
                size_t bytesRemaining = fileSize;
                
                while (bytesRemaining > 0) {
                  size_t bytesToRead = min(bufferSize, bytesRemaining);
                  file.read(buffer, bytesToRead);
                  client.write(buffer, bytesToRead);
                  bytesRemaining -= bytesToRead;
                  yield(); // WiFi-stack tijd geven om te verwerken
                }
                
                file.close();
                Serial.println("Bestand succesvol verzonden: " + fileName);
              } else {
                client.println("HTTP/1.1 404 Not Found");
                client.println("Connection: close");
                client.println();
                Serial.println("Fout bij openen bestand: " + filePath);
              }
            } else {
              client.println("HTTP/1.1 404 Not Found");
              client.println("Connection: close");
              client.println();
              Serial.println("Bestand niet gevonden: " + filePath);
            }
          }
          // Verzoek om de SD-kaart te wissen (alleen de timelapse map)
          else if (header.indexOf("GET /wipe") >= 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>SD-kaart Wissen</title>");
            client.println("<style>body { font-family: Arial, sans-serif; margin: 20px; }");
            client.println(".btn { padding: 10px 15px; text-decoration: none; display: inline-block; border-radius: 4px; background-color: #4CAF50; color: white; margin-top: 20px; }</style>");
            client.println("</head><body>");
            
            if (sdCardAvailable) {
              // Eerst alle mappen/bestanden in de timelapse map wissen
              removeDir("/timelapse");
              
              // Nu de timelapse map opnieuw aanmaken
              SD_MMC.mkdir("/timelapse");
              
              client.println("<h1>SD-kaart succesvol gewist</h1>");
              client.println("<p>Alle timelapse foto's zijn verwijderd.</p>");
            } else {
              client.println("<h1>Fout: SD-kaart niet beschikbaar</h1>");
            }
            
            client.println("<p><a href=\"/\" class=\"btn\">Terug naar het overzicht</a></p>");
            client.println("</body></html>");
          }
          // Bevestigingspagina voor het wissen van de SD-kaart
          else if (header.indexOf("GET /confirmwipe") >= 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>Bevestig SD-kaart Wissen</title>");
            client.println("<style>body { font-family: Arial, sans-serif; margin: 20px; }");
            client.println(".warning { background-color: #ffdddd; border-left: 6px solid #f44336; padding: 15px; margin-bottom: 15px; }");
            client.println(".btn { padding: 10px 15px; text-decoration: none; margin: 5px; display: inline-block; border-radius: 4px; }");
            client.println(".btn-danger { background-color: #f44336; color: white; }");
            client.println(".btn-safe { background-color: #4CAF50; color: white; }</style></head>");
            client.println("<body><h1>SD-kaart Wissen</h1>");
            client.println("<div class=\"warning\"><h2>Waarschuwing!</h2>");
            client.println("<p>Je staat op het punt om alle timelapse foto's van de SD-kaart te wissen.</p>");
            client.println("<p>Deze actie kan niet ongedaan gemaakt worden!</p></div>");
            client.println("<p>Weet je zeker dat je wilt doorgaan?</p>");
            client.println("<a href=\"/wipe\" class=\"btn btn-danger\">Ja, wis alle foto's</a>");
            client.println("<a href=\"/\" class=\"btn btn-safe\">Nee, ga terug</a>");
            client.println("</body></html>");
          }

          // Standaard hoofdpagina met foto overzicht en opties
          else {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<title>MOESTUIN Timelapse</title>");
            client.println("<style>");
            client.println("body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }");
            client.println("h1, h2 { color: #333; }");
            client.println(".container { max-width: 900px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }");
            client.println(".btn { display: inline-block; padding: 10px 20px; background-color: #4CAF50; color: white; text-decoration: none; border-radius: 4px; margin: 5px; }");
            client.println(".btn-warning { background-color: #f44336; }");
            client.println(".btn-info { background-color: #2196F3; }");
            client.println(".btn-primary { background-color: #673AB7; }");
            client.println(".status { margin: 20px 0; padding: 15px; background-color: #e7f3fe; border-left: 6px solid #2196F3; }");
            client.println(".actions { margin: 20px 0; }");
            client.println(".day-list { margin: 20px 0; }");
            client.println(".day-item { padding: 12px; margin-bottom: 8px; background-color: #f9f9f9; border-radius: 4px; display: flex; justify-content: space-between; align-items: center; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }");
            client.println(".day-link { color: #2196F3; text-decoration: none; font-weight: bold; font-size: 1.1em; }");
            client.println(".day-link:hover { text-decoration: underline; }");
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<div class=\"container\">");
            client.println("<h1>Moestuin Plantengroei Timelapse</h1>");
            
            // Status sectie
            client.println("<div class=\"status\">");
            if (sdCardAvailable) {
              client.println("<p>SD-kaart status: <span style=\"color: green;\">Beschikbaar</span></p>");
            } else {
              client.println("<p>SD-kaart status: <span style=\"color: red;\">NIET BESCHIKBAAR</span></p>");
              client.println("<p>Plaats een SD-kaart en herstart de camera.</p>");
            }
            client.println("<p>Foto interval: " + String(photoInterval) + " minuten</p>");
            client.println("<p>Opnametijden: " + String(dayStartHour) + ":00 - " + String(dayEndHour) + ":00</p>");
            
            // Tijd weergeven
            time_t now;
            struct tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            client.println("<p>Huidige tijd: " + String(timeinfo.tm_hour) + ":" + 
                          (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min) + ":" + 
                          (timeinfo.tm_sec < 10 ? "0" : "") + String(timeinfo.tm_sec) + "</p>");
            
            client.println("</div>");
            
            // Acties sectie
            client.println("<div class=\"actions\">");
            client.println("<a href=\"/photo\" class=\"btn btn-primary\">Maak Nu Een Foto</a>");
            client.println("<a href=\"/stream\" target=\"_blank\" class=\"btn btn-info\">Open Live Stream (30 sec)</a>");
            client.println("<a href=\"/confirmwipe\" class=\"btn btn-warning\">Wis SD-kaart</a>");
            client.println("</div>");
            
            // Toon dagen met opnamen
            if (sdCardAvailable) {
              client.println("<h2>Opgenomen Timelapse Foto's</h2>");
              
              File root = SD_MMC.open("/timelapse");
              if (root && root.isDirectory()) {
                // Array van mappen (dagen) creëren
                struct DayFolder {
                  String name;
                  int fileCount;
                };
                
                // Eerst tellen hoeveel mappen er zijn
                int folderCount = 0;
                File entry = root.openNextFile();
                while (entry) {
                  if (entry.isDirectory()) {
                    folderCount++;
                  }
                  entry.close();
                  entry = root.openNextFile();
                }
                
                // Als er mappen zijn, toon ze
                if (folderCount > 0) {
                  // Array voor mappen maken
                  DayFolder days[folderCount];
                  
                  // Reset directory pointer
                  root.close();
                  root = SD_MMC.open("/timelapse");
                  
                  // Mappen lezen en aantal bestanden per map tellen
                  int dayIndex = 0;
                  entry = root.openNextFile();
                  while (entry && dayIndex < folderCount) {
                    if (entry.isDirectory()) {
                      String dirName = String(entry.name()).substring(String(entry.name()).lastIndexOf('/') + 1);
                      days[dayIndex].name = dirName;
                      
                      // Tel aantal bestanden in deze map
                      File dayDir = SD_MMC.open(String(entry.name()));
                      int fileCount = 0;
                      File dayFile = dayDir.openNextFile();
                      while (dayFile) {
                        if (!dayFile.isDirectory()) {
                          fileCount++;
                        }
                        dayFile.close();
                        dayFile = dayDir.openNextFile();
                      }
                      dayDir.close();
                      
                      days[dayIndex].fileCount = fileCount;
                      dayIndex++;
                    }
                    entry.close();
                    entry = root.openNextFile();
                  }
                  
                  // Toon de dagen in de UI, nieuwste eerst
                  client.println("<div class=\"day-list\">");
                  for (int i = folderCount - 1; i >= 0; i--) {
                    client.println("<div class=\"day-item\">");
                    client.println("<a href=\"/day/" + days[i].name + "\" class=\"day-link\">" + days[i].name + "</a>");
                    client.println("<span>" + String(days[i].fileCount) + " foto's</span>");
                    client.println("</div>");
                  }
                  client.println("</div>");
                } else {
                  client.println("<p>Geen timelapse opnamen gevonden.</p>");
                }
              } else {
                client.println("<p>Timelapse map niet gevonden of leeg.</p>");
              }
            }
            
            client.println("</div>"); // container einde
            client.println("</body></html>");
          }
          
          break;
        } else {
          currentLine = "";
        }
      } else if (c != '\r') {
        currentLine += c;
      }
    }
  }
  
  // Verbinding sluiten
  header = "";
  client.stop();
  Serial.println("Client verbinding verbroken");
}
