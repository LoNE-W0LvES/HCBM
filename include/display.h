// display.h
#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void initDisplay();
void showRealtimeData();
void showEditScreen(const String& label, float value);
void updateDisplayIfNeeded();
void toggleWiFiInfo();
void forceDisplayUpdate();
void processDisplayEvents();

#endif