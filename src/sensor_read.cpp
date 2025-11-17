// sensor_read.cpp - AHT21 VERSION (Fixed to use config.h pins and DEBUG macros)
#include "sensor_read.h"
#include "config.h"  // IMPORTANT: Use pins from config.h
#include "variables.h"
#include "events.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// AHT21 sensor objects
Adafruit_AHTX0 ahtAmbient;
Adafruit_AHTX0 ahtInternal;

// Default I2C address for AHT21
#define AHT_ADDR 0x38

// State tracking
static int consecutiveFailures = 0;
const int MAX_FAILURES = 5;
static bool ambientSensorOK = false;
static bool internalSensorOK = false;

// I2C bus for internal sensor
TwoWire I2C_Internal = TwoWire(1);

// I2C Scanner function
void scanI2C(TwoWire &wire, const char* busName) {
  DEBUG_PRINTF("\n[I2C] ==================== %s SCAN ====================\n", busName);
  byte error, address;
  int nDevices = 0;

  for(address = 1; address < 127; address++) {
    wire.beginTransmission(address);
    error = wire.endTransmission();

    if (error == 0) {
      DEBUG_PRINTF("[I2C] [OK] Device found at address 0x%02X", address);
      
      // Identify known devices
      if (address == 0x3C) DEBUG_PRINT(" (SSD1306 OLED)");
      else if (address == 0x38) DEBUG_PRINT(" (AHT21 Sensor)");
      else if (address == 0x76 || address == 0x77) DEBUG_PRINT(" (BMP280/BME280)");
      
      DEBUG_PRINTLN("");
      nDevices++;
    }
  }
  
  if (nDevices == 0) {
    DEBUG_PRINTF("[I2C] [ERROR] NO DEVICES FOUND ON %s!\n", busName);
  } else {
    DEBUG_PRINTF("[I2C] Total devices found: %d\n", nDevices);
  }
  DEBUG_PRINTLN("[I2C] ====================================================\n");
}

