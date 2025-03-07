/**
 * @file wifi.cpp
 * @brief WiFi management implementation for T-Deck UI Firmware
 */

 #include "wifi.h"

 // Constructor
 WiFiManager::WiFiManager()
     : _scanInProgress(false)
     , _lastScanTime(0)
     , _lastStatusCheck(0)
     , _lastStatus(WL_DISCONNECTED)
     , _reconnectAttempted(false)
     , _connectionStartTime(0)
     , _connectionTimeout(30000) // 30 second timeout for connections
 {
     // Initialize with default values
     createDefaultConfig();
 }
 
 // Initialize WiFi functionality
 bool WiFiManager::init() {
     TDECK_LOG_I("Initializing WiFi manager");
 
     // Load WiFi configuration from file
     if (!loadConfig()) {
         TDECK_LOG_W("Failed to load WiFi config, using defaults");
         createDefaultConfig();
         saveConfig(); // Create default config file
     }
 
     // Set WiFi mode to station initially
     WiFi.mode(WIFI_STA);
     
     // Set hostname
     WiFi.setHostname("tdeck");
     
     // Set power save mode to NONE for better response time
     // Use WiFi.setSleep(true) to enable modem sleep for power saving if needed
     WiFi.setSleep(false);
 
     // Auto-connect if enabled in config
     if (_config.autoConnect && !_config.lastConnectedSsid.isEmpty()) {
         TDECK_LOG_I("Auto-connecting to last network: %s", _config.lastConnectedSsid.c_str());
         for (const auto& net : _config.preferredNetworks) {
             if (net == _config.lastConnectedSsid) {
                 // Found the network in preferred list, try to connect
                 // This requires the password to be saved in the ESP32's NVS
                 // from a previous successful connection
                 WiFi.begin();
                 _connectionStartTime = millis();
                 return true;
             }
         }
     }
 
     return true;
 }
 
 // Update WiFi status (called periodically)
 void WiFiManager::update() {
     unsigned long currentTime = millis();
     
     // Check connection status periodically (every second)
     if (currentTime - _lastStatusCheck >= 1000) {
         _lastStatusCheck = currentTime;
         
         wl_status_t currentStatus = WiFi.status();
         
         // Status changed
         if (currentStatus != _lastStatus) {
             _lastStatus = currentStatus;
             
             switch (currentStatus) {
                 case WL_CONNECTED:
                     TDECK_LOG_I("WiFi connected to: %s", WiFi.SSID().c_str());
                     TDECK_LOG_I("IP address: %s", WiFi.localIP().toString().c_str());
                     
                     // Update last connected network if not already set
                     if (_config.lastConnectedSsid != WiFi.SSID()) {
                         _config.lastConnectedSsid = WiFi.SSID();
                         saveConfig();
                     }
                     break;
                     
                 case WL_DISCONNECTED:
                     TDECK_LOG_I("WiFi disconnected");
                     break;
                     
                 case WL_CONNECT_FAILED:
                     TDECK_LOG_W("WiFi connection failed");
                     break;
                     
                 case WL_NO_SSID_AVAIL:
                     TDECK_LOG_W("WiFi SSID not available");
                     break;
                     
                 default:
                     TDECK_LOG_I("WiFi status changed: %d", currentStatus);
                     break;
             }
         }
         
         // Handle connection timeout
         if (currentStatus == WL_CONNECTING && 
             _connectionStartTime > 0 && 
             currentTime - _connectionStartTime > _connectionTimeout) {
             handleConnectTimeout();
         }
         
         // Auto-reconnect logic (if disconnected and not already trying to reconnect)
         if (currentStatus == WL_DISCONNECTED && 
             !_reconnectAttempted && 
             _config.autoConnect && 
             !_config.lastConnectedSsid.isEmpty()) {
             
             // Only try to reconnect if we have been connected before
             _reconnectAttempted = true;
             
             // Try to reconnect
             TDECK_LOG_I("Attempting to reconnect to: %s", _config.lastConnectedSsid.c_str());
             WiFi.begin();
             _connectionStartTime = currentTime;
         }
     }
     
     // Check if scan has completed
     if (_scanInProgress && WiFi.scanComplete() != WIFI_SCAN_RUNNING) {
         int numNetworks = WiFi.scanComplete();
         _scanInProgress = false;
         
         TDECK_LOG_I("WiFi scan completed, found %d networks", numNetworks);
         
         // Clear previous results
         _scanResults.clear();
         
         if (numNetworks > 0) {
             // Process scan results
             for (int i = 0; i < numNetworks; i++) {
                 WiFiNetwork network;
                 network.ssid = WiFi.SSID(i);
                 network.rssi = WiFi.RSSI(i);
                 network.encryptionType = WiFi.encryptionType(i);
                 network.bssid = WiFi.BSSIDstr(i);
                 network.channel = WiFi.channel(i);
                 
                 // Check if we're currently connected to this network
                 network.isConnected = (WiFi.status() == WL_CONNECTED && 
                                      WiFi.SSID() == network.ssid);
                 
                 _scanResults.push_back(network);
             }
             
             // Sort networks by signal strength (RSSI)
             std::sort(_scanResults.begin(), _scanResults.end(), 
                      [](const WiFiNetwork& a, const WiFiNetwork& b) {
                          return a.rssi > b.rssi;
                      });
         }
         
         // Delete scan results from memory
         WiFi.scanDelete();
     }
 }
 
 // Start scanning for WiFi networks
 bool WiFiManager::startScan() {
     // Can't start a new scan if one is already in progress
     if (_scanInProgress) {
         return false;
     }
     
     // Don't scan too frequently (limit to once every 10 seconds)
     if (millis() - _lastScanTime < 10000) {
         TDECK_LOG_W("WiFi scan requested too soon after previous scan");
         return false;
     }
     
     TDECK_LOG_I("Starting WiFi scan");
     
     // Start async scan (non-blocking)
     int result = WiFi.scanNetworks(true); // async=true
     
     if (result == WIFI_SCAN_FAILED) {
         TDECK_LOG_E("WiFi scan failed to start");
         return false;
     }
     
     _scanInProgress = true;
     _lastScanTime = millis();
     return true;
 }
 
 // Get results of the last WiFi scan
 std::vector<WiFiNetwork> WiFiManager::getScanResults() {
     return _scanResults;
 }
 
 // Check if WiFi scan is in progress
 bool WiFiManager::isScanInProgress() {
     return _scanInProgress;
 }
 
 // Connect to a WiFi network
 bool WiFiManager::connect(const String& ssid, const String& password, bool saveCredentials) {
     TDECK_LOG_I("Connecting to WiFi: %s", ssid.c_str());
     
     // Store the network in preferred list if requested
     if (saveCredentials) {
         addPreferredNetwork(ssid);
     }
     
     // Set WiFi mode to station
     WiFi.mode(WIFI_STA);
     
     // Disconnect from any current connection
     WiFi.disconnect();
     delay(100);
     
     // Begin connection with provided credentials
     WiFi.begin(ssid.c_str(), password.c_str());
     
     // Reset flags and timers
     _reconnectAttempted = false;
     _connectionStartTime = millis();
     
     return true;
 }
 
 // Disconnect from current WiFi network
 void WiFiManager::disconnect() {
     TDECK_LOG_I("Disconnecting from WiFi");
     WiFi.disconnect();
 }
 
 // Check if connected to a WiFi network
 bool WiFiManager::isConnected() {
     return WiFi.status() == WL_CONNECTED;
 }
 
 // Get current connection information
 bool WiFiManager::getConnectionInfo(WiFiNetwork* info) {
     if (!isConnected() || !info) {
         return false;
     }
     
     info->ssid = WiFi.SSID();
     info->rssi = WiFi.RSSI();
     info->bssid = WiFi.BSSIDstr();
     info->channel = WiFi.channel();
     info->isConnected = true;
     // Note: We can't determine encryption type of connected network
     info->encryptionType = 255; // Unknown/not applicable
     
     return true;
 }
 
 // Start WiFi Access Point mode
 bool WiFiManager::startAP(const char* customSsid, const char* customPassword) {
     String ssid = customSsid ? String(customSsid) : 
                   (_config.apSsid.isEmpty() ? String(TDECK_WIFI_AP_SSID) : _config.apSsid);
                   
     String password = customPassword ? String(customPassword) : 
                       (_config.apPassword.isEmpty() ? String(TDECK_WIFI_AP_PASSWORD) : _config.apPassword);
                       
     uint8_t channel = _config.apChannel > 0 ? _config.apChannel : TDECK_WIFI_AP_CHANNEL;
     bool hidden = _config.apHidden;
     uint8_t maxConnections = _config.apMaxConnections > 0 ? 
                             _config.apMaxConnections : TDECK_WIFI_AP_MAX_CONNECTIONS;
     
     TDECK_LOG_I("Starting WiFi AP: %s", ssid.c_str());
     
     // Set WiFi mode to AP or AP+STA if already connected as a station
     if (WiFi.status() == WL_CONNECTED) {
         WiFi.mode(WIFI_AP_STA);
     } else {
         WiFi.mode(WIFI_AP);
     }
     
     // Configure and start AP
     bool result = WiFi.softAP(ssid.c_str(), password.c_str(), channel, hidden, maxConnections);
     
     if (result) {
         TDECK_LOG_I("AP started with IP: %s", WiFi.softAPIP().toString().c_str());
     } else {
         TDECK_LOG_E("Failed to start AP");
     }
     
     return result;
 }
 
 // Stop WiFi Access Point mode
 void WiFiManager::stopAP() {
     TDECK_LOG_I("Stopping WiFi AP");
     
     // Disconnect any connected clients
     WiFi.softAPdisconnect(true);
     
     // If connected as a station, switch to STA mode, otherwise turn off WiFi
     if (WiFi.status() == WL_CONNECTED) {
         WiFi.mode(WIFI_STA);
     } else {
         WiFi.mode(WIFI_OFF);
     }
 }
 
 // Check if AP mode is active
 bool WiFiManager::isAPActive() {
     return WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
 }
 
 // Get number of stations connected to AP
 uint8_t WiFiManager::getAPStationCount() {
     if (!isAPActive()) {
         return 0;
     }
     return WiFi.softAPgetStationNum();
 }
 
 // Save WiFi configuration
 bool WiFiManager::saveConfig() {
     TDECK_LOG_I("Saving WiFi configuration");
     
     // Create a JSON document to store the configuration
     DynamicJsonDocument doc(2048); // Adjust size as needed
     
     doc["autoConnect"] = _config.autoConnect;
     doc["preferredNetworkOnly"] = _config.preferredNetworkOnly;
     doc["lastConnectedSsid"] = _config.lastConnectedSsid;
     doc["apSsid"] = _config.apSsid;
     doc["apPassword"] = _config.apPassword;
     doc["apChannel"] = _config.apChannel;
     doc["apHidden"] = _config.apHidden;
     doc["apMaxConnections"] = _config.apMaxConnections;
     
     // Store preferred networks as array
     JsonArray networks = doc.createNestedArray("preferredNetworks");
     for (const auto& network : _config.preferredNetworks) {
         networks.add(network);
     }
     
     // Save to file
     return fsManager.saveJsonToFile(TDECK_FS_WIFI_CONFIG_FILE, doc);
 }
 
 // Load WiFi configuration
 bool WiFiManager::loadConfig() {
     TDECK_LOG_I("Loading WiFi configuration");
     
     // Create a JSON document to load the configuration
     DynamicJsonDocument doc(2048); // Adjust size as needed
     
     // Check if config file exists and load it
     if (!fsManager.loadJsonFromFile(TDECK_FS_WIFI_CONFIG_FILE, doc)) {
         TDECK_LOG_W("WiFi config file not found or invalid JSON");
         return false;
     }
     
     // Extract configuration values
     _config.autoConnect = doc["autoConnect"] | true;
     _config.preferredNetworkOnly = doc["preferredNetworkOnly"] | false;
     _config.lastConnectedSsid = doc["lastConnectedSsid"] | "";
     _config.apSsid = doc["apSsid"] | "";
     _config.apPassword = doc["apPassword"] | "";
     _config.apChannel = doc["apChannel"] | TDECK_WIFI_AP_CHANNEL;
     _config.apHidden = doc["apHidden"] | TDECK_WIFI_AP_HIDE_SSID;
     _config.apMaxConnections = doc["apMaxConnections"] | TDECK_WIFI_AP_MAX_CONNECTIONS;
     
     // Clear and load preferred networks
     _config.preferredNetworks.clear();
     if (doc.containsKey("preferredNetworks") && doc["preferredNetworks"].is<JsonArray>()) {
         for (JsonVariant network : doc["preferredNetworks"].as<JsonArray>()) {
             _config.preferredNetworks.push_back(network.as<String>());
         }
     }
     
     return true;
 }
 
 // Get local IP address
 String WiFiManager::getLocalIP() {
     // Return AP IP if in AP mode, otherwise return station IP
     if (isAPActive() && !isConnected()) {
         return WiFi.softAPIP().toString();
     } else if (isConnected()) {
         return WiFi.localIP().toString();
     } else {
         return "0.0.0.0";
     }
 }
 
 // Get WiFi signal strength
 int32_t WiFiManager::getRSSI() {
     if (!isConnected()) {
         return 0;
     }
     return WiFi.RSSI();
 }
 
 // Get current WiFi status
 wl_status_t WiFiManager::getStatus() {
     return WiFi.status();
 }
 
 // Get WiFi MAC address
 String WiFiManager::getMacAddress() {
     return WiFi.macAddress();
 }
 
 // Add network to preferred networks list
 bool WiFiManager::addPreferredNetwork(const String& ssid) {
     // Check if already in the list
     for (const auto& network : _config.preferredNetworks) {
         if (network == ssid) {
             return true; // Already in list
         }
     }
     
     // Add to list
     _config.preferredNetworks.push_back(ssid);
     
     // Save configuration
     return saveConfig();
 }
 
 // Remove network from preferred networks list
 bool WiFiManager::removePreferredNetwork(const String& ssid) {
     bool found = false;
     
     // Find and remove from list
     for (auto it = _config.preferredNetworks.begin(); it != _config.preferredNetworks.end(); ) {
         if (*it == ssid) {
             it = _config.preferredNetworks.erase(it);
             found = true;
         } else {
             ++it;
         }
     }
     
     // If found and removed, save configuration
     if (found) {
         return saveConfig();
     }
     
     return false;
 }
 
 // Get current WiFi configuration
 const WiFiConfig& WiFiManager::getConfig() const {
     return _config;
 }
 
 // Convert encryption type to string
 String WiFiManager::encryptionTypeToString(uint8_t encType) {
     switch (encType) {
         case WIFI_AUTH_OPEN:
             return "Open";
         case WIFI_AUTH_WEP:
             return "WEP";
         case WIFI_AUTH_WPA_PSK:
             return "WPA-PSK";
         case WIFI_AUTH_WPA2_PSK:
             return "WPA2-PSK";
         case WIFI_AUTH_WPA_WPA2_PSK:
             return "WPA/WPA2-PSK";
         case WIFI_AUTH_WPA2_ENTERPRISE:
             return "WPA2-Enterprise";
         default:
             return "Unknown";
     }
 }
 
 // Create default configuration
 void WiFiManager::createDefaultConfig() {
     _config.autoConnect = true;
     _config.preferredNetworkOnly = false;
     _config.preferredNetworks.clear();
     _config.lastConnectedSsid = "";
     _config.apSsid = TDECK_WIFI_AP_SSID;
     _config.apPassword = TDECK_WIFI_AP_PASSWORD;
     _config.apChannel = TDECK_WIFI_AP_CHANNEL;
     _config.apHidden = TDECK_WIFI_AP_HIDE_SSID;
     _config.apMaxConnections = TDECK_WIFI_AP_MAX_CONNECTIONS;
 }
 
 // Handle connection timeout
 void WiFiManager::handleConnectTimeout() {
     TDECK_LOG_W("WiFi connection attempt timed out");
     
     // Reset connection timer
     _connectionStartTime = 0;
     
     // Disconnect from current attempt
     WiFi.disconnect();
     
     // Try next preferred network if available
     if (!autoConnectToPreferred()) {
         TDECK_LOG_I("No alternative networks available");
     }
 }
 
 // Automatically connect to preferred networks
 bool WiFiManager::autoConnectToPreferred() {
     // Only proceed if we have preferred networks and auto-connect is enabled
     if (_config.preferredNetworks.empty() || !_config.autoConnect) {
         return false;
     }
     
     TDECK_LOG_I("Trying to connect to preferred networks");
     
     // Need to scan first to find available networks
     if (_scanResults.empty() && !_scanInProgress) {
         startScan();
         return false; // Wait for scan to complete
     }
     
     // If scan is in progress, wait for it to complete
     if (_scanInProgress) {
         return false;
     }
     
     // Try to find a preferred network in scan results
     for (const auto& result : _scanResults) {
         for (const auto& preferred : _config.preferredNetworks) {
             if (result.ssid == preferred) {
                 // Found a match, try to connect
                 // This requires the password to be saved in the ESP32's NVS
                 TDECK_LOG_I("Attempting to connect to preferred network: %s", preferred.c_str());
                 WiFi.begin(preferred.c_str());
                 _connectionStartTime = millis();
                 return true;
             }
         }
     }
     
     return false; // No suitable network found
 }