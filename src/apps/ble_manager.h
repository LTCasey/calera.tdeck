/**
 * @file ble_manager.h
 * @brief Bluetooth management application for T-Deck UI
 * 
 * This application provides a UI for scanning, connecting to,
 * and managing Bluetooth devices on the T-Deck.
 */

 #ifndef TDECK_BLE_MANAGER_H
 #define TDECK_BLE_MANAGER_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include <vector>
 #include "config.h"
 #include "comms/bluetooth.h"
 #include "ui/ui_manager.h"
 
 /**
  * @struct BLEDeviceInfo
  * @brief Stores information about a discovered BLE device
  */
 struct BLEDeviceInfo {
     String address;      ///< MAC address of the device
     String name;         ///< Name of the device (if available)
     int rssi;            ///< Signal strength
     bool isPaired;       ///< Whether the device is paired
     bool isConnected;    ///< Whether the device is connected
 };
 
 /**
  * @class BLEManagerApp
  * @brief Application for managing Bluetooth connections
  */
 class BLEManagerApp {
 public:
     /**
      * @brief Initialize the BLE Manager application
      * @param parent Parent container for the application UI
      * @return true if initialization succeeded, false otherwise
      */
     bool init(lv_obj_t* parent);
 
     /**
      * @brief Start the BLE Manager application
      * @return true if the application started successfully, false otherwise
      */
     bool start();
 
     /**
      * @brief Stop the BLE Manager application
      */
     void stop();
 
     /**
      * @brief Check if the application is currently running
      * @return true if running, false otherwise
      */
     bool isRunning() const;
 
 private:
     // UI Objects
     lv_obj_t* _parentContainer = nullptr;  ///< Parent container
     lv_obj_t* _appContainer = nullptr;     ///< Main app container
     lv_obj_t* _tabView = nullptr;          ///< Tab view for different BLE functions
     lv_obj_t* _devicesTab = nullptr;       ///< Tab for scanning/connecting to devices
     lv_obj_t* _statusTab = nullptr;        ///< Tab for BLE status
     lv_obj_t* _settingsTab = nullptr;      ///< Tab for BLE settings
     lv_obj_t* _deviceList = nullptr;       ///< List of available devices
     lv_obj_t* _scanBtn = nullptr;          ///< Scan button
     lv_obj_t* _stopScanBtn = nullptr;      ///< Stop scan button
     lv_obj_t* _messageLabel = nullptr;     ///< Status message label
     lv_obj_t* _btEnableSwitch = nullptr;   ///< Switch for enabling Bluetooth
     lv_obj_t* _btVisibilitySwitch = nullptr; ///< Switch for BT visibility
     lv_obj_t* _btNameTextArea = nullptr;   ///< Text area for device name
     lv_obj_t* _saveSettingsBtn = nullptr;  ///< Button to save settings
     lv_obj_t* _keyboard = nullptr;         ///< Keyboard for input
 
     // State variables
     bool _isRunning = false;               ///< Application running state
     bool _isScanning = false;              ///< BLE scanning state
     String _selectedDeviceAddress;         ///< Currently selected device address
     int _selectedDeviceIndex = -1;         ///< Index of selected device
     std::vector<BLEDeviceInfo> _deviceList_data; ///< List of discovered devices
     
     // Bluetooth Manager reference
     BluetoothManager& _btManager = BluetoothManager::getInstance();
     
     /**
      * @brief Create the UI components for the application
      */
     void createUI();
     
     /**
      * @brief Start a BLE scan operation
      */
     void startScan();
     
     /**
      * @brief Stop the current BLE scan
      */
     void stopScan();
     
     /**
      * @brief Update the device list with scan results
      */
     void updateDeviceList();
     
     /**
      * @brief Update the BLE status display
      */
     void updateBLEStatus();
     
     /**
      * @brief Connect to the selected BLE device
      * @param deviceIndex Index of the device in the list
      */
     void connectToDevice(int deviceIndex);
     
     /**
      * @brief Disconnect from the current BLE device
      */
     void disconnectFromDevice();
     
     /**
      * @brief Pair with the selected BLE device
      * @param deviceIndex Index of the device in the list
      */
     void pairWithDevice(int deviceIndex);
     
     /**
      * @brief Unpair from a BLE device
      * @param deviceIndex Index of the device in the list
      */
     void unpairFromDevice(int deviceIndex);
     
     /**
      * @brief Enable or disable Bluetooth
      * @param enable true to enable, false to disable
      */
     void enableBluetooth(bool enable);
     
     /**
      * @brief Set device visibility
      * @param visible true to make device visible, false to hide
      */
     void setVisibility(bool visible);
     
     /**
      * @brief Save the BLE settings
      */
     void saveSettings();
     
     /**
      * @brief Load BLE settings from storage
      */
     void loadSettings();
     
     // LVGL callback functions
     static void onScanBtnClicked(lv_obj_t* obj, lv_event_t event);
     static void onStopScanBtnClicked(lv_obj_t* obj, lv_event_t event);
     static void onDeviceItemClicked(lv_obj_t* obj, lv_event_t event);
     static void onBTEnableSwitchChanged(lv_obj_t* obj, lv_event_t event);
     static void onBTVisibilitySwitchChanged(lv_obj_t* obj, lv_event_t event);
     static void onSaveSettingsBtnClicked(lv_obj_t* obj, lv_event_t event);
     static void onKeyboardEvent(lv_obj_t* obj, lv_event_t event);
     static void onStatusUpdateTimer(lv_task_t* task);
     static void onDeviceContextMenu(lv_obj_t* obj, lv_event_t event);
 };
 
 #endif // TDECK_BLE_MANAGER_H