// ============================================================================
// storage_manager.cpp - COMPLETE FILE with DEBUG macros
// ============================================================================

#include "storage_manager.h"
#include "config.h"
#include "variables.h"
#include <Preferences.h>

Preferences storage;

void initStorage() {
  storage.begin("cooler", false);
  DEBUG_PRINTLN("[INIT] Storage manager initialized.");
}

void loadConfiguration() {
  upper_temp_threshold = storage.getFloat("upperTemp", DEFAULT_UPPER_TEMP);
  lower_temp_threshold = storage.getFloat("lowerTemp", DEFAULT_LOWER_TEMP);
  upper_hum_threshold = storage.getFloat("upperHum", DEFAULT_UPPER_HUM);
  lower_hum_threshold = storage.getFloat("lowerHum", DEFAULT_LOWER_HUM);
  
  // Load tolerance
  temp_tolerance = storage.getFloat("tempTol", DEFAULT_TEMP_TOLERANCE);
  hum_tolerance = storage.getFloat("humTol", DEFAULT_HUM_TOLERANCE);
  
  // Load mode and validate it's enabled
  mode = storage.getInt("mode", DEFAULT_MODE);
  
  // Validate mode is enabled, fallback to first available mode
  #if !ENABLE_AUTO_MODE
    if (mode == 1) {
      #if ENABLE_MANUAL_MODE
        mode = 2;
      #elif ENABLE_TIMER_MODE
        mode = 3;
      #else
        mode = 1;  // Fallback, at least one mode must be enabled
      #endif
    }
  #endif
  
  #if !ENABLE_MANUAL_MODE
    if (mode == 2) {
      #if ENABLE_AUTO_MODE
        mode = 1;
      #elif ENABLE_TIMER_MODE
        mode = 3;
      #else
        mode = 1;  // Fallback
      #endif
    }
  #endif
  
  #if !ENABLE_TIMER_MODE
    if (mode == 3) {
      #if ENABLE_AUTO_MODE
        mode = 1;
      #elif ENABLE_MANUAL_MODE
        mode = 2;
      #else
        mode = 1;  // Fallback
      #endif
    }
  #endif
  
  // Load priority and validate it's enabled
  String savedPriority = storage.getString("priority", DEFAULT_PRIORITY);
  priority = savedPriority;
  
  // Validate priority is enabled, fallback to first available
  bool priorityValid = false;
  
  #if ENABLE_PRIORITY_TEMPERATURE
    if (priority == "Temperature") priorityValid = true;
  #endif
  #if ENABLE_PRIORITY_HUMIDITY
    if (priority == "Humidity") priorityValid = true;
  #endif
  #if ENABLE_PRIORITY_BOTH
    if (priority == "None") priorityValid = true;
  #endif
  
  // If invalid, set to first available priority
  if (!priorityValid) {
    #if ENABLE_PRIORITY_TEMPERATURE
      priority = "Temperature";
    #elif ENABLE_PRIORITY_HUMIDITY
      priority = "Humidity";
    #elif ENABLE_PRIORITY_BOTH
      priority = "None";
    #else
      priority = "Temperature";  // Fallback
    #endif
    
    DEBUG_PRINTF("[STORAGE] Invalid priority '%s', reset to '%s'\n", 
                  savedPriority.c_str(), priority.c_str());
  }
  
  timer_value = storage.getInt("timerVal", DEFAULT_TIMER_VALUE);
  
  DEBUG_PRINTLN("[STORAGE] Configuration loaded:");
  DEBUG_PRINTF("  Temp: %.1f - %.1f°C (Tolerance: %.1f°C)\n", 
                lower_temp_threshold, upper_temp_threshold, temp_tolerance);
  DEBUG_PRINTF("  Hum: %.0f - %.0f%% (Tolerance: %.0f%%)\n", 
                lower_hum_threshold, upper_hum_threshold, hum_tolerance);
  DEBUG_PRINTF("  Mode: %d | Priority: %s | Timer: %d min\n", 
                mode, priority.c_str(), timer_value);
}

void saveConfiguration() {
  storage.putFloat("upperTemp", upper_temp_threshold);
  storage.putFloat("lowerTemp", lower_temp_threshold);
  storage.putFloat("upperHum", upper_hum_threshold);
  storage.putFloat("lowerHum", lower_hum_threshold);
  
  // Save tolerance
  storage.putFloat("tempTol", temp_tolerance);
  storage.putFloat("humTol", hum_tolerance);
  
  storage.putInt("mode", mode);
  storage.putString("priority", priority);
  storage.putInt("timerVal", timer_value);
  
  DEBUG_PRINTLN("[STORAGE] Configuration saved.");
  DEBUG_PRINTF("  Temp Tolerance: %.1f°C | Hum Tolerance: %.0f%%\n", 
                temp_tolerance, hum_tolerance);
}

void saveLanguage(const char* lang) {
  storage.putString("lang", lang);
  DEBUG_PRINTF("[STORAGE] Language saved: %s\n", lang);
}

String getLanguage() {
  return storage.getString("lang", "en");
}