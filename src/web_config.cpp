// web_config.cpp - FIXED VERSION with optimizations
#include "web_config.h"
#include "html_pages.h"
#include "variables.h"
#include "storage_manager.h"
#include "wifi_manager.h"
#include "events.h"
#include "control_logic.h"
#include "config.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>

// ============================================
// WEBSOCKET CONFIGURATION
// ============================================
#define WS_MAX_CLIENTS 4  // Limit concurrent WebSocket connections

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

static unsigned long lastWebUpdate = 0;
static unsigned long lastCleanup = 0;
static bool pendingUpdate = false;

// ============================================
// HELPER FUNCTIONS
// ============================================

// Notify all WebSocket clients
void notifyClients(const String& message) {
  if (ws.count() > 0) {
    ws.textAll(message);
  }
}

// Send complete data update to clients (optimized)
void sendDataUpdate() {
  // Don't prepare data if no clients connected
  if (ws.count() == 0) {
    return;
  }
  
  JsonDocument doc;
  doc["ambient_temp_rt"] = ambient_temp;
  doc["ambient_hum_rt"]  = ambient_hum;
  doc["temp_rt"]         = internal_temp;
  doc["hum_rt"]          = internal_hum;
  doc["relay_state"]     = relay_state;
  doc["mode"]            = mode;
  doc["priority"]        = priority;
  doc["timer_value"]     = timer_value;
  doc["upper_temp_threshold"] = upper_temp_threshold;
  doc["lower_temp_threshold"] = lower_temp_threshold;
  doc["upper_hum_threshold"]  = upper_hum_threshold;
  doc["lower_hum_threshold"]  = lower_hum_threshold;
  doc["calculated_upper_temp"] = calculated_upper_temp;
  doc["calculated_lower_temp"] = calculated_lower_temp;
  doc["calculated_upper_hum"]  = calculated_upper_hum;
  doc["calculated_lower_hum"]  = calculated_lower_hum;
  doc["protection_active"]    = isProtectionActive();
  doc["protection_remaining"] = getProtectionRemaining();
  
  unsigned long elapsedSec = 0;
  if (mode == 3) {
    elapsedSec = (millis() - coolerTimerStart) / 1000UL;
  }
  doc["timer_elapsed"] = elapsedSec;
  
  String out;
  serializeJson(doc, out);
  notifyClients(out);
  
  lastWebUpdate = millis();
}

// Process events with timeout to prevent blocking
void processWebEvents() {
  SystemEvent event;
  bool needsUpdate = false;
  int eventsProcessed = 0;
  const int MAX_EVENTS_PER_CYCLE = 10;  // Don't process more than 10 events per cycle
  
  while (!eventQueue.isEmpty() && eventsProcessed < MAX_EVENTS_PER_CYCLE) {
    if (eventQueue.pop(event)) {
      needsUpdate = true;
      eventsProcessed++;
      
      switch (event.type) {
        case EVENT_RELAY_CHANGED:
          DEBUG_PRINTF("[WEB] Relay: %s (src:%d)\n", 
                       event.relay_state ? "ON" : "OFF",
                       event.relay_source);
          break;
          
        case EVENT_MODE_CHANGED:
          DEBUG_PRINTF("[WEB] Mode: %d (src:%d)\n",
                       event.mode_value,
                       event.mode_source);
          break;
          
        case EVENT_PRIORITY_CHANGED:
          DEBUG_PRINTF("[WEB] Priority: %s\n", event.priority_value);
          break;
          
        case EVENT_THRESHOLDS_CHANGED:
          DEBUG_PRINTLN("[WEB] Thresholds changed");
          break;
          
        case EVENT_TIMER_CHANGED:
          DEBUG_PRINTLN("[WEB] Timer changed");
          break;
          
        default:
          break;
      }
    }
  }
  
  if (eventsProcessed >= MAX_EVENTS_PER_CYCLE) {
    DEBUG_PRINTF("[WEB] Event queue still has %d events, will process next cycle\n", 
                 eventQueue.getCount());
  }
  
  if (needsUpdate) {
    pendingUpdate = true;
  }
  
  // Only send updates if we have connected clients
  if ((pendingUpdate || (millis() - lastWebUpdate >= 1000)) && ws.count() > 0) {
    sendDataUpdate();
    pendingUpdate = false;
  }
}

