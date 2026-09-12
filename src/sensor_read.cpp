// sensor_read.cpp - AHT21 Ambient + AM2315C Internal
#include "sensor_read.h"
#include "config.h"  // IMPORTANT: Use pins from config.h
#include "variables.h"
#include "events.h"
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <AM2315C.h>

// Forward declarations for calibration offsets
extern float internal_temp_offset;
extern float internal_hum_offset;
extern void saveConfiguration();  // From storage_manager.cpp

// I2C bus for internal sensor (must be defined before sensor objects)
TwoWire I2C_Internal = TwoWire(1);

// Sensor objects (AM2315C takes TwoWire pointer in constructor)
Adafruit_AHTX0 ahtAmbient;
AM2315C am2315Internal(&I2C_Internal);

// I2C address for both sensors (same address, different I2C buses)
#define AHT_ADDR 0x38

// State tracking
static int consecutiveFailures = 0;
const int MAX_FAILURES = 5;
static bool ambientSensorOK = false;
static bool internalSensorOK = false;
static unsigned long lastInternalRead = 0;
const unsigned long INTERNAL_READ_INTERVAL = 1000;  // 1 second minimum between reads

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
      else if (address == 0x38) DEBUG_PRINT(" (AHT21/AM2315C - Temp/Humidity Sensor)");
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
  DEBUG_PRINTLN("[SENSOR] Sensor Initialization (AHT21 Ambient + AM2315C Internal)");
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
  I2C_Internal.begin(INTERNAL_SDA, INTERNAL_SCL, 50000);  // Reduced to 50kHz for stability
  delay(500);

  DEBUG_PRINTLN("[SENSOR] Scanning I2C buses...\n");

  // Scan both buses
  DEBUG_PRINTF("Shared bus: GPIO %d (SDA), %d (SCL)\n", I2C_SDA, I2C_SCL);
  scanI2C(Wire, "SHARED BUS (Display + Ambient)");
  
  DEBUG_PRINTF("Internal bus: GPIO %d (SDA), %d (SCL)\n", INTERNAL_SDA, INTERNAL_SCL);
  scanI2C(I2C_Internal, "INTERNAL BUS");
  
  DEBUG_PRINTLN("[SENSOR] Initializing sensors...\n");

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

  // Initialize internal sensor (on I2C_Internal bus) - AM2315C (RobTillaart library)
  DEBUG_PRINTF("[SENSOR] Checking internal sensor (GPIO %d-%d, 0x%02X)...\n",
                INTERNAL_SDA, INTERNAL_SCL, AHT_ADDR);
  if (am2315Internal.begin()) {
    DEBUG_PRINTLN("[SENSOR] [OK] Internal AM2315C initialized");
    internalSensorOK = true;

    // Apply calibration offsets
    am2315Internal.setTempOffset(internal_temp_offset);
    am2315Internal.setHumOffset(internal_hum_offset);

    // Get sensor details
    DEBUG_PRINTLN("[SENSOR]   - Type: AM2315C (AHT20 encased)");
    DEBUG_PRINTF("[SENSOR]   - I2C Address: 0x%02X (fixed)\n", AHT_ADDR);
    DEBUG_PRINTF("[SENSOR]   - Bus: GPIO %d (SDA), %d (SCL)\n", INTERNAL_SDA, INTERNAL_SCL);
    if (internal_temp_offset != 0.0 || internal_hum_offset != 0.0) {
      DEBUG_PRINTF("[SENSOR]   - Calibration offsets (from storage): Temp %+.2f°C, Humidity %+.2f%%\n",
                    internal_temp_offset, internal_hum_offset);
    } else {
      DEBUG_PRINTLN("[SENSOR]   - Calibration offsets: None (using raw sensor values)");
    }
  } else {
    DEBUG_PRINTLN("[SENSOR] [FAIL] Internal sensor not found");
    internalSensorOK = false;
  }

  // Set initialization time for first read timing
  lastInternalRead = millis();

  // Final status report
  DEBUG_PRINTLN("\n[SENSOR] ============================================");
  DEBUG_PRINTLN("[SENSOR] INITIALIZATION COMPLETE");
  DEBUG_PRINTLN("[SENSOR] ============================================");
  
  if (ambientSensorOK && internalSensorOK) {
    DEBUG_PRINTLN("[SENSOR] [OK] STATUS: ALL SENSORS OPERATIONAL");
    DEBUG_PRINTF("[SENSOR]   - Ambient sensor:  AHT21 @ 0x%02X (GPIO %d-%d, shared with display)\n",
                  AHT_ADDR, I2C_SDA, I2C_SCL);
    DEBUG_PRINTF("[SENSOR]   - Internal sensor: AM2315C @ 0x%02X (GPIO %d-%d)\n",
                  AHT_ADDR, INTERNAL_SDA, INTERNAL_SCL);
    DEBUG_PRINTLN("[SENSOR]   - Two separate I2C buses configured");
  } else if (ambientSensorOK) {
    DEBUG_PRINTLN("[SENSOR] [WARN] STATUS: PARTIAL - Only ambient sensor working");
    DEBUG_PRINTF("[SENSOR]   - Ambient sensor: AHT21 @ 0x%02X (GPIO %d-%d)\n",
                  AHT_ADDR, I2C_SDA, I2C_SCL);
    DEBUG_PRINTLN("[SENSOR]   - Internal readings will use default values");
  } else if (internalSensorOK) {
    DEBUG_PRINTLN("[SENSOR] [WARN] STATUS: PARTIAL - Only internal sensor working");
    DEBUG_PRINTF("[SENSOR]   - Internal sensor: AM2315C @ 0x%02X (GPIO %d-%d)\n",
                  AHT_ADDR, INTERNAL_SDA, INTERNAL_SCL);
    DEBUG_PRINTLN("[SENSOR]   - Ambient readings will use default values");
  } else {
    DEBUG_PRINTLN("[SENSOR] [ERROR] STATUS: CRITICAL - NO SENSORS DETECTED!");
    DEBUG_PRINTLN("[SENSOR] ============================================");
    DEBUG_PRINTLN("[SENSOR] POSSIBLE CAUSES:");
    DEBUG_PRINTLN("[SENSOR]   1. Check wiring connections");
    DEBUG_PRINTLN("[SENSOR]   2. Verify sensors have 3.3V power");
    DEBUG_PRINTLN("[SENSOR]   3. Check pull-up resistors (4.7k-10k ohm)");
    DEBUG_PRINTF("[SENSOR]   4. Ambient AHT21: 0x%02X @ GPIO %d (SDA), %d (SCL)\n", AHT_ADDR, I2C_SDA, I2C_SCL);
    DEBUG_PRINTF("[SENSOR]   5. Internal AM2315C: 0x%02X @ GPIO %d (SDA), %d (SCL)\n", AHT_ADDR, INTERNAL_SDA, INTERNAL_SCL);
    DEBUG_PRINTLN("[SENSOR] ============================================");
    DEBUG_PRINTLN("[SENSOR] SYSTEM WILL RUN WITH DEFAULT VALUES");
  }
  DEBUG_PRINTLN("[SENSOR] ============================================\n");
}

