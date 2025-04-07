#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"
#include <WiFi.h>

// Webserver instantie
extern WiFiServer server;

// Basis webserver functionaliteit
void startWebServer();
void handleClientRequests();

#endif // WEB_SERVER_H