/**
 * @file wifi_manager.h
 * @brief WiFi management application for T-Deck UI
 * 
 * This application provides a UI for scanning, connecting to,
 * and managing WiFi networks on the T-Deck device.
 */

 #ifndef TDECK_WIFI_MANAGER_H
 #define TDECK_WIFI_MANAGER_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include <vector>
 #include <WiFi.h>
 #include "config.h"
 #include "comms/wifi.h"
 #include "ui/ui_manager.h"
 
 /**
  * @class WiFiManagerApp
  * @brief Application for managing WiFi connections
  */
 class WiFiManagerApp {
 public:
     /**
      * @brief Initialize the WiFi Manager application
      * @param parent Parent container for the application UI
      * @return true if initialization succeeded, false otherwise
      */
     bool init(lv_obj_t* parent);
 
     /**
      * @brief Start the WiFi Manager application
      * @return true if the application started successfully, false otherwise
      */
     bool start();
 
     /**
      * @brief Stop the WiFi Manager application
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
     lv_obj_t* _tabView = nullptr;          ///< Tab view for different WiFi functions
     lv_obj_t* _scanTab = nullptr;          ///< Tab for scanning networks
     lv_obj_t* _connectTab = nullptr;       ///< Tab for connecting to networks
     lv_obj_t* _statusTab = nullptr;        ///< Tab for WiFi status
     lv_obj_t* _settingsTab = nullptr;      ///< Tab for WiFi settings
     lv_obj_t* _networkList = nullptr;      ///< List of available networks
     lv_obj_t* _scanBtn = nullptr;          ///< Scan button
     lv_obj_t* _messageLabel = nullptr;     ///< Status message label
     lv_obj_t* _passwordTextArea = nullptr; ///< Password input field
     lv_obj_t* _connectBtn = nullptr;       ///< Connect button
     lv_obj_t* _disconnectBtn = nullptr;    ///< Disconnect button
     lv_obj_t* _apModeSwitch = nullptr;     ///< Switch for AP mode
     lv_obj_t* _keyboard = nullptr;         ///< Keyboard for input
     
     // State variables
     bool _isRunning = false;               ///< Application running state
     bool _isScanning = false;              ///< WiFi scanning state
     String _selectedSSID;                  ///< Currently selected SSID
     int _selectedNetworkIndex = -1;        ///< Index of selected network in scan results
     std::vector<WiFiResult> _scanResults;  ///< WiFi scan results
     WiFiConfigData _wifiConfig;            ///< WiFi configuration data
     
     // WiFi Manager reference
     WiFiManager& _wifiManager = WiFiManager::getInstance();
     
     /**
      * @brief Create the UI components for the application
      */
     void createUI();
     
     /**
      * @brief Start a WiFi scan operation
      */
     void startScan();
     
     /**
      * @brief Update the network list with scan results
      */
     void updateNetworkList();
     
     /**
      * @brief Update the WiFi status display
      */
     void updateWiFiStatus();
     
     /**
      * @brief Connect to the selected WiFi network
      */
     void connectToNetwork();
     
     /**
      * @brief Disconnect from the current WiFi network
      */
     void disconnectFromNetwork();
     
     /**
      * @brief Toggle AP mode on/off
      * @param state true to enable AP mode, false to disable
      */
     void toggleAPMode(bool state);
     
     /**
      * @brief Save the current WiFi configuration
      */
     void saveWiFiConfig();
     
     /**
      * @brief Load the WiFi configuration from storage
      */
     void loadWiFiConfig();
     
     // LVGL callback functions
     static void onScanBtnClicked(lv_obj_t* obj, lv_event_t event);
     static void onNetworkItemClicked(lv_obj_t* obj, lv_event_t event);
     static void onConnectBtnClicked(lv_obj_t* obj, lv_event_t event);
     static void onDisconnectBtnClicked(lv_obj_t* obj, lv_event_t event);
     static void onApModeSwitchChanged(lv_obj_t* obj, lv_event_t event);
     static void onKeyboardEvent(lv_obj_t* obj, lv_event_t event);
     static void onStatusUpdateTimer(lv_task_t* task);
 };
 
 #endif // TDECK_WIFI_MANAGER_H