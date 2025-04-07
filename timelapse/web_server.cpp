#include "web_server.h"
#include "web_handlers.h"
#include "web_utils.h"

// Webserver instantie
WiFiServer server(80);

// Start de webserver
void startWebServer() {
  server.begin();
  Serial.println("HTTP server gestart");
  Serial.print("Je kunt de interface benaderen op: http://");
  Serial.println(WiFi.localIP());
  
  // Initialiseer handlers
  initializeWebHandlers();
}

// Web-client verzoeken afhandelen
void handleClientRequests() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  Serial.println("Nieuwe client verbonden");
  String currentLine = "";
  String header = "";
  
  unsigned long currentTime = millis();
  unsigned long previousTime = currentTime;
  const unsigned long timeoutTime = 10000; // tijdslimiet voor HTTP verzoeken
  
  while (client.connected() && currentTime - previousTime <= timeoutTime) {
    currentTime = millis();
    
    if (client.available()) {
      char c = client.read();
      header += c;
      
      if (c == '\n') {
        if (currentLine.length() == 0) {
          // HTTP headers zijn klaar, verwerk het verzoek
          
          // Root pagina
          if (header.indexOf("GET / ") >= 0) {
            handleRootPage(client);
          }
          // Dag foto's bekijken
          else if (header.indexOf("GET /day/") >= 0) {
            String folderName = extractPathParameter(header, "GET /day/");
            handleDayView(client, folderName);
          }
          // Individuele foto bekijken
          else if (header.indexOf("GET /view/") >= 0) {
            String relativePath = extractPathParameter(header, "GET /view/");
            handleImageView(client, relativePath);
          }
          // Snapshot endpoint
          else if (header.indexOf("GET /snapshot") >= 0) {
            handleSnapshot(client);
          }
          // Handmatige foto maken
          else if (header.indexOf("GET /photo") >= 0) {
            handlePhoto(client);
          }
          // Live stream
          else if (header.indexOf("GET /stream") >= 0) {
            handleStream(client);
          }
          // Download foto
          else if (header.indexOf("GET /download/") >= 0) {
            String relativePath = extractPathParameter(header, "GET /download/");
            handleDownload(client, relativePath);
          }
          // Wis SD-kaart
          else if (header.indexOf("GET /wipe") >= 0) {
            handleWipe(client);
          }
          // Bevestig wissen
          else if (header.indexOf("GET /confirmwipe") >= 0) {
            handleConfirmWipe(client);
          }
          // Iframe view voor dashboard
          else if (header.indexOf("GET /iframe") >= 0) {
            handleIframeView(client);
          }
          // Instellingen opslaan
          else if (header.indexOf("POST /savesettings") >= 0) {
            // Verbeterde afhandeling van POST data
            
            // Zoek naar Content-Length header om de lengte van de POST body te bepalen
            int contentLength = 0;
            int contentLengthPos = header.indexOf("Content-Length: ");
            if (contentLengthPos > 0) {
              int endPos = header.indexOf("\r\n", contentLengthPos);
              if (endPos > 0) {
                contentLength = header.substring(contentLengthPos + 16, endPos).toInt();
                Serial.println("Content-Length: " + String(contentLength));
              }
            }
            
            // Vind de start van de POST body
            String body = "";
            int bodyStart = header.indexOf("\r\n\r\n");
            if (bodyStart > 0) {
              // Alles na de headers is de body
              body = header.substring(bodyStart + 4);
              
              // Als we nog niet alle data hebben ontvangen
              if (body.length() < contentLength) {
                Serial.println("Wachten op meer POST data...");
                unsigned long postTimeout = millis() + 5000; // 5 seconden timeout
                
                // Blijf lezen tot we alle data hebben of tot de timeout
                while (body.length() < contentLength && millis() < postTimeout && client.connected()) {
                  if (client.available()) {
                    char c = client.read();
                    body += c;
                  }
                  yield(); // Geef de WiFi stack tijd
                }
              }
            }
            
            Serial.println("Ontvangen POST data: " + body);
            Serial.println("Data lengte: " + String(body.length()) + " bytes");
            
            handleSaveSettings(client, body);
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