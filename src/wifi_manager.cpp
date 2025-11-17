#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

// State variables
static WiFiMode currentMode = WIFI_CLIENT_MODE;
static unsigned long lastConnectionAttempt = 0;
static unsigned long connectionStartTime = 0;
static bool connectionInProgress = false;
static String savedSSID = "";
static String savedPassword = "";
static Preferences wifiPrefs;
static bool wifiDisabled = false;

// Connection states
enum ConnectionState {
  CONN_IDLE,
  CONN_STARTING,
  CONN_CONNECTING,
  CONN_SUCCESS,
  CONN_FAILED
};
static ConnectionState connState = CONN_IDLE;

// Scan state management
static SemaphoreHandle_t scanMutex = NULL;
static String scanResults = "[]";
static bool scanInProgress = false;

void initWiFiManager() {
  wifiPrefs.begin("wificfg", false);
  
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  
  WiFi.mode(WIFI_OFF);
  delay(200);
  
  DEBUG_PRINTLN("[WIFI] WiFi Manager initialized.");
}

bool loadWiFiCredentials(String &ssid, String &password) {
  ssid = wifiPrefs.getString("ssid", "");
  password = wifiPrefs.getString("password", "");
  
  if (ssid.length() > 0) {
    savedSSID = ssid;
    savedPassword = password;
    DEBUG_PRINTF("[WIFI] Loaded credentials:\n");
    DEBUG_PRINTF("[WIFI]   SSID: '%s'\n", ssid.c_str());
    DEBUG_PRINTF("[WIFI]   Password: '%s'\n", password.c_str());
    return true;
  }
  
  DEBUG_PRINTLN("[WIFI] No saved credentials found.");
  return false;
}

void saveWiFiCredentials(const char* ssid, const char* password) {
  wifiPrefs.putString("ssid", ssid);
  wifiPrefs.putString("password", password);
  savedSSID = String(ssid);
  savedPassword = String(password);
  wifiDisabled = false;
  DEBUG_PRINTF("[WIFI] Credentials saved:\n");
  DEBUG_PRINTF("[WIFI]   SSID: '%s'\n", ssid);
  DEBUG_PRINTF("[WIFI]   Password: '%s'\n", password);
}

void clearWiFiCredentials() {
  wifiPrefs.remove("ssid");
  wifiPrefs.remove("password");
  savedSSID = "";
  savedPassword = "";
  
  WiFi.disconnect(true, true);
  delay(100);
  
  DEBUG_PRINTLN("[WIFI] Credentials cleared. Will start AP mode on restart.");
}

bool startWiFiClient() {
  if (savedSSID.length() == 0) {
    DEBUG_PRINTLN("[WIFI] No credentials available for client mode.");
    return false;
  }
  
  WiFi.disconnect(true);
  delay(100);
  
  WiFi.mode(WIFI_STA);
  delay(100);
  
  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  
  DEBUG_PRINTF("[WIFI] Starting connection to '%s'", savedSSID.c_str());
  
  connectionStartTime = millis();
  connectionInProgress = true;
  connState = CONN_STARTING;
  lastConnectionAttempt = millis();
  
  return true;  // Return true to indicate connection attempt started
}

