#include "control_logic.h"
#include "config.h"
#include "variables.h"
#include "events.h"
#include "display.h"


// Auto-off tracking
static unsigned long stableStartTime = 0;
static bool isStable = false;

unsigned long coolerTimerStart = 0;
unsigned long lastRelayOffTime = 0;
static bool timerState = false;
static bool compressorProtectionActive = false;

void initControl() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  relay_state = false;
  lastRelayOffTime = millis();
  
  DEBUG_PRINTLN("[INIT] Cooler control initialized.");
  
#if ENABLE_COMPRESSOR_PROTECTION
  DEBUG_PRINTF("[INIT] Compressor protection: ENABLED (min off time: %d seconds)\n", 
               MIN_OFF_TIME_MS / 1000);
#else
  DEBUG_PRINTLN("[INIT] Compressor protection: DISABLED");
#endif
  
#if ENABLE_AUTO_OFF_STABLE
  DEBUG_PRINTF("[INIT] Auto-off on stable: ENABLED (stable time: %d minutes)\n", 
               STABLE_TIME_MS / 60000);
#else
  DEBUG_PRINTLN("[INIT] Auto-off on stable: DISABLED");
#endif
}

void setRelayState(bool state, int source) {
  if (relay_state != state) {
    relay_state = state;
    digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
    
    if (!relay_state) {
      lastRelayOffTime = millis();
#if ENABLE_COMPRESSOR_PROTECTION
      compressorProtectionActive = true;
#endif
    }
    
    // Post event for immediate updates
    postRelayEvent(relay_state, source);

    // Force immediate display update (only if not in edit mode)
    extern bool edit_mode;
    if (!edit_mode) {
      showRealtimeData();
    }

    DEBUG_PRINTF("[COOLER] Relay %s (source:%d)\n",
                  relay_state ? "ON" : "OFF", source);
  }
}

void resetStableTimer() {
  stableStartTime = 0;
  isStable = false;
}

// Check if internal temp is within ambient + tolerance
bool isWithinTolerance() {
  float tempDiff = internal_temp - ambient_temp;
  float humDiff = internal_hum - ambient_hum;
  
  bool tempOK = (tempDiff <= temp_tolerance);
  bool humOK = (humDiff <= hum_tolerance);
  
  if (priority == "Temperature") {
    return tempOK;
  } else if (priority == "Humidity") {
    return humOK;
  } else { // Both
    return tempOK && humOK;
  }
}

