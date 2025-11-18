// ============================================================================
// config.h - ADD THESE LINES (Priority Configuration Section)
// ============================================================================
// Place this section after the MODE CONFIGURATION section in your config.h

// ============================================
// PRIORITY CONFIGURATION (Auto Mode Only)
// ============================================

// Enable/Disable specific priorities (only applies in Auto mode)
#define ENABLE_PRIORITY_TEMPERATURE true
#define ENABLE_PRIORITY_HUMIDITY true
#define ENABLE_PRIORITY_BOTH true

// Default priority for Auto mode is already defined above as:
// #define DEFAULT_PRIORITY "Temperature"


// ============================================================================
// buttons.cpp - COMPLETE FILE
// ============================================================================

#include "buttons.h"
#include "config.h"
#include "variables.h"
#include "display.h"
#include "storage_manager.h"
#include "wifi_manager.h"
#include "web_config.h"
#include "events.h"
#include "web_state.h"

#define BTN_TOP     13
#define BTN_MID     12
#define BTN_BOTTOM  10
#define BTN_LEFT    14
#define BTN_RIGHT   11

#define HOLD_TIME_MS 10000

static unsigned long lastPress = 0;
static unsigned long btnTopPressStart = 0;
static bool btnTopHeld = false;
static bool btnTopProcessed = false;  // NEW: Track if button action was already processed
const unsigned long debounceMs = 200;
const unsigned long manualToggleDebounce = 500;
static unsigned long lastManualToggle = 0;
int currentParam = 0;

static inline bool pressed(int pin) {
  if (digitalRead(pin) == LOW && millis() - lastPress > debounceMs) {
    lastPress = millis();
    return true;
  }
  return false;
}

static inline bool released(int pin) {
  return (digitalRead(pin) == HIGH);
}