// Non-blocking connection check - call this in loop()
bool updateWiFiConnection() {
  if (!connectionInProgress) {
    return false;
  }
  
  wl_status_t status = WiFi.status();
  unsigned long elapsed = millis() - connectionStartTime;
  
  // Print dots every second
  static unsigned long lastDot = 0;
  if (millis() - lastDot > 1000) {
    DEBUG_PRINT(".");
    lastDot = millis();
  }
  
  // Check if connected
  if (status == WL_CONNECTED) {
    connectionInProgress = false;
    connState = CONN_SUCCESS;
    currentMode = WIFI_CLIENT_MODE;
    wifiDisabled = false;
    
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("[WIFI] ✓ Connected!");
    DEBUG_PRINTF("[WIFI] IP Address: %s\n", WiFi.localIP().toString().c_str());
    DEBUG_PRINTF("[WIFI] Signal: %d dBm\n", WiFi.RSSI());
    
    return true;
  }
  
  // Check for timeout
  if (elapsed >= WIFI_TIMEOUT_MS) {
    connectionInProgress = false;
    connState = CONN_FAILED;
    
    DEBUG_PRINTLN("");
    DEBUG_PRINTLN("[WIFI] ✗ Connection failed (timeout).");
    DEBUG_PRINTF("[WIFI] Status code: %d\n", status);
    DEBUG_PRINTLN("[WIFI] WiFi will remain off. Hold button to start hotspot.");
    
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_OFF);
    wifiDisabled = true;
    
    return false;
  }
  
  return false;  // Still connecting
}

void startWiFiAP() {
  WiFi.disconnect(true);
  delay(100);
  
  WiFi.mode(WIFI_OFF);
  delay(100);
  
  WiFi.mode(WIFI_AP);
  delay(100);
  
  bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
  
  if (!apStarted) {
    DEBUG_PRINTLN("[WIFI] ✗ Failed to start AP! Retrying...");
    delay(500);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
  }
  
  currentMode = WIFI_AP_MODE;
  wifiDisabled = false;
  
  DEBUG_PRINTLN("\n[WIFI] ╔═══════════════════════════╗");
  DEBUG_PRINTLN("[WIFI] 📶 Access Point Mode Enabled");
  DEBUG_PRINTLN("[WIFI] ╠═══════════════════════════╣");
  DEBUG_PRINTF("[WIFI] SSID: %s\n", AP_SSID);
  DEBUG_PRINTF("[WIFI] Password: %s\n", AP_PASSWORD);
  DEBUG_PRINTF("[WIFI] IP Address: %s\n", WiFi.softAPIP().toString().c_str());
  DEBUG_PRINTF("[WIFI] AP Status: %s\n", apStarted ? "RUNNING" : "FAILED");
  DEBUG_PRINTF("[WIFI] Connected Clients: %d\n", WiFi.softAPgetStationNum());
  DEBUG_PRINTLN("[WIFI] ╠═══════════════════════════╣");
  DEBUG_PRINTLN("[WIFI] Connect to setup WiFi");
  DEBUG_PRINTLN("[WIFI] ╚═══════════════════════════╝\n");
}

void handleWiFiConnection() {
  // If connection is in progress, update it
  if (connectionInProgress) {
    updateWiFiConnection();
    return;
  }
  
  // Don't try to reconnect if disabled or in AP mode
  if (wifiDisabled || currentMode == WIFI_AP_MODE) {
    return;
  }
  
  // Already connected
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }
  
  // Check if enough time has passed since last attempt
  if (millis() - lastConnectionAttempt < WIFI_RETRY_INTERVAL_MS) {
    return;
  }
  
  // Try to reconnect
  if (savedSSID.length() > 0) {
    DEBUG_PRINTLN("[WIFI] Connection lost. Attempting to reconnect...");
    startWiFiClient();
  }
}

WiFiMode getWiFiMode() {
  return currentMode;
}

String getWiFiStatus() {
  if (connectionInProgress) {
    return "Connecting...";
  }
  
  if (wifiDisabled && currentMode != WIFI_AP_MODE) {
    return "WiFi Off";
  }
  
  if (currentMode == WIFI_AP_MODE) {
    return "AP Mode (Clients: " + String(WiFi.softAPgetStationNum()) + ")";
  }
  
  switch (WiFi.status()) {
    case WL_CONNECTED:
      return "Connected";
    case WL_NO_SSID_AVAIL:
      return "SSID Not Found";
    case WL_CONNECT_FAILED:
      return "Connection Failed";
    case WL_CONNECTION_LOST:
      return "Connection Lost";
    case WL_DISCONNECTED:
      return "Disconnected";
    case WL_IDLE_STATUS:
      return "Idle";
    default:
      return "Unknown (" + String(WiFi.status()) + ")";
  }
}

