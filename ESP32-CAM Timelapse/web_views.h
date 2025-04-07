#ifndef WEB_VIEWS_H
#define WEB_VIEWS_H

#include "config.h"
#include <WiFi.h>

// Declaraties voor HTML-generatiefuncties
void generateStatusSection(WiFiClient& client);
void generatePhotosTab(WiFiClient& client);
void generateSettingsForm(WiFiClient& client);
void generateSuccessPage(WiFiClient& client, String title, String message, String redirectPath = "/");
void generateErrorPage(WiFiClient& client, String title, String message);
void generateSinglePhotoView(WiFiClient& client, String photoPath, String photoName, String backLink);

#endif // WEB_VIEWS_H