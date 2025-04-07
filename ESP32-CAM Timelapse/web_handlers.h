#ifndef WEB_HANDLERS_H
#define WEB_HANDLERS_H

#include "config.h"
#include <WiFi.h>

// Declaraties voor endpoint-handlers
void handleRootPage(WiFiClient& client);
void handleDayView(WiFiClient& client, String folderName);
void handleImageView(WiFiClient& client, String relativePath);
void handlePhoto(WiFiClient& client);
void handleStream(WiFiClient& client);
void handleDownload(WiFiClient& client, String relativePath);
void handleWipe(WiFiClient& client);
void handleConfirmWipe(WiFiClient& client);
void handleIframeView(WiFiClient& client);
void handleSaveSettings(WiFiClient& client, String body);
void handleSnapshot(WiFiClient& client);

// Initialisatiefunctie
void initializeWebHandlers();

#endif // WEB_HANDLERS_H