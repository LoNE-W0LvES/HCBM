// web_config.h
#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <Arduino.h>

void startWebServer();
void updateWebServer();
void processWebEvents();
void notifyClients(const String& message);
int getWebSocketClientCount();  // Get number of connected WebSocket clients

#endif