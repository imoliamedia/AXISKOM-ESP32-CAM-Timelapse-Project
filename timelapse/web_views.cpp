#include "web_views.h"
#include "html_templates.h"
#include "time_manager.h"
#include "settings_manager.h"
#include "sd_card.h"

// Genereer de statussectie voor de hoofdpagina
void generateStatusSection(WiFiClient& client) {
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
}

// Genereer het foto's tabblad
void generatePhotosTab(WiFiClient& client) {
  client.println("<div id=\"photos-tab\" class=\"tab-content\">");
  client.println("<h2>Opgenomen Timelapse Foto's</h2>");
  
  // Toon dagen met opnamen
  if (sdCardAvailable) {
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
  
  client.println("</div>"); // einde foto's tabblad
}

// Genereer het instellingen formulier
void generateSettingsForm(WiFiClient& client) {
  String settingsHTML = String(SETTINGS_TAB_HTML);
  settingsHTML.replace("{photoInterval}", String(photoInterval));
  settingsHTML.replace("{dayStartHour}", String(dayStartHour));
  settingsHTML.replace("{dayEndHour}", String(dayEndHour));
  settingsHTML.replace("{jpegQuality}", String(jpegQuality));
  client.println(settingsHTML);
}

// Genereer een succes/bevestigingspagina
void generateSuccessPage(WiFiClient& client, String title, String message, String redirectPath) {
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  
  if (redirectPath != "") {
    client.println("<meta http-equiv=\"refresh\" content=\"2;url=" + redirectPath + "\">");
  }
  
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>" + title + "</title>");
  client.println("<style>" + String(CSS_STYLES) + "</style>");
  client.println("</head><body>");
  client.println("<div class=\"message success\">");
  client.println("<h2>" + title + "</h2>");
  client.println("<p>" + message + "</p>");
  
  if (redirectPath != "") {
    client.println("<p>Je wordt automatisch teruggeleid...</p>");
  }
  
  client.println("</div>");
  
  if (redirectPath != "") {
    client.println("<a href=\"" + redirectPath + "\" class=\"btn\">Direct doorgaan</a>");
  }
  
  client.println("</body></html>");
}

// Genereer een foutpagina
void generateErrorPage(WiFiClient& client, String title, String message) {
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>" + title + "</title>");
  client.println("<style>" + String(CSS_STYLES) + "</style>");
  client.println("</head><body>");
  client.println("<div class=\"message error\">");
  client.println("<h2>" + title + "</h2>");
  client.println("<p>" + message + "</p>");
  client.println("</div>");
  client.println("<a href=\"/\" class=\"btn\">Terug naar het overzicht</a>");
  client.println("</body></html>");
}

// Genereer een pagina voor het bekijken van een enkele foto
void generateSinglePhotoView(WiFiClient& client, String photoPath, String photoName, String backLink) {
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<meta http-equiv=\"Content-Security-Policy\" content=\"frame-ancestors 'self' *\">");
  client.println("<title>Bekijk foto: " + photoName + "</title>");
  client.println("<style>" + String(CSS_STYLES) + "</style>");
  client.println("</head><body>");
  client.println("<div class=\"container\">");
  client.println("<h1>Foto: " + photoName + "</h1>");
  client.println("<a href=\"" + backLink + "\" class=\"btn btn-back\">Terug</a>");
  client.println("<div style=\"margin: 20px 0; text-align: center;\">");
  client.println("<img src=\"" + photoPath + "\" style=\"max-width: 100%; max-height: 80vh; border-radius: 5px; box-shadow: 0 0 10px rgba(0,0,0,0.2);\">");
  client.println("</div>");
  client.println("<div style=\"display: flex; justify-content: center; gap: 10px;\">");
  client.println("<a href=\"/download" + photoPath + "\" class=\"btn\">Download</a>");
  client.println("</div>");
  client.println("</div>");
  client.println("</body></html>");
}