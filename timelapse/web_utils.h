#ifndef WEB_UTILS_H
#define WEB_UTILS_H

#include "config.h"
#include <WiFi.h>
#include <map>

// Declaraties voor hulpfuncties
void sendHttpHeaders(WiFiClient& client, String contentType = "text/html");
String extractPathParameter(String header, String prefix);
String extractFormValue(String body, String name);
void processSettingsForm(String body);
String urlDecode(String input);
void parseQueryParams(String url, std::map<String, String>& params);
void sendImageFile(WiFiClient client, String filePath);
bool startsWith(String str, String prefix);
String httpDate();
String getMimeType(String filename);

#endif // WEB_UTILS_H