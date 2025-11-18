#include "display.h"
#include "config.h"
#include "variables.h"
#include "wifi_manager.h"
#include "events.h"
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_SCL 9
#define I2C_SDA 8

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static unsigned long lastUpdate = 0;
static bool showWiFiInfo = false;
static unsigned long wifiInfoTimer = 0;
static bool forceUpdate = false;
extern unsigned long coolerTimerStart;

void initDisplay() {
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    DEBUG_PRINTLN("[ERR] OLED not found!");
    return;
  }

  // Flip the display (rotate 180 degrees)
  display.setRotation(2);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.println("OLED READY");
  display.display();

  DEBUG_PRINTLN("[INIT] OLED initialized.");
}

void showWiFiInfoScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  display.setCursor(0, 0);
  display.println("WiFi Information");
  display.println("----------------");
  
  if (getWiFiMode() == WIFI_AP_MODE) {
    // Hotspot mode
    display.println("Mode: Hotspot");
    display.print("SSID: ");
    display.println(AP_SSID);
    display.print("Pass: ");
    display.println(AP_PASSWORD);
  } else if (isWiFiConnected()) {
    // Connected to WiFi
    display.println("Mode: Connected");
    display.print("SSID: ");
    
    // Get the actual SSID
    String connectedSSID = WiFi.SSID();
    if (connectedSSID.length() > 0) {
      // Truncate if too long (max ~16 chars per line on small font)
      if (connectedSSID.length() > 16) {
        display.println(connectedSSID.substring(0, 16));
      } else {
        display.println(connectedSSID);
      }
    } else {
      display.println("Unknown");
    }
  } else if (isWiFiConnecting()) {
    // Currently connecting
    display.println("Mode: Connecting");
    display.println("Please wait...");
  } else {
    // WiFi disabled or failed
    display.println("Mode: Offline");
    display.println("Hold TOP 10s");
    display.println("to start hotspot");
  }
  
  display.print("IP: ");
  display.println(getIPAddress());
  
  display.display();
}

void showRealtimeData() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // --- TOP LINES: depends on mode ---
  if (mode == 1) {  // Auto
    // Show configured thresholds (no decimals to save space)
    display.setCursor(0, 0);
    display.printf("T:%d-%dC H:%d-%d%%",
                   (int)lower_temp_threshold, (int)upper_temp_threshold,
                   (int)lower_hum_threshold, (int)upper_hum_threshold);

    // Show calculated thresholds (ambient-aware)
    display.setCursor(0, 9);
    display.printf("NT:%d-%dC NH:%d-%d%%",
                   (int)calculated_lower_temp, (int)calculated_upper_temp,
                   (int)calculated_lower_hum, (int)calculated_upper_hum);
  }
  else if (mode == 3) {  // Timer mode
    unsigned long elapsedSec = (millis() - coolerTimerStart) / 1000UL;
    unsigned long totalSec = timer_value * 60UL;
    unsigned long remainingSec = (elapsedSec < totalSec) ? (totalSec - elapsedSec) : 0;

    int hh = remainingSec / 3600;
    int mm = (remainingSec % 3600) / 60;
    int ss = remainingSec % 60;

    display.setCursor(0, 0);
    display.printf("Rem: %02d:%02d:%02d", hh, mm, ss);
  }
  else {
    // Manual: show mode info
    display.setCursor(0, 0);
    display.print("Manual Control");
  }

  // --- Mode & Priority ---
  display.setCursor(0, 19);
  if (mode == 3) {
    display.printf("Mode:TIMER  %s\n", relay_state ? "ON " : "OFF");
  } else if (mode == 1) {
    display.printf("Mode:AUTO(%s)   %s\n",
                   priority == "Temperature" ? "TEMP" :
                   priority == "Humidity" ? "HUM" : "BOTH",
                   relay_state ? "ON " : "OFF");
  } else {
    display.printf("Mode:MANUAL  %s\n", relay_state ? "ON " : "OFF");
  }


  // --- Internal conditions ---
  display.setCursor(0, 30);
  display.printf("Int T:%dC H:%d%%", (int)internal_temp, (int)internal_hum);

  // --- Ambient conditions ---
  display.setCursor(0, 40);
  display.printf("Amb T:%dC H:%d%%", (int)ambient_temp, (int)ambient_hum);

  // --- Bottom indicator ---
  display.setCursor(0, 52);
  // Show protection status in Auto/Timer modes
  extern bool isProtectionActive();
  extern unsigned long getProtectionRemaining();
  
  if (isProtectionActive()) {
    unsigned long remaining = getProtectionRemaining();
    display.printf("PROTECT:%lus", remaining);
  }

  display.display();
  lastUpdate = millis();
}

void showEditScreen(const String& label, float value) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Header
  display.setTextSize(1);
  display.setCursor(25, 0);
  display.println("EDIT MODE");

  // Label
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.printf("%s:", label.c_str());

  // Large Value
  display.setTextSize(2);
  display.setCursor(10, 38);
  display.printf("%.1f", value);
  
  // Instructions
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print("Up/Dn +/- MID=OK");
  
  display.display();
}

void processDisplayEvents() {
  SystemEvent event;
  
  while (!eventQueue.isEmpty()) {
    if (eventQueue.pop(event)) {
      switch (event.type) {
        case EVENT_RELAY_CHANGED:
          DEBUG_PRINTF("[DISPLAY] Relay changed: %s (source:%d)\n", 
                       event.relay_state ? "ON" : "OFF",
                       event.relay_source);
          forceUpdate = true;
          break;
          
        case EVENT_MODE_CHANGED:
          DEBUG_PRINTF("[DISPLAY] Mode changed: %d (source:%d)\n",
                       event.mode_value,
                       event.mode_source);
          forceUpdate = true;
          break;
          
        case EVENT_PRIORITY_CHANGED:
          DEBUG_PRINTF("[DISPLAY] Priority changed: %s\n",
                       event.priority_value);
          forceUpdate = true;
          break;
          
        case EVENT_THRESHOLDS_CHANGED:
          DEBUG_PRINTLN("[DISPLAY] Thresholds changed");
          forceUpdate = true;
          break;
          
        case EVENT_TIMER_CHANGED:
          DEBUG_PRINTLN("[DISPLAY] Timer changed");
          forceUpdate = true;
          break;
          
        case EVENT_EDIT_MODE_CHANGED:
          DEBUG_PRINTF("[DISPLAY] Edit mode: %s\n",
                       event.edit_entering ? "ENTER" : "EXIT");
          // Display already updated by buttons.cpp
          break;
          
        case EVENT_SENSOR_UPDATE:
          // Sensors update frequently, don't log
          break;
          
        default:
          break;
      }
    }
  }
}

void updateDisplayIfNeeded() {
  // Process any pending events first
  processDisplayEvents();
  
  // Check if WiFi info should be shown
  if (showWiFiInfo) {
    if (millis() - wifiInfoTimer > 5000) {  // Show for 5 seconds
      showWiFiInfo = false;
      forceUpdate = true;
    } else {
      showWiFiInfoScreen();
      return;
    }
  }
  
  // Update display if forced or periodic interval reached (but not in edit mode)
  if (!edit_mode && (forceUpdate || millis() - lastUpdate > 1500)) {
    showRealtimeData();
    forceUpdate = false;
  }
}

void toggleWiFiInfo() {
  showWiFiInfo = !showWiFiInfo;
  wifiInfoTimer = millis();
  if (showWiFiInfo) {
    showWiFiInfoScreen();
  } else {
    showRealtimeData();
  }
}

void forceDisplayUpdate() {
  forceUpdate = true;
}