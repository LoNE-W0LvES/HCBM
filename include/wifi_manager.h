#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// WiFi connection modes
enum WiFiMode {
  WIFI_CLIENT_MODE,
  WIFI_AP_MODE
};

// Initialize WiFi manager
void initWiFiManager();

// Start WiFi in client mode (connect to router) - non-blocking
bool startWiFiClient();

// Update WiFi connection status (call this in loop)
bool updateWiFiConnection();

// Start WiFi in AP mode (hotspot)
void startWiFiAP();

// Check WiFi status and reconnect if needed (non-blocking)
void handleWiFiConnection();

// Save WiFi credentials to preferences
void saveWiFiCredentials(const char* ssid, const char* password);

// Load WiFi credentials from preferences
bool loadWiFiCredentials(String &ssid, String &password);

// Clear saved WiFi credentials
void clearWiFiCredentials();

// Scan for available WiFi networks
String scanWiFiNetworks();

// Get current WiFi mode
WiFiMode getWiFiMode();

// Get WiFi status string
String getWiFiStatus();

// Get IP address
String getIPAddress();

// Check if WiFi is connected
bool isWiFiConnected();

// Check if WiFi is intentionally disabled
bool isWiFiDisabled();

// Check if WiFi is currently connecting
bool isWiFiConnecting();

#endif