void updateControl() {
  bool relay_on = relay_state;
  
#if ENABLE_COMPRESSOR_PROTECTION
  // Check compressor protection
  if (compressorProtectionActive) {
    if (millis() - lastRelayOffTime >= MIN_OFF_TIME_MS) {
      compressorProtectionActive = false;
      DEBUG_PRINTLN("[PROTECT] Compressor protection expired");
    } else {
      // Force relay OFF during protection period
      if (relay_state) {
        setRelayState(false, 4); // source 4 = protection
        DEBUG_PRINTLN("[PROTECT] Forced OFF - protection active");
      }
      return;
    }
  }
#endif

#if ENABLE_AUTO_OFF_STABLE
  // Auto-off logic - Check if within tolerance for extended period
  if (relay_state && mode == 1) {  // Only in Auto mode when relay is ON
    if (isWithinTolerance()) {
      if (!isStable) {
        // Just became stable
        isStable = true;
        stableStartTime = millis();
        DEBUG_PRINTF("[AUTO-OFF] Stable conditions detected (Int:%.1f°C/%.0f%% vs Amb:%.1f°C/%.0f%% + Tol:%.1f°C/%.0f%%)\n",
                     internal_temp, internal_hum, ambient_temp, ambient_hum,
                     temp_tolerance, hum_tolerance);
      } else {
        // Check if stable for long enough
        unsigned long stableDuration = millis() - stableStartTime;
        if (stableDuration >= STABLE_TIME_MS) {
          DEBUG_PRINTLN("[AUTO-OFF] ⚠️ Stable for 11+ minutes - Turning OFF (cooling ineffective)");
          setRelayState(false, 5); // source 5 = auto-off
          resetStableTimer();
          return;
        } else if (stableDuration % 60000 == 0 && stableDuration > 0) {
          // Log every minute
          unsigned long remainingMin = (STABLE_TIME_MS - stableDuration) / 60000;
          DEBUG_PRINTF("[AUTO-OFF] Stable for %lu min, will turn OFF in %lu min\n",
                       stableDuration / 60000, remainingMin);
        }
      }
    } else {
      // Conditions changed, reset timer
      if (isStable) {
        DEBUG_PRINTLN("[AUTO-OFF] Conditions improved, timer reset");
        resetStableTimer();
      }
    }
  } else {
    // Reset timer if relay is OFF or not in Auto mode
    if (isStable) {
      resetStableTimer();
    }
  }
#endif
  
  // ===================================================
  // 📘 Detect physical button press
  // ===================================================
  if (switch_manual) {
    switch_manual = false;
    
    // In Auto mode: Apply intelligent toggle logic
    if (mode == 1) {
      if (!relay_state) {
        // Turning ON: Check if conditions allow it
        bool canTurnOn = false;
        
        if (priority == "Temperature") {
          canTurnOn = (internal_temp >= lower_temp);
        } else if (priority == "Humidity") {
          canTurnOn = (internal_hum >= lower_hum);
        } else { // Both
          canTurnOn = (internal_temp >= lower_temp) || 
                      (internal_hum >= lower_hum);
        }
        
        if (canTurnOn) {
#if ENABLE_COMPRESSOR_PROTECTION
          if (!compressorProtectionActive) {
            setRelayState(true, 0); // source 0 = button
            DEBUG_PRINTLN("[BUTTON] Manual ON (conditions met)");
          } else {
            DEBUG_PRINTLN("[BUTTON] Blocked - compressor protection active");
          }
#else
          setRelayState(true, 0); // source 0 = button
          DEBUG_PRINTLN("[BUTTON] Manual ON (conditions met)");
#endif
        } else {
          DEBUG_PRINTLN("[BUTTON] Blocked - below lower threshold");
        }
      } else {
        // Turning OFF: Always allow in Auto mode
        setRelayState(false, 0); // source 0 = button
        DEBUG_PRINTLN("[BUTTON] Manual OFF (protection started)");
      }
    }
    // Manual and Timer modes: always allow toggle
    else {
      setRelayState(!relay_state, 0); // source 0 = button
      
      if (mode == 3) {
        timerState = relay_state;
        coolerTimerStart = millis();
        postTimerEvent();
        DEBUG_PRINTLN("[TIMER] Reset by button");
      }
    }
    
    return;
  }

  // ===================================================
  // 🧠 Mode-specific control logic
  // ===================================================

  // AUTO MODE – hysteresis control
  if (mode == 1) {
    if (relay_state) {
      // Currently ON: Check if should turn OFF
      if (priority == "Temperature") {
        relay_on = (internal_temp > lower_temp);
      } else if (priority == "Humidity") {
        relay_on = (internal_hum > lower_hum);
      } else { // Both
        relay_on = (internal_temp > lower_temp) || 
                   (internal_hum > lower_hum);
      }
    } else {
      // Currently OFF: Check if should turn ON
      bool shouldTurnOn = false;
#if ENABLE_COMPRESSOR_PROTECTION
      if (!compressorProtectionActive) {
        shouldTurnOn = true;
      }
#else
      shouldTurnOn = true;
#endif
      
      if (shouldTurnOn) {
        if (priority == "Temperature") {
          relay_on = (internal_temp > upper_temp_threshold);
        } else if (priority == "Humidity") {
          relay_on = (internal_hum > upper_hum_threshold);
        } else { // Both
          relay_on = (internal_temp > upper_temp_threshold) || 
                     (internal_hum > upper_hum_threshold);
        }
      }
    }
  }

  // MANUAL MODE – user control only
  else if (mode == 2) {
    relay_on = relay_state;
  }

  // TIMER MODE – scheduled operation
  else if (mode == 3) {
    unsigned long elapsedMin = (millis() - coolerTimerStart) / 60000UL;

    if (elapsedMin >= (unsigned long)timer_value) {
      // Check if we can turn ON (respect compressor protection)
      bool canFlip = false;
#if ENABLE_COMPRESSOR_PROTECTION
      if (!timerState || !compressorProtectionActive) {
        canFlip = true;
      }
#else
      canFlip = true;
#endif
      
      if (canFlip) {
        coolerTimerStart = millis();
        timerState = !timerState;
        postTimerEvent();
        DEBUG_PRINTF("[TIMER] Auto flip -> %s\n", timerState ? "ON" : "OFF");
      }
#if ENABLE_COMPRESSOR_PROTECTION
      else {
        DEBUG_PRINTLN("[TIMER] Waiting for compressor protection");
      }
#endif
    }

    relay_on = timerState;
  }

  // ===================================================
  // ⚡ Apply Relay Output
  // ===================================================
  if (relay_on != relay_state) {
    setRelayState(relay_on, 1); // source 1 = auto
  }
}

// Get remaining protection time in seconds
unsigned long getProtectionRemaining() {
#if ENABLE_COMPRESSOR_PROTECTION
  if (!compressorProtectionActive) return 0;
  
  unsigned long elapsed = millis() - lastRelayOffTime;
  if (elapsed >= MIN_OFF_TIME_MS) return 0;
  
  return (MIN_OFF_TIME_MS - elapsed) / 1000;
#else
  return 0;
#endif
}

bool isProtectionActive() {
#if ENABLE_COMPRESSOR_PROTECTION
  return compressorProtectionActive;
#else
  return false;
#endif
}