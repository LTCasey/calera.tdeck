/**
 * @file bluetooth.h
 * @brief Bluetooth management for T-Deck UI Firmware
 * 
 * This file contains the Bluetooth manager class which handles
 * Bluetooth connectivity, pairing, device discovery, and
 * managing Bluetooth configuration.
 */

 #ifndef TDECK_BLUETOOTH_H
 #define TDECK_BLUETOOTH_H
 
 #include <Arduino.h>
 #include <BluetoothSerial.h>
 #include <BLEDevice.h>
 #include <BLEServer.h>
 #include <BLEClient.h>
 #include <BLEScan.h>
 #include <BLEAdvertisedDevice.h>
 #include <vector>
 #include <map>
 #include <ArduinoJson.h>
 #include "../config.h"
 #include "../system/fs_manager.h"
 
 // External declaration of file system manager
 extern FSManager fsManager;
 
 // Check if Bluetooth is available
 #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
 #error Bluetooth is not enabled! Please run `make menuconfig` and enable it
 #endif
 
 /**
  * @brief Structure to hold Bluetooth device information
  */
 struct BluetoothDevice {
     String address;        // MAC address
     String name;           // Device name
     int rssi;              // Signal strength
     bool paired;           // Whether the device is paired
     bool connected;        // Whether the device is connected
     std::map<String, String> services; // Service UUIDs and descriptions
 };
 
 /**
  * @brief Structure to hold Bluetooth configuration
  */
 struct BluetoothConfig {
     String deviceName;             // Custom device name
     bool autoConnect;              // Auto-connect to last device on startup
     bool discoverableOnStartup;    // Make device discoverable on startup
     String lastConnectedDevice;    // Last successfully connected device address
     std::vector<String> pairedDevices; // List of paired device addresses
     bool classicEnabled;           // Enable Classic Bluetooth (Serial)
     bool bleEnabled;               // Enable Bluetooth Low Energy
 };
 
 // Callback class for BLE scan results
 class BLEScanCallbacks : public BLEAdvertisedDeviceCallbacks {
 public:
     BLEScanCallbacks(std::vector<BluetoothDevice>& scanResults);
     void onResult(BLEAdvertisedDevice advertisedDevice) override;
     
 private:
     std::vector<BluetoothDevice>& _scanResults;
 };
 
 /**
  * @brief Manages Bluetooth connectivity and configuration
  */
 class BluetoothManager {
 public:
     /**
      * @brief Construct a new Bluetooth Manager object
      */
     BluetoothManager();
 
     /**
      * @brief Destroy the Bluetooth Manager object
      */
     ~BluetoothManager();
 
     /**
      * @brief Initialize Bluetooth functionality
      * 
      * @return true if initialization successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Update Bluetooth status (called periodically)
      */
     void update();
 
     /**
      * @brief Start scanning for Bluetooth devices
      * 
      * @param durationSeconds Duration of scan in seconds
      * @return true if scan started successfully
      * @return false if scan failed to start
      */
     bool startScan(int durationSeconds = 5);
 
     /**
      * @brief Get results of the last Bluetooth scan
      * 
      * @return std::vector<BluetoothDevice> List of found Bluetooth devices
      */
     std::vector<BluetoothDevice> getScanResults();
 
     /**
      * @brief Check if Bluetooth scan is in progress
      * 
      * @return true if scan is in progress
      * @return false if no scan is in progress
      */
     bool isScanInProgress();
 
     /**
      * @brief Connect to a Bluetooth device
      * 
      * @param address Device MAC address
      * @param savePairing Whether to save pairing for future reconnection
      * @return true if connection attempt started
      * @return false if connection attempt failed to start
      */
     bool connect(const String& address, bool savePairing = true);
 
     /**
      * @brief Disconnect from the current Bluetooth device
      */
     void disconnect();
 
     /**
      * @brief Get the current connection status
      * 
      * @return true if connected to a Bluetooth device
      * @return false if not connected
      */
     bool isConnected();
 
     /**
      * @brief Get information about currently connected device
      * 
      * @param device Pointer to BluetoothDevice struct to fill
      * @return true if connected and info was filled
      * @return false if not connected
      */
     bool getConnectedDeviceInfo(BluetoothDevice* device);
 
     /**
      * @brief Make the device discoverable
      * 
      * @param discoverable Whether to make discoverable or not
      * @param durationSeconds Duration in seconds (0 for indefinite)
      * @return true if operation successful
      * @return false if operation failed
      */
     bool setDiscoverable(bool discoverable, int durationSeconds = 0);
 
     /**
      * @brief Check if the device is currently discoverable
      * 
      * @return true if device is discoverable
      * @return false if device is not discoverable
      */
     bool isDiscoverable();
 
     /**
      * @brief Set Bluetooth device name
      * 
      * @param name New device name
      * @return true if name was set successfully
      * @return false if name could not be set
      */
     bool setDeviceName(const String& name);
 
     /**
      * @brief Get Bluetooth device name
      * 
      * @return String Current device name
      */
     String getDeviceName();
 
     /**
      * @brief Save Bluetooth configuration
      * 
      * @return true if configuration saved successfully
      * @return false if configuration save failed
      */
     bool saveConfig();
 
     /**
      * @brief Load Bluetooth configuration
      * 
      * @return true if configuration loaded successfully
      * @return false if configuration load failed
      */
     bool loadConfig();
 
     /**
      * @brief Send data via Bluetooth Serial (Classic)
      * 
      * @param data Data to send
      * @return size_t Number of bytes sent
      */
     size_t sendSerialData(const String& data);
 
     /**
      * @brief Check if data is available to read from Bluetooth Serial
      * 
      * @return int Number of bytes available
      */
     int serialDataAvailable();
 
     /**
      * @brief Read data from Bluetooth Serial
      * 
      * @param buffer Buffer to store data
      * @param maxLen Maximum length to read
      * @return int Number of bytes read
      */
     int readSerialData(char* buffer, size_t maxLen);
 
     /**
      * @brief Read a line from Bluetooth Serial
      * 
      * @return String Line read from serial
      */
     String readSerialLine();
 
     /**
      * @brief Get device RSSI (signal strength)
      * 
      * @return int RSSI value or 0 if not connected
      */
     int getRSSI();
 
     /**
      * @brief Get the current Bluetooth configuration
      * 
      * @return const BluetoothConfig& Reference to current configuration
      */
     const BluetoothConfig& getConfig() const;
 
     /**
      * @brief Add a device to paired devices list
      * 
      * @param address Device MAC address
      * @return true if added successfully
      * @return false if already in list or failed to save
      */
     bool addPairedDevice(const String& address);
 
     /**
      * @brief Remove a device from paired devices list
      * 
      * @param address Device MAC address
      * @return true if removed successfully
      * @return false if not in list or failed to save
      */
     bool removePairedDevice(const String& address);
 
 private:
     BluetoothConfig _config;                   // Bluetooth configuration
     std::vector<BluetoothDevice> _scanResults; // Results of the last scan
     bool _scanInProgress;                      // Whether a scan is currently in progress
     unsigned long _lastScanTime;               // Timestamp of the last scan
     unsigned long _lastStatusCheck;            // Timestamp of the last status check
     bool _isConnected;                         // Current connection status
     String _connectedDeviceAddress;            // Address of connected device
     bool _isDiscoverable;                      // Whether device is discoverable
     unsigned long _discoverableEndTime;        // When discoverability ends
     
     // Bluetooth components
     BluetoothSerial* _serialBT;                // Serial Bluetooth instance
     BLEScan* _bleScan;                         // BLE scanner
     BLEServer* _bleServer;                     // BLE server
     BLEClient* _bleClient;                     // BLE client
     BLEScanCallbacks* _scanCallbacks;          // BLE scan callbacks
     
     /**
      * @brief Create default configuration
      */
     void createDefaultConfig();
     
     /**
      * @brief Initialize Classic Bluetooth (Serial)
      */
     bool initClassicBT();
     
     /**
      * @brief Initialize BLE
      */
     bool initBLE();
     
     /**
      * @brief Automatically connect to last paired device
      */
     bool autoConnectToLastDevice();
 };
 
 #endif // TDECK_BLUETOOTH_H