void initSensors() {
  DEBUG_PRINTLN("\n[SENSOR] ============================================");
  DEBUG_PRINTLN("[SENSOR] AHT21 Sensor Initialization");
  DEBUG_PRINTLN("[SENSOR] ============================================");
  
  // Print pin configuration from config.h
  DEBUG_PRINTLN("[SENSOR] Pin Configuration (from config.h):");
  DEBUG_PRINTF("[SENSOR]   Ambient I2C:  GPIO %d (SDA), %d (SCL) - Shared with display\n", 
                I2C_SDA, I2C_SCL);
  DEBUG_PRINTF("[SENSOR]   Internal I2C: GPIO %d (SDA), %d (SCL) - TwoWire(1)\n", 
                INTERNAL_SDA, INTERNAL_SCL);
  
  // Wait for display I2C bus to stabilize
  // Display already initialized Wire on GPIO I2C_SDA-I2C_SCL in display.cpp
  delay(300);
  
  // Initialize internal sensor I2C bus using TwoWire(1)
  DEBUG_PRINTF("[SENSOR] Initializing Internal I2C bus (GPIO %d-%d, TwoWire(1))...\n",
                INTERNAL_SDA, INTERNAL_SCL);
  I2C_Internal.begin(INTERNAL_SDA, INTERNAL_SCL, 100000);
  delay(200);

  DEBUG_PRINTLN("[SENSOR] Scanning I2C buses...\n");

  // Scan both buses
  DEBUG_PRINTF("Shared bus: GPIO %d (SDA), %d (SCL)\n", I2C_SDA, I2C_SCL);
  scanI2C(Wire, "SHARED BUS (Display + Ambient)");
  
  DEBUG_PRINTF("Internal bus: GPIO %d (SDA), %d (SCL)\n", INTERNAL_SDA, INTERNAL_SCL);
  scanI2C(I2C_Internal, "INTERNAL BUS");
  
  DEBUG_PRINTLN("[SENSOR] Initializing AHT21 sensors...\n");
  
  // Initialize ambient sensor (on Wire bus - shared with display)
  DEBUG_PRINTF("[SENSOR] Checking ambient sensor (GPIO %d-%d, 0x%02X)...\n", 
                I2C_SDA, I2C_SCL, AHT_ADDR);
  if (ahtAmbient.begin(&Wire, 0, AHT_ADDR)) {
    DEBUG_PRINTLN("[SENSOR] [OK] Ambient AHT21 initialized");
    ambientSensorOK = true;
    
    // Get sensor details
    DEBUG_PRINTLN("[SENSOR]   - Type: AHT21");
    DEBUG_PRINTF("[SENSOR]   - I2C Address: 0x%02X\n", AHT_ADDR);
    DEBUG_PRINTF("[SENSOR]   - Bus: GPIO %d (SDA), %d (SCL) - Shared with display\n", 
                  I2C_SDA, I2C_SCL);
  } else {
    DEBUG_PRINTLN("[SENSOR] [FAIL] Ambient sensor not found");
    ambientSensorOK = false;
  }
  
  delay(100);
  
  // Initialize internal sensor (on I2C_Internal bus)
  DEBUG_PRINTF("[SENSOR] Checking internal sensor (GPIO %d-%d, 0x%02X)...\n",
                INTERNAL_SDA, INTERNAL_SCL, AHT_ADDR);
  if (ahtInternal.begin(&I2C_Internal, 0, AHT_ADDR)) {
    DEBUG_PRINTLN("[SENSOR] [OK] Internal AHT21 initialized");
    internalSensorOK = true;
    
    // Get sensor details
    DEBUG_PRINTLN("[SENSOR]   - Type: AHT21");
    DEBUG_PRINTF("[SENSOR]   - I2C Address: 0x%02X\n", AHT_ADDR);
    DEBUG_PRINTF("[SENSOR]   - Bus: GPIO %d (SDA), %d (SCL)\n", INTERNAL_SDA, INTERNAL_SCL);
  } else {
    DEBUG_PRINTLN("[SENSOR] [FAIL] Internal sensor not found");
    internalSensorOK = false;
  }
  
  delay(100);
  
  // Final status report
  DEBUG_PRINTLN("\n[SENSOR] ============================================");
  DEBUG_PRINTLN("[SENSOR] INITIALIZATION COMPLETE");
  DEBUG_PRINTLN("[SENSOR] ============================================");
  
  if (ambientSensorOK && internalSensorOK) {
    DEBUG_PRINTLN("[SENSOR] [OK] STATUS: ALL SENSORS OPERATIONAL");
    DEBUG_PRINTF("[SENSOR]   - Ambient sensor:  GPIO %d-%d @ 0x%02X (shared with display)\n",
                  I2C_SDA, I2C_SCL, AHT_ADDR);
    DEBUG_PRINTF("[SENSOR]   - Internal sensor: GPIO %d-%d @ 0x%02X\n",
                  INTERNAL_SDA, INTERNAL_SCL, AHT_ADDR);
    DEBUG_PRINTLN("[SENSOR]   - Two separate I2C buses configured");
  } else if (ambientSensorOK) {
    DEBUG_PRINTLN("[SENSOR] [WARN] STATUS: PARTIAL - Only ambient sensor working");
    DEBUG_PRINTF("[SENSOR]   - Ambient sensor: GPIO %d-%d @ 0x%02X\n",
                  I2C_SDA, I2C_SCL, AHT_ADDR);
    DEBUG_PRINTLN("[SENSOR]   - Internal readings will use default values");
  } else if (internalSensorOK) {
    DEBUG_PRINTLN("[SENSOR] [WARN] STATUS: PARTIAL - Only internal sensor working");
    DEBUG_PRINTF("[SENSOR]   - Internal sensor: GPIO %d-%d @ 0x%02X\n",
                  INTERNAL_SDA, INTERNAL_SCL, AHT_ADDR);
    DEBUG_PRINTLN("[SENSOR]   - Ambient readings will use default values");
  } else {
    DEBUG_PRINTLN("[SENSOR] [ERROR] STATUS: CRITICAL - NO SENSORS DETECTED!");
    DEBUG_PRINTLN("[SENSOR] ============================================");
    DEBUG_PRINTLN("[SENSOR] POSSIBLE CAUSES:");
    DEBUG_PRINTLN("[SENSOR]   1. Check wiring connections");
    DEBUG_PRINTLN("[SENSOR]   2. Verify sensors have 3.3V power");
    DEBUG_PRINTLN("[SENSOR]   3. Check pull-up resistors (4.7k-10k ohm)");
    DEBUG_PRINTF("[SENSOR]   4. Ambient: GPIO %d (SDA), %d (SCL)\n", I2C_SDA, I2C_SCL);
    DEBUG_PRINTF("[SENSOR]   5. Internal: GPIO %d (SDA), %d (SCL)\n", INTERNAL_SDA, INTERNAL_SCL);
    DEBUG_PRINTLN("[SENSOR] ============================================");
    DEBUG_PRINTLN("[SENSOR] SYSTEM WILL RUN WITH DEFAULT VALUES");
  }
  DEBUG_PRINTLN("[SENSOR] ============================================\n");
}

