#include <Arduino.h>
#include "config.h"
#include "sensor_read.h"
#include "control_logic.h"
#include "web_config.h"
#include "display.h"
#include "buttons.h"
#include "variables.h"
#include "storage_manager.h"
#include "wifi_manager.h"
#include "events.h"
#include "web_state.h"

bool manual_override = false;

float internal_temp = 0, internal_hum = 0, ambient_temp = 0, ambient_hum = 0;
float upper_temp_threshold = 35, lower_temp_threshold = 25;
float upper_hum_threshold = 75, lower_hum_threshold = 45;

float lower_temp = 25, lower_hum = 45;

// Calculated thresholds (ambient-aware)
float calculated_upper_temp = 35;
float calculated_lower_temp = 25;
float calculated_upper_hum = 75;
float calculated_lower_hum = 45;

int mode = 1;
String priority = "Temperature";
int timer_value = 10;
bool timer_active = false;
bool switch_manual = false;
bool relay_state = false;
int current_menu = 1;
bool edit_mode = false;

unsigned long lastLog = 0;
unsigned long lastSensorRead = 0;
unsigned long lastPerfLog = 0;

// Web server state tracking
bool webServerStarted = false;
bool wifiConnectionAttempted = false;

void setup() {
  // Initialize debug serial first
  #if DEBUG_MODE
    Serial.begin(115200);
    while (!Serial && millis() < 2000) delay(10);
  #endif
  
  delay(500);
  
  DEBUG_PRINTLN("\n╔═══════════════════════════════╗");
  DEBUG_PRINTLN("║  Smart Cooler v2.4             ║");
  DEBUG_PRINTLN("║  BME280 + Tolerance Control    ║");
  DEBUG_PRINTLN("╚═══════════════════════════════╝\n");
  
  DEBUG_PRINTLN("[CONFIG] Mode Configuration:");
  #if ENABLE_AUTO_MODE
    DEBUG_PRINTLN("  ✓ Auto Mode: ENABLED");
  #else
    DEBUG_PRINTLN("  ✗ Auto Mode: DISABLED");
  #endif
  
  #if ENABLE_MANUAL_MODE
    DEBUG_PRINTLN("  ✓ Manual Mode: ENABLED");
  #else
    DEBUG_PRINTLN("  ✗ Manual Mode: DISABLED");
  #endif
  
  #if ENABLE_TIMER_MODE
    DEBUG_PRINTLN("  ✓ Timer Mode: ENABLED");
  #else
    DEBUG_PRINTLN("  ✗ Timer Mode: DISABLED");
  #endif
  
  DEBUG_PRINTF("  Default Mode: %d (%s)\n", DEFAULT_MODE,
               DEFAULT_MODE == 1 ? "Auto" : DEFAULT_MODE == 2 ? "Manual" : "Timer");
  DEBUG_PRINTF("  Default Priority: %s\n\n", DEFAULT_PRIORITY);
  
  initStorage();
  loadConfiguration();
  
  initDisplay();
  initSensors();
  initControl();
  initButtons();

  initWiFiManager();
  
  String ssid, password;
  bool hasCredentials = loadWiFiCredentials(ssid, password);
  
  if (hasCredentials) {
    DEBUG_PRINTLN("[WIFI] Found saved credentials. Starting connection...");
    startWiFiClient();  // Non-blocking - starts connection
    wifiConnectionAttempted = true;
    // Web server will start when connection succeeds
  } else {
    DEBUG_PRINTLN("[WIFI] ℹ️ No saved credentials found.");
    DEBUG_PRINTLN("[WIFI] Starting Access Point mode automatically...");
    delay(500);
    startWiFiAP();
    
    // Start web server immediately in AP mode
    DEBUG_PRINTLN("[WEB] Starting web server in AP mode...");
    startWebServer();
    webServerStarted = true;
  }
  
  delay(1000);
  readSensors();
  
  showRealtimeData();
  
  DEBUG_PRINTLN("\n[BOOT] System ready!");
  DEBUG_PRINTLN("════════════════════════════════");
  DEBUG_PRINTF("[BOOT] Thresholds: Temp %.1f-%.1f°C, Hum %.0f-%.0f%%\n",
                lower_temp_threshold, upper_temp_threshold,
                lower_hum_threshold, upper_hum_threshold);
  DEBUG_PRINTF("[BOOT] Ambient: %.1f°C, %.0f%%\n",
                ambient_temp, ambient_hum);
  DEBUG_PRINTF("[BOOT] Internal: %.1f°C, %.0f%%\n",
                internal_temp, internal_hum);
  DEBUG_PRINTLN("════════════════════════════════\n");
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Feed watchdog
  yield();
  
  // Handle WiFi connection and start web server when ready
  if (wifiConnectionAttempted && !webServerStarted) {
    // Check if connection succeeded
    if (isWiFiConnected() && !isWiFiConnecting()) {
      DEBUG_PRINTLN("\n[WEB] ✓ WiFi connected - starting web server!");
      startWebServer();
      webServerStarted = true;
      DEBUG_PRINTF("[WEB] Access dashboard: http://%s/\n\n", getIPAddress().c_str());
    } 
    // Check if connection failed
    else if (!isWiFiConnecting() && isWiFiDisabled()) {
      DEBUG_PRINTLN("\n[WEB] Web interface disabled (no WiFi connection).");
      DEBUG_PRINTLN("[WEB] Hold TOP button for 10 seconds to start hotspot.\n");
      wifiConnectionAttempted = false;  // Stop checking
    }
  }
  
  handleWiFiConnection();
  
  // Read sensors every 2 seconds
  if (currentMillis - lastSensorRead >= 2000) {
    readSensors();
    lastSensorRead = currentMillis;

    // Use configured thresholds directly
    lower_temp = lower_temp_threshold;
    lower_hum = lower_hum_threshold;

    // Calculate ambient-aware thresholds for control and display
    const float MIN_TEMP_GAP = 1.5;  // Minimum 1.5°C gap for temperature
    const float MIN_HUM_GAP = 3.0;   // Minimum 3% gap for humidity

    // CRITICAL: Upper thresholds must be above ambient for cooling to work
    // If ambient is already high, raise upper threshold to prevent futile operation
    calculated_upper_temp = max(upper_temp_threshold, ambient_temp + MIN_TEMP_GAP);
    calculated_upper_hum = max(upper_hum_threshold, ambient_hum + MIN_HUM_GAP);

    // Lower thresholds = max of configured lower or ambient
    calculated_lower_temp = max(lower_temp_threshold, ambient_temp);
    calculated_lower_hum = max(lower_hum_threshold, ambient_hum);

    // Ensure minimum gap is ALWAYS maintained
    if (calculated_upper_temp - calculated_lower_temp < MIN_TEMP_GAP) {
      calculated_lower_temp = calculated_upper_temp - MIN_TEMP_GAP;
    }

    if (calculated_upper_hum - calculated_lower_hum < MIN_HUM_GAP) {
      calculated_lower_hum = calculated_upper_hum - MIN_HUM_GAP;
    }

    DEBUG_PRINTF("[CALC] Amb:%.1fC/%.0f%% → CalcUpper:%.1fC/%.0f%% CalcLower:%.1fC/%.0f%%\n",
                 ambient_temp, ambient_hum,
                 calculated_upper_temp, calculated_upper_hum,
                 calculated_lower_temp, calculated_lower_hum);

    postSensorEvent();
  }

  handleButtons();
  updateControl();
  updateDisplayIfNeeded();
  
  // Only update web server if it's started
  if (webServerStarted) {
    updateWebServer();
  }
  
  // Optional: Debug logging every 10 seconds
  if (currentMillis - lastLog >= 10000) {
    lastLog = currentMillis;
    DEBUG_PRINTF("[DATA] Amb: %.1f°C/%.0f%% | Int: %.1f°C/%.0f%% | LowerLimits: %.1f°C/%.0f%%\n",
                  ambient_temp, ambient_hum, internal_temp, internal_hum,
                  lower_temp, lower_hum);
    DEBUG_PRINTF("[STATE] Mode:%s | Relay:%s",
                  mode==1?"Auto":mode==2?"Manual":"Timer",
                  relay_state?"ON":"OFF");
    
    if (webServerStarted && isWiFiConnected()) {
      DEBUG_PRINTF(" | Web: http://%s/", getIPAddress().c_str());
    }
    DEBUG_PRINTLN("");
  }
  
  // Performance monitoring every 30 seconds
  if (currentMillis - lastPerfLog >= 30000) {
    lastPerfLog = currentMillis;
    
    // Only log if web server is running
    if (webServerStarted) {
      int wsClients = getWebSocketClientCount();
      DEBUG_PRINTF("[PERF] Free heap: %d bytes | WS clients: %d | Events queued: %d\n",
                    ESP.getFreeHeap(), wsClients, eventQueue.getCount());
      
      // Warning if heap is low
      if (ESP.getFreeHeap() < 50000) {
        DEBUG_PRINTLN("[PERF] ⚠️ WARNING: Low heap memory!");
      }
      
      // Warning if too many events queued
      if (eventQueue.getCount() > 8) {
        DEBUG_PRINTF("[PERF] ⚠️ WARNING: High event queue: %d events\n", eventQueue.getCount());
      }
    }
  }
  
  delay(10);
}