#ifndef HTML_PAGES_H
#define HTML_PAGES_H

#include <Arduino.h>

// Function declarations
String getModeOptionsHTML();
String getPriorityOptionsHTML();

// WiFi Setup Page
extern const char WIFI_SETUP_page[] PROGMEM;

// Configuration Page
extern const char CONFIG_page[] PROGMEM;

// Main Dashboard Page
extern const char MAIN_page[] PROGMEM;

#endif