void readSensors() {
  bool readingFailed = false;
  
  // Read ambient sensor
  if (ambientSensorOK) {
    sensors_event_t humidity, temp;
    
    if (ahtAmbient.getEvent(&humidity, &temp)) {
      // Validate temperature reading
      if (!isnan(temp.temperature) && temp.temperature > -40 && temp.temperature < 85) {
        ambient_temp = temp.temperature;
        consecutiveFailures = 0;
      } else {
        DEBUG_PRINTF("[SENSOR] [ERROR] Ambient temp invalid: %.2f\n", temp.temperature);
        readingFailed = true;
      }
      
      // Validate humidity reading
      if (!isnan(humidity.relative_humidity) && 
          humidity.relative_humidity >= 0 && 
          humidity.relative_humidity <= 100) {
        ambient_hum = humidity.relative_humidity;
      } else {
        DEBUG_PRINTF("[SENSOR] [ERROR] Ambient humidity invalid: %.2f\n", humidity.relative_humidity);
        readingFailed = true;
      }
    } else {
      DEBUG_PRINTLN("[SENSOR] [ERROR] Failed to read ambient sensor");
      readingFailed = true;
    }
  }
  
  // Read internal sensor
  if (internalSensorOK) {
    sensors_event_t humidity, temp;
    
    if (ahtInternal.getEvent(&humidity, &temp)) {
      // Validate temperature reading
      if (!isnan(temp.temperature) && temp.temperature > -40 && temp.temperature < 85) {
        internal_temp = temp.temperature;
      } else {
        DEBUG_PRINTF("[SENSOR] [ERROR] Internal temp invalid: %.2f\n", temp.temperature);
        readingFailed = true;
      }
      
      // Validate humidity reading
      if (!isnan(humidity.relative_humidity) && 
          humidity.relative_humidity >= 0 && 
          humidity.relative_humidity <= 100) {
        internal_hum = humidity.relative_humidity;
      } else {
        DEBUG_PRINTF("[SENSOR] [ERROR] Internal humidity invalid: %.2f\n", humidity.relative_humidity);
        readingFailed = true;
      }
    } else {
      DEBUG_PRINTLN("[SENSOR] [ERROR] Failed to read internal sensor");
      readingFailed = true;
    }
  }
  
  // Handle consecutive failures
  if (readingFailed) {
    consecutiveFailures++;
    
    if (consecutiveFailures >= MAX_FAILURES) {
      DEBUG_PRINTLN("\n[SENSOR] [WARN] CRITICAL: Too many consecutive failures!");
      DEBUG_PRINTF("[SENSOR] Failed readings: %d/%d\n", consecutiveFailures, MAX_FAILURES);
      DEBUG_PRINTLN("[SENSOR] Re-scanning I2C buses...\n");
      scanI2C(Wire, "SHARED BUS");
      scanI2C(I2C_Internal, "INTERNAL BUS");
      consecutiveFailures = 0;
    }
  }
}