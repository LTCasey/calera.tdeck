/**
 * @file bluetooth.cpp
 * @brief Bluetooth management implementation for T-Deck UI Firmware
 */

 #include "bluetooth.h"

 // Implementation of the BLE scan callbacks
 BLEScanCallbacks::BLEScanCallbacks(std::vector<BluetoothDevice>& scanResults)
     : _scanResults(scanResults)
 {
 }
 
 void BLEScanCallbacks::onResult(BLEAdvertisedDevice advertisedDevice) {
     BluetoothDevice device;
     
     // Fill device information
     device.address = advertisedDevice.getAddress().toString().c_str();
     
     if (advertisedDevice.haveName()) {
         device.name = advertisedDevice.getName().c_str();
     } else {
         device.name = "Unknown";
     }
     
     device.rssi = advertisedDevice.getRSSI();
     device.paired = false;  // Will be updated later if device is paired
     device.connected = false; // Will be updated later if device is connected
     
     // Add services if advertised
     if (advertisedDevice.haveServiceUUID()) {
         String uuid = advertisedDevice.getServiceUUID().toString().c_str();
         device.services[uuid] = "Unknown Service"; // Default description
         
         // Add known service descriptions (could be expanded)
         if (uuid == "1800") device.services[uuid] = "Generic Access";
         if (uuid == "1801") device.services[uuid] = "Generic Attribute";
         if (uuid == "180F") device.services[uuid] = "Battery Service";
         if (uuid == "180A") device.services[uuid] = "Device Information";
         if (uuid == "1812") device.services[uuid] = "HID";
     }
     
     // Check if device already exists in results (by address)
     bool exists = false;
     for (auto& existingDevice : _scanResults) {
         if (existingDevice.address == device.address) {
             // Update existing device info
             existingDevice = device;
             exists = true;
             break;
         }
     }
     
     // Add new device if not found
     if (!exists) {
         _scanResults.push_back(device);
     }
     
     // Log discovery
     TDECK_LOG_I("BLE device found: %s (%s), RSSI: %d", 
                 device.name.c_str(), 
                 device.address.c_str(), 
                 device.rssi);
 }
 
 // Constructor
 BluetoothManager::BluetoothManager()
     : _scanInProgress(false)
     , _lastScanTime(0)
     , _lastStatusCheck(0)
     , _isConnected(false)
     , _connectedDeviceAddress("")
     , _isDiscoverable(false)
     , _discoverableEndTime(0)
     , _serialBT(nullptr)
     , _bleScan(nullptr)
     , _bleServer(nullptr)
     , _bleClient(nullptr)
     , _scanCallbacks(nullptr)
 {
     // Initialize with default values
     createDefaultConfig();
 }
 
 // Destructor
 BluetoothManager::~BluetoothManager() {
     // Clean up resources
     if (_serialBT) {
         _serialBT->end();
         delete _serialBT;
     }
     
     if (_scanCallbacks) {
         delete _scanCallbacks;
     }
     
     // BLE resources are managed by the BLE library
 }
 
 // Initialize Bluetooth functionality
 bool BluetoothManager::init() {
     TDECK_LOG_I("Initializing Bluetooth manager");
     
     // Load configuration from file
     if (!loadConfig()) {
         TDECK_LOG_W("Failed to load Bluetooth config, using defaults");
         createDefaultConfig();
         saveConfig(); // Create default config file
     }
     
     bool success = true;
     
     // Initialize Classic Bluetooth if enabled
     if (_config.classicEnabled) {
         success &= initClassicBT();
     }
     
     // Initialize BLE if enabled
     if (_config.bleEnabled) {
         success &= initBLE();
     }
     
     // Make discoverable if configured
     if (_config.discoverableOnStartup) {
         setDiscoverable(true);
     }
     
     // Auto-connect if enabled and we have a last connected device
     if (_config.autoConnect && !_config.lastConnectedDevice.isEmpty()) {
         autoConnectToLastDevice();
     }
     
     return success;
 }
 
 // Update Bluetooth status (called periodically)
 void BluetoothManager::update() {
     unsigned long currentTime = millis();
     
     // Check connection status periodically (every 1 second)
     if (currentTime - _lastStatusCheck >= 1000) {
         _lastStatusCheck = currentTime;
         
         // Check if discoverable timeout has expired
         if (_isDiscoverable && _discoverableEndTime > 0 && currentTime > _discoverableEndTime) {
             TDECK_LOG_I("Discoverable timeout expired");
             setDiscoverable(false);
         }
         
         // Classic BT connection status (if enabled)
         if (_config.classicEnabled && _serialBT) {
             bool isConnected = _serialBT->connected();
             
             // Connection status changed
             if (isConnected != _isConnected) {
                 _isConnected = isConnected;
                 
                 if (isConnected) {
                     TDECK_LOG_I("Bluetooth Classic connected");
                     
                     // Update last connected device if not already set
                     // Note: Serial BT doesn't provide the connected device address
                     // so we can't reliably track the connected device
                 } else {
                     TDECK_LOG_I("Bluetooth Classic disconnected");
                 }
             }
         }
         
         // BLE connection status monitoring could be added here
         // This is more complex as it depends on connection callbacks
         // from the BLE library
     }
     
     // Check if scan has completed (for BLE)
     if (_scanInProgress && _bleScan && !_bleScan->isScanning()) {
         _scanInProgress = false;
         TDECK_LOG_I("BLE scan completed, found %d devices", _scanResults.size());
     }
 }
 
 // Start scanning for Bluetooth devices
 bool BluetoothManager::startScan(int durationSeconds) {
     // Can't start a new scan if one is already in progress
     if (_scanInProgress) {
         return false;
     }
     
     // Don't scan too frequently (limit to once every 10 seconds)
     if (millis() - _lastScanTime < 10000) {
         TDECK_LOG_W("Bluetooth scan requested too soon after previous scan");
         return false;
     }
     
     TDECK_LOG_I("Starting Bluetooth scan");
     
     // Clear previous results
     _scanResults.clear();
     
     // Start BLE scan if enabled
     if (_config.bleEnabled && _bleScan) {
         _bleScan->setActiveScan(true);
         _bleScan->start(durationSeconds, false); // Non-blocking scan
         _scanInProgress = true;
         _lastScanTime = millis();
         return true;
     }
     
     // Classic Bluetooth scanning not supported by BluetoothSerial
     TDECK_LOG_W("BLE not enabled, cannot perform scan");
     return false;
 }
 
 // Get results of the last Bluetooth scan
 std::vector<BluetoothDevice> BluetoothManager::getScanResults() {
     // Mark paired devices in scan results
     for (auto& device : _scanResults) {
         // Check if device is in paired list
         for (const auto& pairedAddress : _config.pairedDevices) {
             if (device.address == pairedAddress) {
                 device.paired = true;
                 break;
             }
         }
         
         // Check if device is currently connected
         device.connected = (_isConnected && device.address == _connectedDeviceAddress);
     }
     
     return _scanResults;
 }
 
 // Check if Bluetooth scan is in progress
 bool BluetoothManager::isScanInProgress() {
     return _scanInProgress;
 }
 
 // Connect to a Bluetooth device
 bool BluetoothManager::connect(const String& address, bool savePairing) {
     TDECK_LOG_I("Connecting to Bluetooth device: %s", address.c_str());
     
     // Store the device in paired list if requested
     if (savePairing) {
         addPairedDevice(address);
     }
     
     // For Classic Bluetooth, we can't connect to a specific device
     // The connection must be initiated from the remote device
     if (_config.classicEnabled && _serialBT) {
         TDECK_LOG_I("Classic Bluetooth connection must be initiated from remote device");
     }
     
     // For BLE, we can initiate a connection
     if (_config.bleEnabled && _bleClient) {
         // Implement BLE connection logic here
         // This is more complex and requires setting up connection callbacks
         // and service discovery
         
         // Basic implementation:
         TDECK_LOG_I("BLE connection not fully implemented");
         
         // Set connected device for tracking
         _connectedDeviceAddress = address;
         _config.lastConnectedDevice = address;
         saveConfig();
         
         return true;
     }
     
     return false;
 }
 
 // Disconnect from the current Bluetooth device
 void BluetoothManager::disconnect() {
     TDECK_LOG_I("Disconnecting Bluetooth");
     
     if (_config.classicEnabled && _serialBT && _isConnected) {
         // For Classic Bluetooth, we don't have a direct disconnect method
         // The simplest approach is to restart the Bluetooth service
         _serialBT->end();
         delay(100);
         _serialBT->begin(_config.deviceName);
     }
     
     if (_config.bleEnabled && _bleClient && _isConnected) {
         // Implement BLE disconnect logic here
         _bleClient->disconnect();
     }
     
     _isConnected = false;
     _connectedDeviceAddress = "";
 }
 
 // Check if connected to a Bluetooth device
 bool BluetoothManager::isConnected() {
     return _isConnected;
 }
 
 // Get information about currently connected device
 bool BluetoothManager::getConnectedDeviceInfo(BluetoothDevice* device) {
     if (!_isConnected || !device) {
         return false;
     }
     
     // Try to find the device in scan results first
     for (const auto& scannedDevice : _scanResults) {
         if (scannedDevice.address == _connectedDeviceAddress) {
             *device = scannedDevice;
             device->connected = true;
             return true;
         }
     }
     
     // If not found in scan results, create basic device info
     device->address = _connectedDeviceAddress;
     device->name = "Connected Device"; // We don't know the name
     device->rssi = getRSSI();
     device->paired = true; // Assume it's paired if connected
     device->connected = true;
     
     return true;
 }
 
 // Make the device discoverable
 bool BluetoothManager::setDiscoverable(bool discoverable, int durationSeconds) {
     _isDiscoverable = discoverable;
     
     if (discoverable) {
         TDECK_LOG_I("Setting Bluetooth to discoverable mode");
         
         // Set timeout for discoverability
         if (durationSeconds > 0) {
             _discoverableEndTime = millis() + (durationSeconds * 1000);
         } else {
             _discoverableEndTime = 0; // No timeout
         }
         
         // For Classic Bluetooth, discoverability is always on when the service is started
         
         // For BLE, we need to start advertising
         if (_config.bleEnabled && _bleServer) {
             // Implement BLE advertising configuration here
             // This would involve setting up advertising data and starting advertising
             
             TDECK_LOG_I("BLE advertising not fully implemented");
         }
     } else {
         TDECK_LOG_I("Setting Bluetooth to non-discoverable mode");
         
         // For BLE, stop advertising
         if (_config.bleEnabled && _bleServer) {
             // Stop BLE advertising
             // BLEDevice::stopAdvertising();
         }
     }
     
     return true;
 }
 
 // Check if the device is currently discoverable
 bool BluetoothManager::isDiscoverable() {
     return _isDiscoverable;
 }
 
 // Set Bluetooth device name
 bool BluetoothManager::setDeviceName(const String& name) {
     TDECK_LOG_I("Setting Bluetooth device name: %s", name.c_str());
     
     _config.deviceName = name;
     
     // For Classic Bluetooth, we need to restart the service
     if (_config.classicEnabled && _serialBT) {
         _serialBT->end();
         delay(100);
         _serialBT->begin(name);
     }
     
     // For BLE, we need to set the device name
     if (_config.bleEnabled) {
         BLEDevice::setDeviceName(name.c_str());
     }
     
     // Save the configuration
     return saveConfig();
 }
 
 // Get Bluetooth device name
 String BluetoothManager::getDeviceName() {
     return _config.deviceName;
 }
 
 // Save Bluetooth configuration
 bool BluetoothManager::saveConfig() {
     TDECK_LOG_I("Saving Bluetooth configuration");
     
     // Create a JSON document to store the configuration
     DynamicJsonDocument doc(2048); // Adjust size as needed
     
     doc["deviceName"] = _config.deviceName;
     doc["autoConnect"] = _config.autoConnect;
     doc["discoverableOnStartup"] = _config.discoverableOnStartup;
     doc["lastConnectedDevice"] = _config.lastConnectedDevice;
     doc["classicEnabled"] = _config.classicEnabled;
     doc["bleEnabled"] = _config.bleEnabled;
     
     // Store paired devices as array
     JsonArray devices = doc.createNestedArray("pairedDevices");
     for (const auto& device : _config.pairedDevices) {
         devices.add(device);
     }
     
     // Save to file
     return fsManager.saveJsonToFile(TDECK_FS_BT_CONFIG_FILE, doc);
 }
 
 // Load Bluetooth configuration
 bool BluetoothManager::loadConfig() {
     TDECK_LOG_I("Loading Bluetooth configuration");
     
     // Create a JSON document to load the configuration
     DynamicJsonDocument doc(2048); // Adjust size as needed
     
     // Check if config file exists and load it
     if (!fsManager.loadJsonFromFile(TDECK_FS_BT_CONFIG_FILE, doc)) {
         TDECK_LOG_W("Bluetooth config file not found or invalid JSON");
         return false;
     }
     
     // Extract configuration values
     _config.deviceName = doc["deviceName"] | TDECK_BT_DEVICE_NAME;
     _config.autoConnect = doc["autoConnect"] | true;
     _config.discoverableOnStartup = doc["discoverableOnStartup"] | false;
     _config.lastConnectedDevice = doc["lastConnectedDevice"] | "";
     _config.classicEnabled = doc["classicEnabled"] | true;
     _config.bleEnabled = doc["bleEnabled"] | true;
     
     // Clear and load paired devices
     _config.pairedDevices.clear();
     if (doc.containsKey("pairedDevices") && doc["pairedDevices"].is<JsonArray>()) {
         for (JsonVariant device : doc["pairedDevices"].as<JsonArray>()) {
             _config.pairedDevices.push_back(device.as<String>());
         }
     }
     
     return true;
 }
 
 // Send data via Bluetooth Serial (Classic)
 size_t BluetoothManager::sendSerialData(const String& data) {
     if (!_config.classicEnabled || !_serialBT || !_isConnected) {
         return 0;
     }
     
     return _serialBT->print(data);
 }
 
 // Check if data is available to read from Bluetooth Serial
 int BluetoothManager::serialDataAvailable() {
     if (!_config.classicEnabled || !_serialBT) {
         return 0;
     }
     
     return _serialBT->available();
 }
 
 // Read data from Bluetooth Serial
 int BluetoothManager::readSerialData(char* buffer, size_t maxLen) {
     if (!_config.classicEnabled || !_serialBT || maxLen == 0) {
         return 0;
     }
     
     return _serialBT->readBytes(buffer, maxLen);
 }
 
 // Read a line from Bluetooth Serial
 String BluetoothManager::readSerialLine() {
     if (!_config.classicEnabled || !_serialBT) {
         return "";
     }
     
     return _serialBT->readStringUntil('\n');
 }
 
 // Get device RSSI (signal strength)
 int BluetoothManager::getRSSI() {
     // For Classic Bluetooth, we don't have a direct RSSI method
     
     // For BLE, we could implement RSSI reading
     if (_config.bleEnabled && _bleClient && _isConnected) {
         // Implement BLE RSSI reading here
         return -70; // Placeholder value
     }
     
     return 0;
 }
 
 // Get the current Bluetooth configuration
 const BluetoothConfig& BluetoothManager::getConfig() const {
     return _config;
 }
 
 // Add a device to paired devices list
 bool BluetoothManager::addPairedDevice(const String& address) {
     // Check if already in the list
     for (const auto& device : _config.pairedDevices) {
         if (device == address) {
             return true; // Already in list
         }
     }
     
     // Add to list
     _config.pairedDevices.push_back(address);
     
     // Save configuration
     return saveConfig();
 }
 
 // Remove a device from paired devices list
 bool BluetoothManager::removePairedDevice(const String& address) {
     bool found = false;
     
     // Find and remove from list
     for (auto it = _config.pairedDevices.begin(); it != _config.pairedDevices.end(); ) {
         if (*it == address) {
             it = _config.pairedDevices.erase(it);
             found = true;
         } else {
             ++it;
         }
     }
     
     // If found and removed, save configuration
     if (found) {
         // If this was the last connected device, clear that too
         if (_config.lastConnectedDevice == address) {
             _config.lastConnectedDevice = "";
         }
         
         return saveConfig();
     }
     
     return false;
 }
 
 // Create default configuration
 void BluetoothManager::createDefaultConfig() {
     _config.deviceName = TDECK_BT_DEVICE_NAME;
     _config.autoConnect = true;
     _config.discoverableOnStartup = false;
     _config.lastConnectedDevice = "";
     _config.pairedDevices.clear();
     _config.classicEnabled = true;
     _config.bleEnabled = true;
 }
 
 // Initialize Classic Bluetooth (Serial)
 bool BluetoothManager::initClassicBT() {
     TDECK_LOG_I("Initializing Classic Bluetooth");
     
     // Create Bluetooth Serial instance
     _serialBT = new BluetoothSerial();
     
     if (!_serialBT) {
         TDECK_LOG_E("Failed to create Bluetooth Serial instance");
         return false;
     }
     
     // Begin Bluetooth Serial with device name
     if (!_serialBT->begin(_config.deviceName)) {
         TDECK_LOG_E("Failed to start Bluetooth Serial");
         delete _serialBT;
         _serialBT = nullptr;
         return false;
     }
     
     TDECK_LOG_I("Classic Bluetooth initialized successfully");
     return true;
 }
 
 // Initialize BLE
 bool BluetoothManager::initBLE() {
     TDECK_LOG_I("Initializing BLE");
     
     // Initialize BLE device
     BLEDevice::init(_config.deviceName.c_str());
     
     // Create scan object
     _bleScan = BLEDevice::getScan();
     
     if (!_bleScan) {
         TDECK_LOG_E("Failed to create BLE scan object");
         return false;
     }
     
     // Set up scan callbacks
     _scanCallbacks = new BLEScanCallbacks(_scanResults);
     _bleScan->setAdvertisedDeviceCallbacks(_scanCallbacks);
     
     // Create server for advertising and connections
     _bleServer = BLEDevice::createServer();
     
     if (!_bleServer) {
         TDECK_LOG_E("Failed to create BLE server");
         return false;
     }
     
     // Create client for outgoing connections
     _bleClient = BLEDevice::createClient();
     
     if (!_bleClient) {
         TDECK_LOG_E("Failed to create BLE client");
         return false;
     }
     
     // Set up server callbacks for connection events
     // This would require implementing a server callbacks class
     
     // Set up client callbacks for connection events
     // This would require implementing a client callbacks class
     
     TDECK_LOG_I("BLE initialized successfully");
     return true;
 }
 
 // Automatically connect to last paired device
 bool BluetoothManager::autoConnectToLastDevice() {
     if (_config.lastConnectedDevice.isEmpty()) {
         return false;
     }
     
     TDECK_LOG_I("Auto-connecting to last device: %s", _config.lastConnectedDevice.c_str());
     
     // Attempt to connect using the address
     return connect(_config.lastConnectedDevice, false);
 }