void initButtons() {
  pinMode(BTN_TOP, INPUT_PULLUP);
  pinMode(BTN_MID, INPUT_PULLUP);
  pinMode(BTN_BOTTOM, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  DEBUG_PRINTLN("[INIT] Buttons initialized.");
}

void handleButtons() {
  // WiFi reset: hold BTN_TOP for 10 seconds
  if (digitalRead(BTN_TOP) == LOW) {
    if (btnTopPressStart == 0) {
      btnTopPressStart = millis();
      btnTopProcessed = false;  // Reset processed flag on new press
    } else if (!btnTopHeld && millis() - btnTopPressStart >= HOLD_TIME_MS) {
      btnTopHeld = true;
      btnTopProcessed = true;  // Mark as processed for long hold
      
      // Check current WiFi state
      if (getWiFiMode() == WIFI_AP_MODE) {
        // Already in AP mode - clear credentials and restart
        DEBUG_PRINTLN("\n[BTN] WiFi Reset - Clearing credentials!");
        clearWiFiCredentials();
        delay(1000);
        ESP.restart();
      } else if (isWiFiDisabled()) {
        // WiFi is off - start hotspot
        DEBUG_PRINTLN("\n[BTN] Starting Hotspot!");
        startWiFiAP();
        
        // Start web server if not already started
        if (!webServerStarted) {
          DEBUG_PRINTLN("[BTN] Starting web server for AP mode...");
          startWebServer();
          webServerStarted = true;
          wifiConnectionAttempted = false;  // Reset flag
          DEBUG_PRINTF("[BTN] Web interface available: http://%s/\n\n", getIPAddress().c_str());
        }
      } else {
        // Connected to WiFi - clear and restart to AP mode
        DEBUG_PRINTLN("\n[BTN] WiFi Reset - Will start hotspot!");
        clearWiFiCredentials();
        delay(1000);
        ESP.restart();
      }
    }
  } else {
    // Button released
    if (btnTopPressStart > 0 && !btnTopProcessed && !edit_mode) {
      // Short press detected (released before 10 seconds)
      unsigned long pressDuration = millis() - btnTopPressStart;
      
      if (pressDuration < HOLD_TIME_MS && millis() - lastPress > debounceMs) {
        // Toggle WiFi info only on release of short press
        toggleWiFiInfo();
        postButtonEvent(BTN_TOP);
        DEBUG_PRINTLN("[BTN] WiFi info toggle");
        lastPress = millis();
      }
    }
    
    // Reset all states
    btnTopPressStart = 0;
    btnTopHeld = false;
    btnTopProcessed = false;
  }

  if (!edit_mode) {
    // Removed the old BTN_TOP handler from here since it's now handled above

    if (pressed(BTN_LEFT)) {
      // Cycle through enabled priorities only
      String originalPriority = priority;
      bool foundNext = false;
      int attempts = 0;
      
      do {
        // Cycle to next priority
        if (priority == "Temperature") priority = "Humidity";
        else if (priority == "Humidity") priority = "None";
        else priority = "Temperature";
        
        // Check if this priority is enabled
        bool enabled = false;
        
        #if ENABLE_PRIORITY_TEMPERATURE
          if (priority == "Temperature") enabled = true;
        #endif
        #if ENABLE_PRIORITY_HUMIDITY
          if (priority == "Humidity") enabled = true;
        #endif
        #if ENABLE_PRIORITY_BOTH
          if (priority == "None") enabled = true;
        #endif
        
        if (enabled) {
          foundNext = true;
          break;
        }
        
        attempts++;
      } while (attempts < 3 && !foundNext);
      
      if (!foundNext) {
        // If no enabled priority found, revert to original
        priority = originalPriority;
        DEBUG_PRINTLN("[BTN] No other priorities enabled");
      } else {
        saveConfiguration();
        postPriorityEvent(priority);
        postButtonEvent(BTN_LEFT);
        
        DEBUG_PRINTF("[BTN] Priority -> %s\n", priority.c_str());
        showRealtimeData();
      }
    }

    if (pressed(BTN_RIGHT)) {
      // Cycle through enabled modes only
      int originalMode = mode;
      do {
        mode++;
        if (mode > 3) mode = 1;
        
        // Check if this mode is enabled
        #if !ENABLE_AUTO_MODE
          if (mode == 1) continue;
        #endif
        #if !ENABLE_MANUAL_MODE
          if (mode == 2) continue;
        #endif
        #if !ENABLE_TIMER_MODE
          if (mode == 3) continue;
        #endif
        
        break;  // Found an enabled mode
      } while (mode != originalMode);
      
      saveConfiguration();
      postModeEvent(mode, 0);
      postButtonEvent(BTN_RIGHT);
      
      DEBUG_PRINTF("[BTN] Mode -> %d\n", mode);
      showRealtimeData();
    }

    if (pressed(BTN_BOTTOM)) {
      if (millis() - lastManualToggle < manualToggleDebounce) {
        DEBUG_PRINTLN("[BTN] Manual toggle ignored (too fast)");
        return;
      }
      
      lastManualToggle = millis();
      switch_manual = true;
      postButtonEvent(BTN_BOTTOM);
      DEBUG_PRINTLN("[BTN] Manual toggle pressed");
    }

    if (pressed(BTN_MID)) {
      edit_mode = true;
      currentParam = 0;
      
      postEditModeEvent(true);
      postButtonEvent(BTN_MID);
      
      DEBUG_PRINTLN("[BTN] Edit mode");
      showEditScreen("Upper Temp", upper_temp_threshold);
    }
  }
  else {
    // IN EDIT MODE
    bool paramChanged = false;
    bool needsRedraw = false;
    
    if (pressed(BTN_MID)) {
      edit_mode = false;
      saveConfiguration();
      
      postEditModeEvent(false);
      postThresholdsEvent();
      postButtonEvent(BTN_MID);
      
      DEBUG_PRINTLN("[BTN] Exit edit");
      showRealtimeData();
      return;
    }

    if (pressed(BTN_TOP)) {
      currentParam--;
      if (currentParam < 0) currentParam = 4;  // 0-4 = 5 parameters
      needsRedraw = true;
      postButtonEvent(BTN_TOP);
    }

    if (pressed(BTN_BOTTOM)) {
      currentParam++;
      if (currentParam > 4) currentParam = 0;  // 0-4 = 5 parameters
      needsRedraw = true;
      postButtonEvent(BTN_BOTTOM);
    }

    if (pressed(BTN_LEFT)) {
      switch (currentParam) {
        case 0: upper_temp_threshold -= 0.5; break;
        case 1: lower_temp_threshold -= 0.5; break;
        case 2: upper_hum_threshold -= 1; break;
        case 3: lower_hum_threshold -= 1; break;
        case 4: timer_value = max(1, timer_value - 1); break;
      }
      paramChanged = true;
      needsRedraw = true;
      postButtonEvent(BTN_LEFT);
    }

    if (pressed(BTN_RIGHT)) {
      switch (currentParam) {
        case 0: upper_temp_threshold += 0.5; break;
        case 1: lower_temp_threshold += 0.5; break;
        case 2: upper_hum_threshold += 1; break;
        case 3: lower_hum_threshold += 1; break;
        case 4: timer_value += 1; break;
      }
      paramChanged = true;
      needsRedraw = true;
      postButtonEvent(BTN_RIGHT);
    }

    if (needsRedraw) {
      switch (currentParam) {
        case 0: showEditScreen("Upper Temp", upper_temp_threshold); break;
        case 1: showEditScreen("Lower Temp", lower_temp_threshold); break;
        case 2: showEditScreen("Upper Hum", upper_hum_threshold); break;
        case 3: showEditScreen("Lower Hum", lower_hum_threshold); break;
        case 4: showEditScreen("Timer (min)", timer_value); break;
      }
    }
    
    if (paramChanged) {
      postThresholdsEvent();
    }
  }
}
