#include "web_utils.h"
#include "settings_manager.h"
#include "camera.h"
#include "sd_card.h"

// Stuur standaard HTTP headers
void sendHttpHeaders(WiFiClient& client, String contentType) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: " + contentType);
  client.println("Connection: close");
  client.println();
}

// Extraheer een padparameter uit de HTTP-header
String extractPathParameter(String header, String prefix) {
  int startPos = header.indexOf(prefix) + prefix.length();
  int endPos = header.indexOf(" ", startPos);
  return header.substring(startPos, endPos);
}

// Haal een waarde uit een formulier
String extractFormValue(String body, String name) {
  String searchPattern = name + "=";
  int pos = body.indexOf(searchPattern);
  if (pos < 0) {
    Serial.println("Parameter niet gevonden: " + name);
    return "";
  }
  
  pos += searchPattern.length();
  
  int endPos = body.indexOf("&", pos);
  if (endPos < 0) endPos = body.length();
  
  String value = body.substring(pos, endPos);
  // Debug output
  Serial.println("Ruwe waarde voor " + name + ": " + value);
  
  // URL decoding
  value = urlDecode(value);
  Serial.println("Gedecodeerde waarde voor " + name + ": " + value);
  
  return value;
}

// Verwerk formulierdata vanaf het instellingenformulier
void processSettingsForm(String body) {
  // Debug informatie
  Serial.println("Formulierdata verwerken: " + body);
  
  // URL-gecodeerde parameters uit het formulier halen
  String intervalStr = extractFormValue(body, "photoInterval");
  String startHourStr = extractFormValue(body, "dayStartHour");
  String endHourStr = extractFormValue(body, "dayEndHour");
  String qualityStr = extractFormValue(body, "jpegQuality");
  
  Serial.println("Geëxtraheerde waarden:");
  Serial.println("- interval: '" + intervalStr + "'");
  Serial.println("- startHour: '" + startHourStr + "'");
  Serial.println("- endHour: '" + endHourStr + "'");
  Serial.println("- quality: '" + qualityStr + "'");
  
  // Converteer naar integers en valideer de waardes
  int interval = intervalStr.toInt();
  int startHour = startHourStr.toInt();
  int endHour = endHourStr.toInt();
  int quality = qualityStr.toInt();
  
  // Validatie
  if (interval >= 1 && interval <= 60) {
    photoInterval = interval;
    Serial.println("Nieuwe foto interval: " + String(photoInterval) + " minuten");
  } else {
    Serial.println("Ongeldige interval waarde: " + String(interval));
  }
  
  if (startHour >= 0 && startHour <= 23) {
    dayStartHour = startHour;
    Serial.println("Nieuwe start uur: " + String(dayStartHour));
  } else {
    Serial.println("Ongeldige startuur waarde: " + String(startHour));
  }
  
  if (endHour >= 0 && endHour <= 23) {
    dayEndHour = endHour;
    Serial.println("Nieuwe eind uur: " + String(dayEndHour));
  } else {
    Serial.println("Ongeldige einduur waarde: " + String(endHour));
  }
  
  if (quality >= 10 && quality <= 63) {
    jpegQuality = quality;
    Serial.println("Nieuwe JPEG kwaliteit: " + String(jpegQuality));
  } else {
    Serial.println("Ongeldige kwaliteitswaarde: " + String(quality));
  }
  
  // Sla instellingen op in flash
  saveSettings();
}

// Decodeer URL-encoded string
String urlDecode(String input) {
  String output = "";
  for (int i = 0; i < input.length(); i++) {
    if (input[i] == '%') {
      if (i + 2 < input.length()) {
        int hexValue = 0;
        for (int j = 1; j <= 2; j++) {
          char c = input[i + j];
          if (c >= '0' && c <= '9') {
            hexValue = hexValue * 16 + (c - '0');
          } else if (c >= 'a' && c <= 'f') {
            hexValue = hexValue * 16 + (c - 'a' + 10);
          } else if (c >= 'A' && c <= 'F') {
            hexValue = hexValue * 16 + (c - 'A' + 10);
          }
        }
        output += (char)hexValue;
        i += 2;
      } else {
        output += input[i];
      }
    } else if (input[i] == '+') {
      output += ' ';
    } else {
      output += input[i];
    }
  }
  return output;
}

// Parse query parameters van een URL
void parseQueryParams(String url, std::map<String, String>& params) {
  int questionMarkPos = url.indexOf('?');
  if (questionMarkPos < 0) return; // Geen query parameters
  
  String queryString = url.substring(questionMarkPos + 1);
  int startPos = 0;
  int ampersandPos;
  
  do {
    ampersandPos = queryString.indexOf('&', startPos);
    String param;
    
    if (ampersandPos < 0) {
      param = queryString.substring(startPos);
    } else {
      param = queryString.substring(startPos, ampersandPos);
      startPos = ampersandPos + 1;
    }
    
    int equalsPos = param.indexOf('=');
    if (equalsPos >= 0) {
      String name = param.substring(0, equalsPos);
      String value = param.substring(equalsPos + 1);
      params[name] = urlDecode(value);
    }
    
  } while (ampersandPos >= 0);
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

// Controleer of een string begint met een bepaald patroon
bool startsWith(String str, String prefix) {
  if (str.length() < prefix.length()) {
    return false;
  }
  return str.substring(0, prefix.length()).equals(prefix);
}

// Genereer een HTTP datum header
String httpDate() {
  time_t now;
  struct tm timeinfo;
  
  time(&now);
  gmtime_r(&now, &timeinfo);
  
  char buf[50];
  strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &timeinfo);
  return String(buf);
}

// Zet een bestandsextensie om naar een MIME-type
String getMimeType(String filename) {
  if (filename.endsWith(".html") || filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".json")) {
    return "application/json";
  }
  return "application/octet-stream";
}