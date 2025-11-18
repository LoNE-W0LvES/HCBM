#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// SYSTEM CONFIGURATION
// ============================================

// Debug mode - set to false to disable serial output
#define DEBUG_MODE true

// Debug print macros
#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif

// ============================================
// COMPRESSOR PROTECTION SETTINGS
// ============================================

// Enable/Disable compressor protection
#define ENABLE_COMPRESSOR_PROTECTION false

// Minimum off time before compressor can restart (milliseconds)
// Default: 180000ms = 3 minutes
// Recommended range: 120000-300000ms (2-5 minutes)
#define MIN_OFF_TIME_MS 180000

// Auto-off when stable (cooling ineffective detection)
#define ENABLE_AUTO_OFF_STABLE true

// Time to wait before auto-off when temperature is stable (milliseconds)
// Default: 660000ms = 11 minutes
// This detects when cooling is ineffective (internal temp not decreasing)
#define STABLE_TIME_MS 660000

// ============================================
// WiFi Configuration
// ============================================
#define AP_SSID "SmartCooler_Setup"
#define AP_PASSWORD "12345678"
#define WIFI_TIMEOUT_MS 15000
#define WIFI_RETRY_INTERVAL_MS 30000

// ============================================
// Sensor Configuration
// ============================================
#define SENSOR_READ_INTERVAL_MS 2000
#define SENSOR_MAX_FAILURES 5

// ============================================
// Display Configuration
// ============================================
#define DISPLAY_UPDATE_INTERVAL_MS 1500
#define WIFI_INFO_DISPLAY_TIME_MS 5000

// ============================================
// Relay/Control Configuration
// ============================================
#define RELAY_PIN 15

// ============================================
// Button Configuration
// ============================================
#define BTN_TOP 13
#define BTN_MID 12
#define BTN_BOTTOM 10
#define BTN_LEFT 14
#define BTN_RIGHT 11
#define BTN_DEBOUNCE_MS 200
#define BTN_HOLD_TIME_MS 10000

// ============================================
// I2C Configuration
// ============================================
#define I2C_SCL 9
#define I2C_SDA 8
#define INTERNAL_SDA 7
#define INTERNAL_SCL 6
#define OLED_ADDRESS 0x3C
#define AHT_ADDRESS 0x38

// ============================================
// Default Values
// ============================================
#define DEFAULT_UPPER_TEMP 35.0
#define DEFAULT_LOWER_TEMP 25.0
#define DEFAULT_UPPER_HUM 75.0
#define DEFAULT_LOWER_HUM 45.0
#define DEFAULT_TIMER_VALUE 10

// ============================================
// MODE CONFIGURATION
// ============================================

// Enable/Disable specific modes
#define ENABLE_AUTO_MODE true      // Auto mode (temperature/humidity control)
#define ENABLE_MANUAL_MODE true    // Manual mode (user control only)
#define ENABLE_TIMER_MODE true     // Timer mode (scheduled on/off)

// Default mode on startup
// 1 = Auto, 2 = Manual, 3 = Timer
// Make sure the selected default mode is enabled above!
#define DEFAULT_MODE 1

// Default priority for Auto mode
// Options: "Temperature", "Humidity", "None" (None means both)
// ============================================
// PRIORITY CONFIGURATION (Auto Mode Only)
// ============================================

// Enable/Disable specific priorities (only applies in Auto mode)
#define ENABLE_PRIORITY_TEMPERATURE true
#define ENABLE_PRIORITY_HUMIDITY true
#define ENABLE_PRIORITY_BOTH true

// Default priority for Auto mode is already defined above as:
#define DEFAULT_PRIORITY "Temperature"


#endif