String getIPAddress() {
  if (currentMode == WIFI_AP_MODE) {
    return WiFi.softAPIP().toString();
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  
  return "0.0.0.0";
}

bool isWiFiConnected() {
  if (currentMode == WIFI_AP_MODE) return true;
  return WiFi.status() == WL_CONNECTED;
}

bool isWiFiDisabled() {
  return wifiDisabled && currentMode != WIFI_AP_MODE;
}

bool isWiFiConnecting() {
  return connectionInProgress;
}

// Task for WiFi scanning
void wifiScanTask(void* parameter) {
  DEBUG_PRINTLN("[WIFI] Scan task started");
  
  // Perform the scan
  int n = WiFi.scanNetworks(false, false, false, 200);
  
  String json = "[";
  
  if (n > 0) {
    DEBUG_PRINTF("[WIFI] Found %d networks\n", n);
    
    for (int i = 0; i < n; i++) {
      if (i > 0) json += ",";
      
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json += "\"encryption\":" + String(WiFi.encryptionType(i));
      json += "}";
    }
    
    WiFi.scanDelete();
  } else if (n == 0) {
    DEBUG_PRINTLN("[WIFI] No networks found.");
  } else {
    DEBUG_PRINTF("[WIFI] Scan failed with error: %d\n", n);
  }
  
  json += "]";
  
  // Store results
  if (xSemaphoreTake(scanMutex, portMAX_DELAY) == pdTRUE) {
    scanResults = json;
    scanInProgress = false;
    xSemaphoreGive(scanMutex);
  }
  
  DEBUG_PRINTLN("[WIFI] Scan task complete");
  vTaskDelete(NULL);
}

String scanWiFiNetworks() {
  // Initialize mutex on first call
  if (scanMutex == NULL) {
    scanMutex = xSemaphoreCreateMutex();
  }
  
  DEBUG_PRINTLN("[WIFI] Starting network scan...");
  
  // Check if scan is already in progress
  if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    if (scanInProgress) {
      String result = scanResults;
      xSemaphoreGive(scanMutex);
      DEBUG_PRINTLN("[WIFI] Scan already in progress, returning cached results");
      return result;
    }
    
    scanInProgress = true;
    scanResults = "[]"; // Reset
    xSemaphoreGive(scanMutex);
  } else {
    DEBUG_PRINTLN("[WIFI] Could not acquire scan mutex");
    return "[]";
  }
  
  // Temporarily enable WiFi for scanning if disabled
  bool wasDisabled = wifiDisabled;
  bool wasAPMode = (currentMode == WIFI_AP_MODE);
  
  if (!wasAPMode) {
    WiFi.mode(WIFI_STA);
    delay(100);
  }
  
  // Create scan task on core 1 (away from AsyncTCP on core 0)
  TaskHandle_t scanTaskHandle = NULL;
  xTaskCreatePinnedToCore(
    wifiScanTask,      // Task function
    "WiFiScan",        // Task name
    4096,              // Stack size
    NULL,              // Parameters
    1,                 // Priority
    &scanTaskHandle,   // Task handle
    1                  // Core 1
  );
  
  // Wait for scan to complete (with timeout)
  unsigned long startWait = millis();
  while (scanInProgress && (millis() - startWait < 15000)) {
    delay(100);
    // Allow other tasks to run
    vTaskDelay(1);
  }
  
  // Get results
  String result = "[]";
  if (xSemaphoreTake(scanMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    result = scanResults;
    xSemaphoreGive(scanMutex);
  }
  
  // Restore previous state
  if (wasDisabled) {
    wifiDisabled = true;
  }
  
  DEBUG_PRINTLN("[WIFI] Scan complete");
  return result;
}