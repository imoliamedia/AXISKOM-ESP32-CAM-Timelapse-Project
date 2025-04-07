#include "web_handlers.h"
#include "html_templates.h"
#include "web_views.h"
#include "web_utils.h"
#include "camera.h"
#include "sd_card.h"
#include "settings_manager.h"
#include "time_manager.h"

void initializeWebHandlers() {
  // Placeholder voor eventuele initialisatie van handlers
}

// Handler voor de hoofdpagina
void handleRootPage(WiFiClient& client) {
  sendHttpHeaders(client);
  
  // Start van de hoofdpagina
  client.println(MAIN_PAGE_HTML_START);
  client.println(CSS_STYLES);
  client.println(MAIN_PAGE_HTML_MIDDLE);
  
  // Status sectie invullen
  generateStatusSection(client);
  
  client.println(MAIN_PAGE_HTML_ACTIONS);
  
  // Foto's tabblad
  generatePhotosTab(client);
  
  // Instellingen tabblad
  generateSettingsForm(client);
  
  // Einde van de hoofdpagina
  client.println(MAIN_PAGE_HTML_END);
  client.println(TABS_SCRIPT);
  client.println(MAIN_PAGE_HTML_FINAL);
}

// Handler voor het bekijken van een specifieke dag
void handleDayView(WiFiClient& client, String folderName) {
  String fullPath = "/timelapse/" + folderName;
  
  sendHttpHeaders(client);
  
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>Foto's van " + folderName + "</title>");
  client.println("<style>");
  client.println(CSS_STYLES);
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

// Handler voor het bekijken van een afbeelding
void handleImageView(WiFiClient& client, String relativePath) {
  String filePath = "/" + relativePath;
  Serial.println("View aangevraagd voor bestand: " + filePath);
  
  sendImageFile(client, filePath);
}

// Handler voor het maken van een handmatige foto
void handlePhoto(WiFiClient& client) {
  sendHttpHeaders(client);
  
  // HTML begin
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>Handmatige Foto</title>");
  client.println("<style>");
  client.println(CSS_STYLES);
  client.println("</style></head>");
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

// Handler voor live stream
void handleStream(WiFiClient& client) {
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

// Handler voor het downloaden van een bestand
void handleDownload(WiFiClient& client, String relativePath) {
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

// Handler voor het wissen van de SD-kaart
void handleWipe(WiFiClient& client) {
  sendHttpHeaders(client);
  
  client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>SD-kaart Wissen</title>");
  client.println("<style>" + String(CSS_STYLES) + "</style>");
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

// Handler voor de bevestigingspagina voor wissen
void handleConfirmWipe(WiFiClient& client) {
  sendHttpHeaders(client);
  
  client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>Bevestig SD-kaart Wissen</title>");
  client.println("<style>" + String(CSS_STYLES) + "</style>");
  client.println("<body><h1>SD-kaart Wissen</h1>");
  client.println("<div class=\"warning\"><h2>Waarschuwing!</h2>");
  client.println("<p>Je staat op het punt om alle timelapse foto's van de SD-kaart te wissen.</p>");
  client.println("<p>Deze actie kan niet ongedaan gemaakt worden!</p></div>");
  client.println("<p>Weet je zeker dat je wilt doorgaan?</p>");
  client.println("<a href=\"/wipe\" class=\"btn btn-warning\">Ja, wis alle foto's</a>");
  client.println("<a href=\"/\" class=\"btn\">Nee, ga terug</a>");
  client.println("</body></html>");
}

// Handler voor iframe modus (voor dashboard integratie)
void handleIframeView(WiFiClient& client) {
  // Status content voorbereiden
  String statusContent = "";
  
  // SD-kaart status
  if (sdCardAvailable) {
    statusContent += "<p>SD-kaart: <span style=\"color: green;\">OK</span> | ";
  } else {
    statusContent += "<p>SD-kaart: <span style=\"color: red;\">Geen</span> | ";
  }
  
  // Tijd weergeven
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  statusContent += "Tijd: " + String(timeinfo.tm_hour) + ":" + 
               (timeinfo.tm_min < 10 ? "0" : "") + String(timeinfo.tm_min) + "</p>";
  
  // Iframe HTML templaten gebruiken
  String iframeHTML = String(IFRAME_HTML);
  iframeHTML.replace("{status_content}", statusContent);
  
  // HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("X-Frame-Options: ALLOWALL");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Content-Security-Policy: frame-ancestors 'self' *");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  // Stuur HTML
  client.println(iframeHTML);
}

// Handler voor het opslaan van instellingen
void handleSaveSettings(WiFiClient& client, String body) {
  // Debug info
  Serial.println("Ontvangen formulierdata: " + body);
  
  // Instellingen verwerken
  processSettingsForm(body);
  
  // Bevestigingspagina tonen met de NIEUWE waarden
  sendHttpHeaders(client);
  
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=\"refresh\" content=\"5;url=/\">"); // 5 seconden voor de extra debugging info
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>Instellingen Opgeslagen</title>");
  client.println("<style>" + String(CSS_STYLES) + "</style>");
  client.println("</head><body>");
  client.println("<div class=\"message success\">");
  client.println("<h2>Succes!</h2>");
  client.println("<p>Instellingen zijn succesvol opgeslagen.</p>");
  client.println("<p>Foto interval: " + String(photoInterval) + " minuten</p>");
  client.println("<p>Opnametijden: " + String(dayStartHour) + ":00 - " + String(dayEndHour) + ":00</p>");
  client.println("<p>JPEG kwaliteit: " + String(jpegQuality) + "</p>");
  client.println("<p>Je wordt automatisch teruggeleid naar de hoofdpagina...</p>");
  client.println("</div>");
  client.println("</body></html>");
}

// Handler voor snapshot (momentopname)
void handleSnapshot(WiFiClient& client) {
  // Maak een foto en stuur deze direct naar de client
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    client.println("HTTP/1.1 500 Internal Server Error");
    client.println("Connection: close");
    client.println();
    Serial.println("Camera frame capture mislukt");
    return;
  }
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: image/jpeg");
  client.println("Cache-Control: no-cache, no-store, must-revalidate");
  client.println("Pragma: no-cache");
  client.println("Expires: 0");
  client.print("Content-Length: ");
  client.println(fb->len);
  client.println("Connection: close");
  client.println();
  
  client.write(fb->buf, fb->len);
  esp_camera_fb_return(fb);
}