void readSensors() {
  bool readingFailed = false;

  // Read ambient sensor (AHT21)
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

      // Debug output
      DEBUG_PRINTF("[SENSOR] Ambient (AHT21): Temp=%.2f°C, Humidity=%.2f%%\n",
                    ambient_temp, ambient_hum);
    } else {
      DEBUG_PRINTLN("[SENSOR] [ERROR] Failed to read ambient sensor");
      readingFailed = true;
    }
  }

  // Read internal sensor (AM2315C) - non-blocking, respects 1000ms minimum between reads
  if (internalSensorOK) {
    unsigned long currentTime = millis();
    unsigned long timeSinceLastRead = currentTime - lastInternalRead;

    // Only attempt read if 1000ms has elapsed since last read
    if (timeSinceLastRead >= INTERNAL_READ_INTERVAL) {
      int readStatus = am2315Internal.read();
      lastInternalRead = currentTime;  // Update timestamp for next read

      if (readStatus == AM2315C_OK) {
        // Get temperature and humidity from AM2315C (BEFORE offset applied by library)
        float raw_temp = am2315Internal.getTemperature();
        float raw_hum = am2315Internal.getHumidity();

        // Validate temperature reading
        if (!isnan(raw_temp) && raw_temp > -40 && raw_temp < 85) {
          internal_temp = raw_temp;
        } else {
          DEBUG_PRINTF("[SENSOR] [ERROR] Internal temp invalid: %.2f\n", raw_temp);
          readingFailed = true;
        }

        // Validate humidity reading
        if (!isnan(raw_hum) && raw_hum >= 0 && raw_hum <= 100) {
          internal_hum = raw_hum;
        } else {
          DEBUG_PRINTF("[SENSOR] [ERROR] Internal humidity invalid: %.2f\n", raw_hum);
          readingFailed = true;
        }

        // DEBUG: Show raw values BEFORE and AFTER offset correction
        // Library ADDS offset: result = raw + offset
        float corrected_temp = raw_temp + internal_temp_offset;
        float corrected_hum = raw_hum + internal_hum_offset;
        DEBUG_PRINTF("[SENSOR] Internal RAW: %.2f°C, %.2f%% | CORRECTED: %.2f°C, %.2f%% | Offsets: %+.2f°C, %+.2f%%\n",
                      raw_temp, raw_hum, corrected_temp, corrected_hum,
                      internal_temp_offset, internal_hum_offset);

        // Debug output with discrepancy calculation
        float hum_diff = internal_hum - ambient_hum;
        DEBUG_PRINTF("[SENSOR] Internal (AM2315C): Temp=%.2f°C, Humidity=%.2f%% (Δ%.2f%% vs Ambient)\n",
                      internal_temp, internal_hum, hum_diff);
      } else {
        DEBUG_PRINTF("[SENSOR] [ERROR] Failed to read internal sensor (status: %d)\n", readStatus);
        readingFailed = true;
      }
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

// Update sensor calibration offsets at runtime
// Note: This function updates variables and the sensor library, but NOT storage!
// The caller must call saveConfiguration() to persist to flash
void setSensorOffsets(float tempOffset, float humOffset) {
  internal_temp_offset = tempOffset;
  internal_hum_offset = humOffset;

  if (internalSensorOK) {
    am2315Internal.setTempOffset(tempOffset);
    am2315Internal.setHumOffset(humOffset);
    DEBUG_PRINTF("[SENSOR] Calibration offsets updated: Temp %+.2f°C, Humidity %+.2f%%\n",
                  tempOffset, humOffset);
  }
}

// Auto-calibrate internal sensor against ambient sensor
// Call this when sensors are placed next to each other
String autoCalibrateInternalSensor() {
  if (!ambientSensorOK || !internalSensorOK) {
    return "{\"error\":\"Both sensors must be operational\"}";
  }

  DEBUG_PRINTLN("\n[SENSOR] ========== AUTO-CALIBRATION STARTED ==========");
  DEBUG_PRINTLN("[SENSOR] Reading both sensors for calibration...");
  DEBUG_PRINTLN("[SENSOR] Waiting 2 seconds before starting measurements...");
  delay(2000);

  // Take multiple readings to get accurate average
  float ambient_temp_sum = 0, ambient_hum_sum = 0;
  float internal_temp_sum = 0, internal_hum_sum = 0;
  int valid_samples = 0;
  const int calibration_samples = 5;

  for (int i = 0; i < calibration_samples; i++) {
    DEBUG_PRINTF("[SENSOR] Sample %d/%d...\n", i + 1, calibration_samples);

    // Read ambient sensor
    sensors_event_t humidity, temp;
    if (ahtAmbient.getEvent(&humidity, &temp)) {
      ambient_temp_sum += temp.temperature;
      ambient_hum_sum += humidity.relative_humidity;
    }

    // Read internal sensor - AM2315C needs 1000ms+ between reads
    delay(100);
    int read_status = am2315Internal.read();
    if (read_status == AM2315C_OK) {
      internal_temp_sum += am2315Internal.getTemperature();
      internal_hum_sum += am2315Internal.getHumidity();
      valid_samples++;
      DEBUG_PRINTF("[SENSOR]   Read OK: %.2f°C, %.2f%%\n",
                    am2315Internal.getTemperature(),
                    am2315Internal.getHumidity());
    } else {
      DEBUG_PRINTF("[SENSOR]   Read FAILED (status: %d)\n", read_status);
    }

    // Wait 1200ms before next read (AM2315C spec is 1000ms minimum)
    if (i < calibration_samples - 1) {
      delay(1200);
    }
  }

  // Check if we have valid samples
  if (valid_samples == 0) {
    DEBUG_PRINTLN("[SENSOR] ERROR: No valid internal sensor readings during calibration!");
    return "{\"error\":\"Internal sensor failed to read\"}";
  }

  // Calculate averages
  float ambient_temp_avg = ambient_temp_sum / calibration_samples;
  float ambient_hum_avg = ambient_hum_sum / calibration_samples;
  float internal_temp_avg = internal_temp_sum / valid_samples;
  float internal_hum_avg = internal_hum_sum / valid_samples;

  // Calculate offsets to match ambient readings
  // RobTillaart library ADDS the offset, so we need NEGATIVE values
  // to reduce the reading: result = raw + offset, so offset = target - raw
  float new_temp_offset = ambient_temp_avg - internal_temp_avg;
  float new_hum_offset = ambient_hum_avg - internal_hum_avg;

  DEBUG_PRINTF("[SENSOR] Ambient average (%d samples):  Temp=%.2f°C, Humidity=%.2f%%\n",
                calibration_samples, ambient_temp_avg, ambient_hum_avg);
  DEBUG_PRINTF("[SENSOR] Internal average (%d valid):   Temp=%.2f°C, Humidity=%.2f%%\n",
                valid_samples, internal_temp_avg, internal_hum_avg);
  DEBUG_PRINTF("[SENSOR] Calculated offsets: Temp %+.2f°C, Humidity %+.2f%%\n",
                new_temp_offset, new_hum_offset);

  // Apply the offsets
  setSensorOffsets(new_temp_offset, new_hum_offset);

  // Save to flash memory so it persists after restart
  saveConfiguration();
  DEBUG_PRINTLN("[SENSOR] Offsets saved to storage");

  DEBUG_PRINTLN("[SENSOR] ========== AUTO-CALIBRATION COMPLETE ==========\n");

  // Return result as JSON
  char result[256];
  snprintf(result, sizeof(result),
           "{\"success\":true,\"ambient_temp\":%.2f,\"ambient_hum\":%.2f,"
           "\"internal_temp\":%.2f,\"internal_hum\":%.2f,"
           "\"offset_temp\":%.2f,\"offset_hum\":%.2f,\"valid_samples\":%d}",
           ambient_temp_avg, ambient_hum_avg,
           internal_temp_avg, internal_hum_avg,
           new_temp_offset, new_hum_offset,
           valid_samples);

  return String(result);
}