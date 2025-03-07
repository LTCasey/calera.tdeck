/**
 * @file wifi.h
 * @brief WiFi management for T-Deck UI Firmware
 * 
 * This file contains the WiFi manager class which handles
 * WiFi client/AP functionalities, scanning, connecting, and
 * managing WiFi configuration.
 */

 #ifndef TDECK_WIFI_H
 #define TDECK_WIFI_H
 
 #include <Arduino.h>
 #include <WiFi.h>
 #include <vector>
 #include <ArduinoJson.h>
 #include "../config.h"
 #include "../system/fs_manager.h"
 
 // External declaration of file system manager
 extern FSManager fsManager;
 
 /**
  * @brief Structure to hold WiFi network information
  */
 struct WiFiNetwork {
     String ssid;
     int32_t rssi;
     uint8_t encryptionType;
     String bssid;
     int32_t channel;
     bool isConnected;
 };
 
 /**
  * @brief Structure to hold WiFi configuration
  */
 struct WiFiConfig {
     bool autoConnect;          // Auto-connect to last known network on startup
     bool preferredNetworkOnly; // Only connect to networks in the preferred list
     std::vector<String> preferredNetworks; // List of preferred network SSIDs
     String lastConnectedSsid;  // Last successfully connected network
     String apSsid;             // Custom AP SSID (if not using default)
     String apPassword;         // Custom AP password (if not using default)
     uint8_t apChannel;         // Custom AP channel (if not using default)
     bool apHidden;             // Whether to hide AP SSID
     uint8_t apMaxConnections;  // Maximum number of AP connections
 };
 
 /**
  * @brief Manages WiFi connectivity and configuration
  */
 class WiFiManager {
 public:
     /**
      * @brief Construct a new WiFi Manager object
      */
     WiFiManager();
 
     /**
      * @brief Initialize WiFi functionality
      * 
      * @return true if initialization successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Update WiFi status (called periodically)
      */
     void update();
 
     /**
      * @brief Scan for available WiFi networks
      * 
      * @return true if scan started successfully
      * @return false if scan failed to start
      */
     bool startScan();
 
     /**
      * @brief Get results of the last WiFi scan
      * 
      * @return std::vector<WiFiNetwork> List of found WiFi networks
      */
     std::vector<WiFiNetwork> getScanResults();
 
     /**
      * @brief Check if WiFi scan is in progress
      * 
      * @return true if scan is in progress
      * @return false if no scan is in progress
      */
     bool isScanInProgress();
 
     /**
      * @brief Connect to a WiFi network
      * 
      * @param ssid Network SSID
      * @param password Network password
      * @param saveCredentials Whether to save credentials for future reconnection
      * @return true if connection attempt started
      * @return false if connection attempt failed to start
      */
     bool connect(const String& ssid, const String& password, bool saveCredentials = true);
 
     /**
      * @brief Disconnect from the current WiFi network
      */
     void disconnect();
 
     /**
      * @brief Get the current connection status
      * 
      * @return true if connected to a WiFi network
      * @return false if not connected
      */
     bool isConnected();
 
     /**
      * @brief Get the current connection information
      * 
      * @param info Pointer to WiFiNetwork struct to fill with current connection info
      * @return true if currently connected and info was filled
      * @return false if not connected
      */
     bool getConnectionInfo(WiFiNetwork* info);
 
     /**
      * @brief Start WiFi Access Point mode
      * 
      * @param customSsid Optional custom SSID (if nullptr, uses configured/default)
      * @param customPassword Optional custom password (if nullptr, uses configured/default)
      * @return true if AP started successfully
      * @return false if AP failed to start
      */
     bool startAP(const char* customSsid = nullptr, const char* customPassword = nullptr);
 
     /**
      * @brief Stop WiFi Access Point mode
      */
     void stopAP();
 
     /**
      * @brief Check if AP mode is active
      * 
      * @return true if AP mode is active
      * @return false if AP mode is not active
      */
     bool isAPActive();
 
     /**
      * @brief Get number of stations connected to AP
      * 
      * @return uint8_t Number of connected stations
      */
     uint8_t getAPStationCount();
 
     /**
      * @brief Save WiFi configuration
      * 
      * @return true if configuration saved successfully
      * @return false if configuration save failed
      */
     bool saveConfig();
 
     /**
      * @brief Load WiFi configuration
      * 
      * @return true if configuration loaded successfully
      * @return false if configuration load failed
      */
     bool loadConfig();
 
     /**
      * @brief Get local IP address (either as client or AP)
      * 
      * @return String IP address as string
      */
     String getLocalIP();
 
     /**
      * @brief Get WiFi signal strength (RSSI)
      * 
      * @return int32_t Signal strength in dBm or 0 if not connected
      */
     int32_t getRSSI();
 
     /**
      * @brief Get the current WiFi status
      * 
      * @return wl_status_t WiFi status (from WiFi.status())
      */
     wl_status_t getStatus();
 
     /**
      * @brief Get WiFi MAC address
      * 
      * @return String MAC address as string
      */
     String getMacAddress();
 
     /**
      * @brief Add a network to preferred networks list
      * 
      * @param ssid Network SSID to add
      * @return true if added successfully
      * @return false if already in list or failed to save
      */
     bool addPreferredNetwork(const String& ssid);
 
     /**
      * @brief Remove a network from preferred networks list
      * 
      * @param ssid Network SSID to remove
      * @return true if removed successfully
      * @return false if not in list or failed to save
      */
     bool removePreferredNetwork(const String& ssid);
 
     /**
      * @brief Get current WiFi configuration
      * 
      * @return const WiFiConfig& Reference to current configuration
      */
     const WiFiConfig& getConfig() const;
 
 private:
     WiFiConfig _config;                   // WiFi configuration
     std::vector<WiFiNetwork> _scanResults; // Results of the last scan
     bool _scanInProgress;                 // Whether a scan is currently in progress
     unsigned long _lastScanTime;          // Timestamp of the last scan
     unsigned long _lastStatusCheck;       // Timestamp of the last status check
     wl_status_t _lastStatus;              // Last WiFi status
     bool _reconnectAttempted;             // Whether a reconnect has been attempted
     unsigned long _connectionStartTime;    // When current connection attempt started
     unsigned long _connectionTimeout;      // Timeout for connection attempt
 
     /**
      * @brief Convert WiFi encryption type to readable string
      * 
      * @param encType Encryption type code
      * @return String Human-readable encryption type
      */
     String encryptionTypeToString(uint8_t encType);
 
     /**
      * @brief Create default configuration
      */
     void createDefaultConfig();
 
     /**
      * @brief Handle connect timeout
      */
     void handleConnectTimeout();
 
     /**
      * @brief Automatically connect to preferred networks
      * 
      * @return true if connection attempt started
      * @return false if no suitable network found
      */
     bool autoConnectToPreferred();
 };
 
 #endif // TDECK_WIFI_H