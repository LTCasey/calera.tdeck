/**
 * @file wifi_manager.cpp
 * @brief Implementation of WiFi management application for T-Deck UI
 */

 #include "apps/wifi_manager.h"
 #include "system/fs_manager.h"
 #include <ArduinoJson.h>
 
 // External reference to the file system manager
 extern FSManager fsManager;
 
 bool WiFiManagerApp::init(lv_obj_t* parent) {
     if (parent == nullptr) {
         TDECK_LOG_E("WiFiManagerApp: Invalid parent container");
         return false;
     }
     
     _parentContainer = parent;
     
     // Load saved WiFi configuration
     loadWiFiConfig();
     
     TDECK_LOG_I("WiFiManagerApp: Initialized");
     return true;
 }
 
 bool WiFiManagerApp::start() {
     if (_isRunning) {
         TDECK_LOG_W("WiFiManagerApp: Already running");
         return true;
     }
     
     if (_parentContainer == nullptr) {
         TDECK_LOG_E("WiFiManagerApp: Not initialized");
         return false;
     }
     
     // Create the UI
     createUI();
     
     // Start with a WiFi scan
     startScan();
     
     // Create a task for periodic status updates
     lv_task_create(onStatusUpdateTimer, 1000, LV_TASK_PRIO_LOW, this);
     
     _isRunning = true;
     TDECK_LOG_I("WiFiManagerApp: Started");
     return true;
 }
 
 void WiFiManagerApp::stop() {
     if (!_isRunning) {
         return;
     }
     
     // Delete UI elements
     if (_appContainer != nullptr) {
         lv_obj_del(_appContainer);
         _appContainer = nullptr;
     }
     
     // Delete task
     lv_task_t* task = lv_task_get_next(NULL);
     while (task != NULL) {
         lv_task_t* next = lv_task_get_next(task);
         if (task->user_data == this) {
             lv_task_del(task);
         }
         task = next;
     }
     
     _isRunning = false;
     TDECK_LOG_I("WiFiManagerApp: Stopped");
 }
 
 bool WiFiManagerApp::isRunning() const {
     return _isRunning;
 }
 
 void WiFiManagerApp::createUI() {
     // Create main app container
     _appContainer = lv_cont_create(_parentContainer, NULL);
     lv_obj_set_size(_appContainer, lv_obj_get_width(_parentContainer), lv_obj_get_height(_parentContainer));
     lv_obj_set_pos(_appContainer, 0, 0);
     lv_cont_set_layout(_appContainer, LV_LAYOUT_OFF);
     
     // Create tab view
     _tabView = lv_tabview_create(_appContainer, NULL);
     lv_obj_set_size(_tabView, lv_obj_get_width(_appContainer), lv_obj_get_height(_appContainer));
     
     // Create tabs
     _scanTab = lv_tabview_add_tab(_tabView, "Scan");
     _connectTab = lv_tabview_add_tab(_tabView, "Connect");
     _statusTab = lv_tabview_add_tab(_tabView, "Status");
     _settingsTab = lv_tabview_add_tab(_tabView, "Settings");
     
     // Set up scan tab
     lv_page_set_scrl_layout(_scanTab, LV_LAYOUT_COLUMN_MID);
     
     _scanBtn = lv_btn_create(_scanTab, NULL);
     lv_obj_set_width(_scanBtn, lv_obj_get_width(_scanTab) - 40);
     lv_obj_align(_scanBtn, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
     lv_obj_set_event_cb(_scanBtn, onScanBtnClicked);
     
     lv_obj_t* scanBtnLabel = lv_label_create(_scanBtn, NULL);
     lv_label_set_text(scanBtnLabel, "Scan for Networks");
     
     _messageLabel = lv_label_create(_scanTab, NULL);
     lv_label_set_text(_messageLabel, "");
     lv_label_set_long_mode(_messageLabel, LV_LABEL_LONG_BREAK);
     lv_obj_set_width(_messageLabel, lv_obj_get_width(_scanTab) - 40);
     lv_obj_align(_messageLabel, _scanBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
     
     _networkList = lv_list_create(_scanTab, NULL);
     lv_obj_set_size(_networkList, lv_obj_get_width(_scanTab) - 20, lv_obj_get_height(_scanTab) - 120);
     lv_obj_align(_networkList, _messageLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
     
     // Set up connect tab
     _passwordTextArea = lv_textarea_create(_connectTab, NULL);
     lv_textarea_set_text(_passwordTextArea, "");
     lv_textarea_set_placeholder_text(_passwordTextArea, "Enter Password");
     lv_textarea_set_pwd_mode(_passwordTextArea, true);
     lv_textarea_set_one_line(_passwordTextArea, true);
     lv_obj_set_width(_passwordTextArea, lv_obj_get_width(_connectTab) - 40);
     lv_obj_align(_passwordTextArea, NULL, LV_ALIGN_IN_TOP_MID, 0, 20);
     
     _connectBtn = lv_btn_create(_connectTab, NULL);
     lv_obj_set_width(_connectBtn, lv_obj_get_width(_connectTab) - 80);
     lv_obj_align(_connectBtn, _passwordTextArea, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     lv_obj_set_event_cb(_connectBtn, onConnectBtnClicked);
     
     lv_obj_t* connectBtnLabel = lv_label_create(_connectBtn, NULL);
     lv_label_set_text(connectBtnLabel, "Connect");
     
     _disconnectBtn = lv_btn_create(_connectTab, _connectBtn);
     lv_obj_align(_disconnectBtn, _connectBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
     lv_obj_set_event_cb(_disconnectBtn, onDisconnectBtnClicked);
     
     lv_obj_t* disconnectBtnLabel = lv_label_create(_disconnectBtn, NULL);
     lv_label_set_text(disconnectBtnLabel, "Disconnect");
     
     // Set up keyboard
     _keyboard = lv_keyboard_create(_appContainer, NULL);
     lv_obj_set_width(_keyboard, LV_HOR_RES);
     lv_obj_set_height(_keyboard, LV_VER_RES / 2);
     lv_obj_set_pos(_keyboard, 0, LV_VER_RES);  // Initially hidden
     lv_keyboard_set_textarea(_keyboard, _passwordTextArea);
     lv_obj_set_event_cb(_keyboard, onKeyboardEvent);
     
     // Set up status tab
     lv_page_set_scrl_layout(_statusTab, LV_LAYOUT_COLUMN_MID);
     
     lv_obj_t* statusTitleLabel = lv_label_create(_statusTab, NULL);
     lv_label_set_text(statusTitleLabel, "WiFi Status");
     lv_obj_align(statusTitleLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
     
     // Add status fields
     lv_obj_t* connStatusLabel = lv_label_create(_statusTab, NULL);
     lv_label_set_text(connStatusLabel, "Connection Status: ");
     lv_obj_align(connStatusLabel, statusTitleLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     
     // Set up settings tab
     lv_page_set_scrl_layout(_settingsTab, LV_LAYOUT_COLUMN_MID);
     
     lv_obj_t* settingsTitleLabel = lv_label_create(_settingsTab, NULL);
     lv_label_set_text(settingsTitleLabel, "WiFi Settings");
     lv_obj_align(settingsTitleLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
     
     // AP Mode switch
     lv_obj_t* apModeLabel = lv_label_create(_settingsTab, NULL);
     lv_label_set_text(apModeLabel, "Access Point Mode");
     lv_obj_align(apModeLabel, settingsTitleLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     
     _apModeSwitch = lv_switch_create(_settingsTab, NULL);
     lv_obj_align(_apModeSwitch, apModeLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
     lv_obj_set_event_cb(_apModeSwitch, onApModeSwitchChanged);
     
     // Set initial switch state based on current mode
     lv_switch_on(_apModeSwitch, _wifiManager.isAPModeActive() ? LV_ANIM_ON : LV_ANIM_OFF);
     
     // Store object reference for callbacks
     lv_obj_set_user_data(_scanBtn, this);
     lv_obj_set_user_data(_networkList, this);
     lv_obj_set_user_data(_connectBtn, this);
     lv_obj_set_user_data(_disconnectBtn, this);
     lv_obj_set_user_data(_apModeSwitch, this);
     lv_obj_set_user_data(_keyboard, this);
 }
 
 void WiFiManagerApp::startScan() {
     if (_isScanning) {
         return;
     }
     
     _isScanning = true;
     lv_label_set_text(_messageLabel, "Scanning for networks...");
     
     // Clear previous results
     lv_list_clean(_networkList);
     _scanResults.clear();
     
     // Start scan asynchronously
     _wifiManager.startScan([this](const std::vector<WiFiResult>& results, bool success) {
         _isScanning = false;
         
         if (!success) {
             lv_label_set_text(_messageLabel, "Scan failed. Try again.");
             return;
         }
         
         _scanResults = results;
         
         if (_scanResults.empty()) {
             lv_label_set_text(_messageLabel, "No networks found.");
             return;
         }
         
         lv_label_set_text(_messageLabel, String(String(_scanResults.size()) + " networks found").c_str());
         updateNetworkList();
     });
 }
 
 void WiFiManagerApp::updateNetworkList() {
     // Clear list
     lv_list_clean(_networkList);
     
     // Add networks to list
     for (size_t i = 0; i < _scanResults.size(); i++) {
         const WiFiResult& result = _scanResults[i];
         
         // Determine signal strength icon
         const char* signalIcon;
         if (result.rssi > -50) {
             signalIcon = LV_SYMBOL_WIFI;
         } else if (result.rssi > -70) {
             signalIcon = LV_SYMBOL_WIFI;  // Would be better to have different WiFi symbols
         } else {
             signalIcon = LV_SYMBOL_WIFI;
         }
         
         // Build list item text
         String itemText = String(signalIcon) + " " + result.ssid;
         
         // Add lock symbol for secured networks
         if (result.encryption != WIFI_AUTH_OPEN) {
             itemText += " " + String(LV_SYMBOL_LOCK);
         }
         
         // Add signal strength
         itemText += String(" (") + String(result.rssi) + String(" dBm)");
         
         // Create list item
         lv_obj_t* listBtn = lv_list_add_btn(_networkList, NULL, itemText.c_str());
         lv_obj_set_event_cb(listBtn, onNetworkItemClicked);
         
         // Store the index in user data
         lv_obj_set_user_data(listBtn, (void*)(i + 1));  // +1 to avoid NULL
     }
 }
 
 void WiFiManagerApp::updateWiFiStatus() {
     // Update status information in the status tab
     if (!_isRunning) {
         return;
     }
     
     // Clear existing objects
     lv_obj_t* statusScrl = lv_page_get_scrl(_statusTab);
     lv_obj_t* child = lv_obj_get_child(statusScrl, NULL);
     
     // Skip the title label
     if (child) {
         child = lv_obj_get_child(statusScrl, child);
     }
     
     // Delete all status fields
     while (child) {
         lv_obj_t* next = lv_obj_get_child(statusScrl, child);
         if (lv_obj_get_y(child) > 50) {  // Skip title and header
             lv_obj_del(child);
         }
         child = next;
     }
     
     // Create new status information
     lv_obj_t* connStatusLabel = lv_label_create(_statusTab, NULL);
     
     if (_wifiManager.isConnected()) {
         String statusText = "Status: Connected\n";
         statusText += "SSID: " + _wifiManager.getCurrentSSID() + "\n";
         statusText += "IP: " + _wifiManager.getIPAddress().toString() + "\n";
         statusText += "Signal: " + String(_wifiManager.getCurrentRSSI()) + " dBm\n";
         statusText += "MAC: " + _wifiManager.getMACAddress();
         
         lv_label_set_text(connStatusLabel, statusText.c_str());
     } else if (_wifiManager.isAPModeActive()) {
         String statusText = "Status: Access Point Mode\n";
         statusText += "SSID: " + _wifiManager.getAPSSID() + "\n";
         statusText += "IP: " + _wifiManager.getIPAddress().toString() + "\n";
         statusText += "Connected Clients: " + String(_wifiManager.getConnectedClientsCount());
         
         lv_label_set_text(connStatusLabel, statusText.c_str());
     } else {
         lv_label_set_text(connStatusLabel, "Status: Disconnected");
     }
     
     lv_label_set_long_mode(connStatusLabel, LV_LABEL_LONG_BREAK);
     lv_obj_set_width(connStatusLabel, lv_obj_get_width(_statusTab) - 40);
     lv_obj_align(connStatusLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 60);
 }
 
 void WiFiManagerApp::connectToNetwork() {
     if (_selectedNetworkIndex < 0 || _selectedNetworkIndex >= (int)_scanResults.size()) {
         lv_label_set_text(_messageLabel, "No network selected");
         return;
     }
     
     const WiFiResult& network = _scanResults[_selectedNetworkIndex];
     String password = String(lv_textarea_get_text(_passwordTextArea));
     
     lv_label_set_text(_messageLabel, String("Connecting to " + network.ssid + "...").c_str());
     
     // Store connection information for future reconnect
     _wifiConfig.lastSSID = network.ssid;
     _wifiConfig.lastPassword = password;
     _wifiConfig.autoConnect = true;
     
     // Connect to the network
     _wifiManager.connect(network.ssid.c_str(), password.c_str(), [this](bool success) {
         if (success) {
             lv_label_set_text(_messageLabel, String("Connected to " + _wifiManager.getCurrentSSID()).c_str());
             saveWiFiConfig();
             updateWiFiStatus();
         } else {
             lv_label_set_text(_messageLabel, "Connection failed");
         }
     });
 }
 
 void WiFiManagerApp::disconnectFromNetwork() {
     _wifiManager.disconnect();
     lv_label_set_text(_messageLabel, "Disconnected");
     updateWiFiStatus();
 }
 
 void WiFiManagerApp::toggleAPMode(bool state) {
     if (state) {
         _wifiManager.startAP(_wifiConfig.apSSID.c_str(), _wifiConfig.apPassword.c_str());
         lv_label_set_text(_messageLabel, String("AP Mode started: " + _wifiConfig.apSSID).c_str());
     } else {
         _wifiManager.stopAP();
         lv_label_set_text(_messageLabel, "AP Mode stopped");
     }
     
     updateWiFiStatus();
 }
 
 void WiFiManagerApp::saveWiFiConfig() {
     // Save WiFi configuration to file
     DynamicJsonDocument doc(512);
     
     doc["autoConnect"] = _wifiConfig.autoConnect;
     doc["lastSSID"] = _wifiConfig.lastSSID;
     doc["lastPassword"] = _wifiConfig.lastPassword;
     doc["apSSID"] = _wifiConfig.apSSID;
     doc["apPassword"] = _wifiConfig.apPassword;
     
     fsManager.saveJsonToFile(TDECK_FS_WIFI_CONFIG_FILE, doc);
     TDECK_LOG_I("WiFiManagerApp: Configuration saved");
 }
 
 void WiFiManagerApp::loadWiFiConfig() {
     // Load WiFi configuration from file
     DynamicJsonDocument doc(512);
     
     // Set defaults
     _wifiConfig.autoConnect = false;
     _wifiConfig.lastSSID = "";
     _wifiConfig.lastPassword = "";
     _wifiConfig.apSSID = TDECK_WIFI_AP_SSID;
     _wifiConfig.apPassword = TDECK_WIFI_AP_PASSWORD;
     
     // Try to load from file
     if (fsManager.loadJsonFromFile(TDECK_FS_WIFI_CONFIG_FILE, doc)) {
         _wifiConfig.autoConnect = doc["autoConnect"] | false;
         _wifiConfig.lastSSID = doc["lastSSID"] | "";
         _wifiConfig.lastPassword = doc["lastPassword"] | "";
         _wifiConfig.apSSID = doc["apSSID"] | TDECK_WIFI_AP_SSID;
         _wifiConfig.apPassword = doc["apPassword"] | TDECK_WIFI_AP_PASSWORD;
         
         TDECK_LOG_I("WiFiManagerApp: Configuration loaded");
         
         // Auto-connect if enabled
         if (_wifiConfig.autoConnect && !_wifiConfig.lastSSID.isEmpty()) {
             _wifiManager.connect(_wifiConfig.lastSSID.c_str(), _wifiConfig.lastPassword.c_str(), nullptr);
         }
     } else {
         TDECK_LOG_I("WiFiManagerApp: Using default configuration");
     }
 }
 
 // LVGL event callbacks
 void WiFiManagerApp::onScanBtnClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     WiFiManagerApp* app = (WiFiManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         app->startScan();
     }
 }
 
 void WiFiManagerApp::onNetworkItemClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     WiFiManagerApp* app = (WiFiManagerApp*)lv_obj_get_user_data(obj->parent);
     if (!app) {
         return;
     }
     
     // Get network index (stored as user_data + 1)
     int index = (int)lv_obj_get_user_data(obj) - 1;
     if (index >= 0 && index < (int)app->_scanResults.size()) {
         app->_selectedNetworkIndex = index;
         app->_selectedSSID = app->_scanResults[index].ssid;
         
         // Switch to connect tab
         lv_tabview_set_tab_act(app->_tabView, 1, LV_ANIM_ON);
         
         // Update connect button text
         lv_obj_t* connectBtnLabel = lv_obj_get_child(app->_connectBtn, NULL);
         if (connectBtnLabel) {
             lv_label_set_text(connectBtnLabel, String("Connect to " + app->_selectedSSID).c_str());
         }
     }
 }
 
 void WiFiManagerApp::onConnectBtnClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     WiFiManagerApp* app = (WiFiManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         app->connectToNetwork();
     }
 }
 
 void WiFiManagerApp::onDisconnectBtnClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     WiFiManagerApp* app = (WiFiManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         app->disconnectFromNetwork();
     }
 }
 
 void WiFiManagerApp::onApModeSwitchChanged(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_VALUE_CHANGED) {
         return;
     }
     
     WiFiManagerApp* app = (WiFiManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         bool state = lv_switch_get_state(obj);
         app->toggleAPMode(state);
     }
 }
 
 void WiFiManagerApp::onKeyboardEvent(lv_obj_t* obj, lv_event_t event) {
     WiFiManagerApp* app = (WiFiManagerApp*)lv_obj_get_user_data(obj);
     if (!app) {
         return;
     }
     
     if (event == LV_EVENT_CANCEL) {
         lv_keyboard_set_textarea(obj, NULL);
         lv_obj_set_pos(obj, 0, LV_VER_RES);  // Hide keyboard
     } else if (event == LV_EVENT_APPLY) {
         lv_keyboard_set_textarea(obj, NULL);
         lv_obj_set_pos(obj, 0, LV_VER_RES);  // Hide keyboard
         app->connectToNetwork();  // Attempt to connect after entering password
     }
 }
 
 void WiFiManagerApp::onStatusUpdateTimer(lv_task_t* task) {
     WiFiManagerApp* app = (WiFiManagerApp*)task->user_data;
     if (app) {
         app->updateWiFiStatus();
     }
 }