// ============================================
// WEBSOCKET EVENT HANDLER (with connection limit)
// ============================================
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    // Limit connections
    if (ws.count() > WS_MAX_CLIENTS) {
      DEBUG_PRINTF("[WS] Too many clients (%d), rejecting #%u\n", ws.count(), client->id());
      client->close();
      return;
    }
    
    DEBUG_PRINTF("[WS] Client #%u connected from %s\n", 
                  client->id(), 
                  client->remoteIP().toString().c_str());
    sendDataUpdate();
    
  } else if (type == WS_EVT_DISCONNECT) {
    DEBUG_PRINTF("[WS] Client #%u disconnected\n", client->id());
    
  } else if (type == WS_EVT_ERROR) {
    DEBUG_PRINTF("[WS] Client #%u error\n", client->id());
  }
}

// ============================================
// WEB SERVER INITIALIZATION
// ============================================
void startWebServer() {
  // Configure WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  
  // ============================================
  // WiFi Setup Page
  // ============================================
  server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send_P(200, "text/html", WIFI_SETUP_page);
  });

  // WiFi Scan
  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *req){
    String networks = scanWiFiNetworks();
    req->send(200, "application/json", networks);
  });

  // Set WiFi Credentials
  server.on("/setwifi", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL,
  [](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){
    JsonDocument doc;
    if(deserializeJson(doc, data)) {
      req->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
      return;
    }
    
    String ssid = String((const char*)doc["ssid"]);
    String password = String((const char*)doc["password"]);
    
    saveWiFiCredentials(ssid.c_str(), password.c_str());
    
    DEBUG_PRINTLN("[WEB] WiFi credentials saved. Restarting in 2 seconds...");
    
    req->send(200, "application/json", "{\"success\":true,\"message\":\"Credentials saved. Restarting...\"}");
    
    delay(500);
    ESP.restart();
  });

  // ============================================
  // Main Dashboard - with dynamic mode options and priority options
  // ============================================
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req){
    if (getWiFiMode() == WIFI_AP_MODE) {
      req->redirect("/setup");
      return;
    }
    
    String page = MAIN_page;
    String lang = getLanguage();
    page.replace("%LANG%", lang);
    
    // Insert dynamic mode options
    String modeOptions = getModeOptionsHTML();
    page.replace("%MODE_OPTIONS%", modeOptions);
    
    // Insert dynamic priority options
    String priorityOptions = getPriorityOptionsHTML();
    page.replace("%PRIORITY_OPTIONS%", priorityOptions);
    
    req->send(200, "text/html", page);
  });

  // ============================================
  // Configuration Page
  // ============================================
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *req){
    req->send_P(200, "text/html", CONFIG_page);
  });

  // ============================================
  // Get Data
  // ============================================
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *req){
    JsonDocument doc;
    doc["ambient_temp_rt"] = ambient_temp;
    doc["ambient_hum_rt"]  = ambient_hum;
    doc["temp_rt"]         = internal_temp;
    doc["hum_rt"]          = internal_hum;
    doc["relay_state"]     = relay_state;
    doc["mode"]            = mode;
    doc["priority"]        = priority;
    doc["timer_value"]     = timer_value;
    doc["upper_temp_threshold"] = upper_temp_threshold;
    doc["lower_temp_threshold"] = lower_temp_threshold;
    doc["upper_hum_threshold"]  = upper_hum_threshold;
    doc["lower_hum_threshold"]  = lower_hum_threshold;
    doc["calculated_upper_temp"] = calculated_upper_temp;
    doc["calculated_lower_temp"] = calculated_lower_temp;
    doc["calculated_upper_hum"]  = calculated_upper_hum;
    doc["calculated_lower_hum"]  = calculated_lower_hum;
    doc["protection_active"]    = isProtectionActive();
    doc["protection_remaining"] = getProtectionRemaining();
    
    unsigned long elapsedSec = 0;
    if (mode == 3) {
      elapsedSec = (millis() - coolerTimerStart) / 1000UL;
    }
    doc["timer_elapsed"] = elapsedSec;
    
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
  });

  // ============================================
  // Relay Control
  // ============================================
  server.on("/relay", HTTP_GET, [](AsyncWebServerRequest *req){
    if(req->hasParam("state")){
      bool requested_state = (req->getParam("state")->value() == "1");
      
      // Check protection when trying to turn ON
      if (requested_state && !relay_state && isProtectionActive()) {
        unsigned long remaining = getProtectionRemaining();
        DEBUG_PRINTF("[WEB] Blocked - protection active (%lu sec remaining)\n", remaining);
        req->send(200, "text/plain", "BLOCKED");
        return;
      }
      
      // In Manual mode: direct control
      if (mode == 2) {
        if (requested_state != relay_state) {
          setRelayState(requested_state, 2); // source 2 = web
          DEBUG_PRINTF("[WEB] Manual relay -> %s\n", requested_state ? "ON" : "OFF");
        }
      }
      // In Auto/Timer mode: trigger manual toggle logic
      else {
        switch_manual = true;
        DEBUG_PRINTLN("[WEB] Manual trigger");
      }
    }
    req->send(200, "text/plain", "OK");
  });

  // ============================================
  // Mode Control - with validation
  // ============================================
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *req){
    if(req->hasParam("set")){
      int oldMode = mode;
      int newMode = req->getParam("set")->value().toInt();
      
      // Validate the mode is enabled
      bool modeEnabled = false;
      
      #if ENABLE_AUTO_MODE
        if (newMode == 1) modeEnabled = true;
      #endif
      #if ENABLE_MANUAL_MODE
        if (newMode == 2) modeEnabled = true;
      #endif
      #if ENABLE_TIMER_MODE
        if (newMode == 3) modeEnabled = true;
      #endif
      
      if (modeEnabled && oldMode != newMode) {
        mode = newMode;
        manual_override = false;
        if (mode == 3) {
          coolerTimerStart = millis();
        }
        saveConfiguration();
        postModeEvent(mode, 1);
        DEBUG_PRINTF("[WEB] Mode -> %d\n", mode);
      } else if (!modeEnabled) {
        DEBUG_PRINTF("[WEB] Mode %d is disabled in config\n", newMode);
      }
    }
    req->send(200, "text/plain", "OK");
  });

  // ============================================
  // Priority Control
  // ============================================
  server.on("/priority", HTTP_GET, [](AsyncWebServerRequest *req){
    if(req->hasParam("set")){
      priority = req->getParam("set")->value();
      saveConfiguration();
      postPriorityEvent(priority);
      DEBUG_PRINTF("[WEB] Priority -> %s\n", priority.c_str());
    }
    req->send(200, "text/plain", "OK");
  });

  // ============================================
  // Language Setting
  // ============================================
  server.on("/setlang", HTTP_GET, [](AsyncWebServerRequest *req){
    if(req->hasParam("lang")){
      String lang = req->getParam("lang")->value();
      saveLanguage(lang.c_str());
      DEBUG_PRINTF("[WEB] Language -> %s\n", lang.c_str());
    }
    req->send(200, "text/plain", "OK");
  });

  // ============================================
  // Save Configuration
  // ============================================
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *req){}, NULL,
  [](AsyncWebServerRequest *req, uint8_t *data, size_t len, size_t index, size_t total){
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data);
    
    if(err) {
      DEBUG_PRINTF("[WEB] JSON parse error: %s\n", err.c_str());
      req->send(400, "text/plain", "Invalid JSON"); 
      return;
    }
    
    upper_temp_threshold = doc["upper_temp_threshold"];
    lower_temp_threshold = doc["lower_temp_threshold"];
    upper_hum_threshold  = doc["upper_hum_threshold"];
    lower_hum_threshold  = doc["lower_hum_threshold"];
    timer_value          = doc["timer_value"];

    saveConfiguration();
    postThresholdsEvent();

    DEBUG_PRINTLN("[WEB] Configuration saved to flash.");
    DEBUG_PRINTF("  Temp: %.1f - %.1f°C\n",
                  lower_temp_threshold, upper_temp_threshold);
    DEBUG_PRINTF("  Hum: %.0f - %.0f%%\n",
                  lower_hum_threshold, upper_hum_threshold);
    DEBUG_PRINTF("  Timer: %d min\n", timer_value);
    
    req->send(200, "text/plain", "Saved");
  });

  server.begin();
  DEBUG_PRINTLN("[HTTP] Web server started.");
  DEBUG_PRINTF("[HTTP] Access: http://%s/\n", getIPAddress().c_str());
}

// ============================================
// WEB SERVER UPDATE (optimized)
// ============================================
void updateWebServer() {
  // Cleanup dead connections (non-blocking check)
  // Only cleanup every 10 seconds to prevent blocking
  if (millis() - lastCleanup > 10000) {
    ws.cleanupClients();
    lastCleanup = millis();
  }
  
  // Process events with a limit to prevent blocking
  processWebEvents();
}

// ============================================
// HELPER FUNCTION - Get WebSocket client count
// ============================================
int getWebSocketClientCount() {
  return ws.count();
}