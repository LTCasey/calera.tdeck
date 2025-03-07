/**
 * @file ble_manager.cpp
 * @brief Implementation of Bluetooth management application for T-Deck UI
 */

 #include "apps/ble_manager.h"
 #include "system/fs_manager.h"
 #include <ArduinoJson.h>
 
 // External reference to the file system manager
 extern FSManager fsManager;
 
 bool BLEManagerApp::init(lv_obj_t* parent) {
     if (parent == nullptr) {
         TDECK_LOG_E("BLEManagerApp: Invalid parent container");
         return false;
     }
     
     _parentContainer = parent;
     
     // Load saved BLE settings
     loadSettings();
     
     TDECK_LOG_I("BLEManagerApp: Initialized");
     return true;
 }
 
 bool BLEManagerApp::start() {
     if (_isRunning) {
         TDECK_LOG_W("BLEManagerApp: Already running");
         return true;
     }
     
     if (_parentContainer == nullptr) {
         TDECK_LOG_E("BLEManagerApp: Not initialized");
         return false;
     }
     
     // Create the UI
     createUI();
     
     // Create a task for periodic status updates
     lv_task_create(onStatusUpdateTimer, 1000, LV_TASK_PRIO_LOW, this);
     
     _isRunning = true;
     TDECK_LOG_I("BLEManagerApp: Started");
     return true;
 }
 
 void BLEManagerApp::stop() {
     if (!_isRunning) {
         return;
     }
     
     // Stop scanning if active
     if (_isScanning) {
         stopScan();
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
     TDECK_LOG_I("BLEManagerApp: Stopped");
 }
 
 bool BLEManagerApp::isRunning() const {
     return _isRunning;
 }
 
 void BLEManagerApp::createUI() {
     // Create main app container
     _appContainer = lv_cont_create(_parentContainer, NULL);
     lv_obj_set_size(_appContainer, lv_obj_get_width(_parentContainer), lv_obj_get_height(_parentContainer));
     lv_obj_set_pos(_appContainer, 0, 0);
     lv_cont_set_layout(_appContainer, LV_LAYOUT_OFF);
     
     // Create tab view
     _tabView = lv_tabview_create(_appContainer, NULL);
     lv_obj_set_size(_tabView, lv_obj_get_width(_appContainer), lv_obj_get_height(_appContainer));
     
     // Create tabs
     _devicesTab = lv_tabview_add_tab(_tabView, "Devices");
     _statusTab = lv_tabview_add_tab(_tabView, "Status");
     _settingsTab = lv_tabview_add_tab(_tabView, "Settings");
     
     // Set up devices tab
     lv_page_set_scrl_layout(_devicesTab, LV_LAYOUT_COLUMN_MID);
     
     _scanBtn = lv_btn_create(_devicesTab, NULL);
     lv_obj_set_width(_scanBtn, lv_obj_get_width(_devicesTab) / 2 - 20);
     lv_obj_align(_scanBtn, NULL, LV_ALIGN_IN_TOP_LEFT, 10, 10);
     lv_obj_set_event_cb(_scanBtn, onScanBtnClicked);
     
     lv_obj_t* scanBtnLabel = lv_label_create(_scanBtn, NULL);
     lv_label_set_text(scanBtnLabel, "Scan");
     
     _stopScanBtn = lv_btn_create(_devicesTab, NULL);
     lv_obj_set_width(_stopScanBtn, lv_obj_get_width(_devicesTab) / 2 - 20);
     lv_obj_align(_stopScanBtn, _scanBtn, LV_ALIGN_OUT_RIGHT_MID, 20, 0);
     lv_obj_set_event_cb(_stopScanBtn, onStopScanBtnClicked);
     
     lv_obj_t* stopScanBtnLabel = lv_label_create(_stopScanBtn, NULL);
     lv_label_set_text(stopScanBtnLabel, "Stop");
     
     _messageLabel = lv_label_create(_devicesTab, NULL);
     lv_label_set_text(_messageLabel, "");
     lv_label_set_long_mode(_messageLabel, LV_LABEL_LONG_BREAK);
     lv_obj_set_width(_messageLabel, lv_obj_get_width(_devicesTab) - 40);
     lv_obj_align(_messageLabel, _scanBtn, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
     
     _deviceList = lv_list_create(_devicesTab, NULL);
     lv_obj_set_size(_deviceList, lv_obj_get_width(_devicesTab) - 20, lv_obj_get_height(_devicesTab) - 100);
     lv_obj_align(_deviceList, _messageLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
     lv_list_set_sb_mode(_deviceList, LV_SB_MODE_AUTO);
     
     // Set up status tab
     lv_page_set_scrl_layout(_statusTab, LV_LAYOUT_COLUMN_MID);
     
     lv_obj_t* statusTitleLabel = lv_label_create(_statusTab, NULL);
     lv_label_set_text(statusTitleLabel, "Bluetooth Status");
     lv_obj_align(statusTitleLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
     
     // Status fields will be added in updateBLEStatus()
     
     // Set up settings tab
     lv_page_set_scrl_layout(_settingsTab, LV_LAYOUT_COLUMN_MID);
     
     lv_obj_t* settingsTitleLabel = lv_label_create(_settingsTab, NULL);
     lv_label_set_text(settingsTitleLabel, "Bluetooth Settings");
     lv_obj_align(settingsTitleLabel, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
     
     // Bluetooth Enable switch
     lv_obj_t* btEnableLabel = lv_label_create(_settingsTab, NULL);
     lv_label_set_text(btEnableLabel, "Enable Bluetooth");
     lv_obj_align(btEnableLabel, settingsTitleLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     
     _btEnableSwitch = lv_switch_create(_settingsTab, NULL);
     lv_obj_align(_btEnableSwitch, btEnableLabel, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
     lv_obj_set_event_cb(_btEnableSwitch, onBTEnableSwitchChanged);
     
     // Bluetooth visibility switch
     lv_obj_t* btVisibilityLabel = lv_label_create(_settingsTab, NULL);
     lv_label_set_text(btVisibilityLabel, "Device Visibility");
     lv_obj_align(btVisibilityLabel, btEnableLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     
     _btVisibilitySwitch = lv_switch_create(_settingsTab, NULL);
     lv_obj_align(_btVisibilitySwitch, btVisibilityLabel, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
     lv_obj_set_event_cb(_btVisibilitySwitch, onBTVisibilitySwitchChanged);
     
     // Device name
     lv_obj_t* btNameLabel = lv_label_create(_settingsTab, NULL);
     lv_label_set_text(btNameLabel, "Device Name");
     lv_obj_align(btNameLabel, btVisibilityLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     
     _btNameTextArea = lv_textarea_create(_settingsTab, NULL);
     lv_textarea_set_text(_btNameTextArea, _btManager.getDeviceName().c_str());
     lv_textarea_set_placeholder_text(_btNameTextArea, "Enter device name");
     lv_textarea_set_one_line(_btNameTextArea, true);
     lv_obj_set_width(_btNameTextArea, lv_obj_get_width(_settingsTab) - 40);
     lv_obj_align(_btNameTextArea, btNameLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
     
     // Save settings button
     _saveSettingsBtn = lv_btn_create(_settingsTab, NULL);
     lv_obj_set_width(_saveSettingsBtn, lv_obj_get_width(_settingsTab) - 80);
     lv_obj_align(_saveSettingsBtn, _btNameTextArea, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     lv_obj_set_event_cb(_saveSettingsBtn, onSaveSettingsBtnClicked);
     
     lv_obj_t* saveSettingsBtnLabel = lv_label_create(_saveSettingsBtn, NULL);
     lv_label_set_text(saveSettingsBtnLabel, "Save Settings");
     
     // Set up keyboard
     _keyboard = lv_keyboard_create(_appContainer, NULL);
     lv_obj_set_width(_keyboard, LV_HOR_RES);
     lv_obj_set_height(_keyboard, LV_VER_RES / 2);
     lv_obj_set_pos(_keyboard, 0, LV_VER_RES);  // Initially hidden
     lv_keyboard_set_textarea(_keyboard, _btNameTextArea);
     lv_obj_set_event_cb(_keyboard, onKeyboardEvent);
     
     // Set initial states
     lv_switch_on(_btEnableSwitch, _btManager.isEnabled() ? LV_ANIM_ON : LV_ANIM_OFF);
     lv_switch_on(_btVisibilitySwitch, _btManager.isDiscoverable() ? LV_ANIM_ON : LV_ANIM_OFF);
     
     // Store object reference for callbacks
     lv_obj_set_user_data(_scanBtn, this);
     lv_obj_set_user_data(_stopScanBtn, this);
     lv_obj_set_user_data(_deviceList, this);
     lv_obj_set_user_data(_btEnableSwitch, this);
     lv_obj_set_user_data(_btVisibilitySwitch, this);
     lv_obj_set_user_data(_saveSettingsBtn, this);
     lv_obj_set_user_data(_keyboard, this);
     
     // Update status
     updateBLEStatus();
 }
 
 void BLEManagerApp::saveSettings() {
     // Save settings
     String deviceName = lv_textarea_get_text(_btNameTextArea);
     
     if (deviceName.length() == 0) {
         lv_label_set_text(_messageLabel, "Device name cannot be empty");
         return;
     }
     
     if (_btManager.isEnabled()) {
         if (!_btManager.setDeviceName(deviceName.c_str())) {
             lv_label_set_text(_messageLabel, "Failed to set device name");
             return;
         }
     }
     
     // Save to configuration file
     DynamicJsonDocument doc(512);
     doc["deviceName"] = deviceName;
     doc["autoEnable"] = lv_switch_get_state(_btEnableSwitch);
     doc["discoverable"] = lv_switch_get_state(_btVisibilitySwitch);
     
     if (fsManager.saveJsonToFile(TDECK_FS_BT_CONFIG_FILE, doc)) {
         lv_label_set_text(_messageLabel, "Settings saved");
         updateBLEStatus();
     } else {
         lv_label_set_text(_messageLabel, "Failed to save settings");
     }
 }
 
 void BLEManagerApp::loadSettings() {
     // Load settings from configuration file
     DynamicJsonDocument doc(512);
     
     if (fsManager.loadJsonFromFile(TDECK_FS_BT_CONFIG_FILE, doc)) {
         String deviceName = doc["deviceName"] | TDECK_BT_DEVICE_NAME;
         bool autoEnable = doc["autoEnable"] | true;
         bool discoverable = doc["discoverable"] | false;
         
         if (autoEnable) {
             _btManager.enable();
         }
         
         if (_btManager.isEnabled()) {
             _btManager.setDeviceName(deviceName.c_str());
             _btManager.setDiscoverable(discoverable);
         }
         
         TDECK_LOG_I("BLEManagerApp: Settings loaded");
     } else {
         TDECK_LOG_I("BLEManagerApp: Using default settings");
     }
 }
 
 // LVGL event callbacks
 void BLEManagerApp::onScanBtnClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         app->startScan();
     }
 }
 
 void BLEManagerApp::onStopScanBtnClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         app->stopScan();
     }
 }
 
 void BLEManagerApp::onDeviceItemClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj->parent);
     if (!app) {
         return;
     }
     
     // Get device index (stored as user_data + 1)
     int index = (int)lv_obj_get_user_data(obj) - 1;
     if (index >= 0 && index < (int)app->_deviceList_data.size()) {
         app->_selectedDeviceIndex = index;
         app->_selectedDeviceAddress = app->_deviceList_data[index].address;
         
         // Show context menu for the device
         app->onDeviceContextMenu(obj, event);
     }
 }
 
 void BLEManagerApp::onDeviceContextMenu(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj->parent);
     if (!app || app->_selectedDeviceIndex < 0) {
         return;
     }
     
     const BLEDeviceInfo& device = app->_deviceList_data[app->_selectedDeviceIndex];
     
     // Create a message box with options
     static const char* btns[] = {
         "Connect", 
         "Disconnect", 
         "Pair", 
         "Unpair", 
         "Cancel", 
         ""
     };
     
     lv_obj_t* mbox = lv_msgbox_create(lv_layer_top(), NULL);
     lv_msgbox_set_text(mbox, String("Device: " + device.name).c_str());
     lv_msgbox_add_btns(mbox, btns);
     lv_obj_set_width(mbox, lv_obj_get_width(app->_appContainer) - 40);
     lv_obj_set_pos(mbox, (lv_obj_get_width(app->_appContainer) - lv_obj_get_width(mbox)) / 2,
                   (lv_obj_get_height(app->_appContainer) - lv_obj_get_height(mbox)) / 2);
     
     // Set callback
     lv_obj_set_user_data(mbox, app);
     lv_obj_set_event_cb(mbox, [](lv_obj_t* obj, lv_event_t event) {
         if (event != LV_EVENT_VALUE_CHANGED) {
             return;
         }
         
         BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj);
         if (!app) {
             return;
         }
         
         const char* txt = lv_msgbox_get_active_btn_text(obj);
         if (txt) {
             String btnText = String(txt);
             
             if (btnText == "Connect") {
                 app->connectToDevice(app->_selectedDeviceIndex);
             } else if (btnText == "Disconnect") {
                 app->disconnectFromDevice();
             } else if (btnText == "Pair") {
                 app->pairWithDevice(app->_selectedDeviceIndex);
             } else if (btnText == "Unpair") {
                 app->unpairFromDevice(app->_selectedDeviceIndex);
             }
         }
         
         lv_obj_del(obj);
     });
 }
 
 void BLEManagerApp::onBTEnableSwitchChanged(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_VALUE_CHANGED) {
         return;
     }
     
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         bool state = lv_switch_get_state(obj);
         app->enableBluetooth(state);
     }
 }
 
 void BLEManagerApp::onBTVisibilitySwitchChanged(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_VALUE_CHANGED) {
         return;
     }
     
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         bool state = lv_switch_get_state(obj);
         app->setVisibility(state);
     }
 }
 
 void BLEManagerApp::onSaveSettingsBtnClicked(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) {
         return;
     }
     
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj);
     if (app) {
         app->saveSettings();
     }
 }
 
 void BLEManagerApp::onKeyboardEvent(lv_obj_t* obj, lv_event_t event) {
     BLEManagerApp* app = (BLEManagerApp*)lv_obj_get_user_data(obj);
     if (!app) {
         return;
     }
     
     if (event == LV_EVENT_CANCEL) {
         lv_keyboard_set_textarea(obj, NULL);
         lv_obj_set_pos(obj, 0, LV_VER_RES);  // Hide keyboard
     } else if (event == LV_EVENT_APPLY) {
         lv_keyboard_set_textarea(obj, NULL);
         lv_obj_set_pos(obj, 0, LV_VER_RES);  // Hide keyboard
         app->saveSettings();
     }
 }
 
 void BLEManagerApp::onStatusUpdateTimer(lv_task_t* task) {
     BLEManagerApp* app = (BLEManagerApp*)task->user_data;
     if (app) {
         app->updateBLEStatus();
     }
 }
 
 void BLEManagerApp::startScan() {
     if (_isScanning) {
         return;
     }
     
     if (!_btManager.isEnabled()) {
         lv_label_set_text(_messageLabel, "Bluetooth is disabled. Enable in Settings tab.");
         return;
     }
     
     _isScanning = true;
     lv_label_set_text(_messageLabel, "Scanning for devices...");
     
     // Clear previous results
     lv_list_clean(_deviceList);
     _deviceList_data.clear();
     
     // Start scan
     _btManager.startScan([this](BLEDevice* device, int rssi, bool isComplete) {
         if (device) {
             // Check if device already in the list
             bool found = false;
             for (auto& dev : _deviceList_data) {
                 if (dev.address == device->getAddress().toString()) {
                     // Update RSSI
                     dev.rssi = rssi;
                     found = true;
                     break;
                 }
             }
             
             if (!found) {
                 // Add new device
                 BLEDeviceInfo info;
                 info.address = device->getAddress().toString();
                 info.name = device->getName().length() > 0 ? device->getName() : "Unknown";
                 info.rssi = rssi;
                 info.isPaired = _btManager.isPaired(device);
                 info.isConnected = _btManager.isConnected(device);
                 
                 _deviceList_data.push_back(info);
                 
                 // Update the list
                 updateDeviceList();
             }
         }
         
         if (isComplete) {
             _isScanning = false;
             if (_deviceList_data.empty()) {
                 lv_label_set_text(_messageLabel, "No devices found.");
             } else {
                 lv_label_set_text(_messageLabel, String(String(_deviceList_data.size()) + " devices found").c_str());
             }
         }
     });
 }
 
 void BLEManagerApp::stopScan() {
     if (!_isScanning) {
         return;
     }
     
     _btManager.stopScan();
     _isScanning = false;
     lv_label_set_text(_messageLabel, "Scan stopped.");
 }
 
 void BLEManagerApp::updateDeviceList() {
     // Clear list
     lv_list_clean(_deviceList);
     
     // Add devices to list
     for (size_t i = 0; i < _deviceList_data.size(); i++) {
         const BLEDeviceInfo& device = _deviceList_data[i];
         
         // Build list item text
         String itemText = device.name;
         itemText += " [" + device.address + "]";
         itemText += String(" (") + String(device.rssi) + String(" dBm)");
         
         // Add status indicators
         if (device.isPaired) {
             itemText += " " + String(LV_SYMBOL_OK);
         }
         
         if (device.isConnected) {
             itemText += " " + String(LV_SYMBOL_WIFI);
         }
         
         // Create list item
         lv_obj_t* listBtn = lv_list_add_btn(_deviceList, NULL, itemText.c_str());
         lv_obj_set_event_cb(listBtn, onDeviceItemClicked);
         
         // Store the index in user data
         lv_obj_set_user_data(listBtn, (void*)(i + 1));  // +1 to avoid NULL
     }
 }
 
 void BLEManagerApp::updateBLEStatus() {
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
         if (lv_obj_get_y(child) > 50) {  // Skip title
             lv_obj_del(child);
         }
         child = next;
     }
     
     // Create new status information
     lv_obj_t* statusLabel = lv_label_create(_statusTab, NULL);
     
     String statusText = "Bluetooth: ";
     statusText += _btManager.isEnabled() ? "Enabled" : "Disabled";
     statusText += "\n";
     
     if (_btManager.isEnabled()) {
         statusText += "Device Name: " + _btManager.getDeviceName() + "\n";
         statusText += "Visibility: " + String(_btManager.isDiscoverable() ? "Visible" : "Hidden") + "\n";
         statusText += "MAC Address: " + _btManager.getMACAddress() + "\n";
         
         // Add connected devices
         int connectedCount = _btManager.getConnectedDevicesCount();
         statusText += "Connected Devices: " + String(connectedCount) + "\n";
         
         if (connectedCount > 0) {
             std::vector<BLEDevice*> connectedDevices = _btManager.getConnectedDevices();
             statusText += "\nConnected Devices:\n";
             
             for (auto device : connectedDevices) {
                 statusText += "- " + String(device->getName().c_str()) + " [" + device->getAddress().toString() + "]\n";
             }
         }
     }
     
     lv_label_set_text(statusLabel, statusText.c_str());
     lv_label_set_long_mode(statusLabel, LV_LABEL_LONG_BREAK);
     lv_obj_set_width(statusLabel, lv_obj_get_width(_statusTab) - 40);
     lv_obj_align(statusLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 20, 60);
 }
 
 void BLEManagerApp::connectToDevice(int deviceIndex) {
     if (deviceIndex < 0 || deviceIndex >= (int)_deviceList_data.size()) {
         lv_label_set_text(_messageLabel, "Invalid device selected");
         return;
     }
     
     const BLEDeviceInfo& device = _deviceList_data[deviceIndex];
     
     if (device.isConnected) {
         lv_label_set_text(_messageLabel, "Device already connected");
         return;
     }
     
     lv_label_set_text(_messageLabel, String("Connecting to " + device.name + "...").c_str());
     
     _btManager.connect(device.address.c_str(), [this, device](bool success) {
         if (success) {
             lv_label_set_text(_messageLabel, String("Connected to " + device.name).c_str());
             updateBLEStatus();
             updateDeviceList();
         } else {
             lv_label_set_text(_messageLabel, String("Failed to connect to " + device.name).c_str());
         }
     });
 }
 
 void BLEManagerApp::disconnectFromDevice() {
     if (_selectedDeviceIndex < 0 || _selectedDeviceIndex >= (int)_deviceList_data.size()) {
         lv_label_set_text(_messageLabel, "No device selected");
         return;
     }
     
     const BLEDeviceInfo& device = _deviceList_data[_selectedDeviceIndex];
     
     if (!device.isConnected) {
         lv_label_set_text(_messageLabel, "Device not connected");
         return;
     }
     
     lv_label_set_text(_messageLabel, String("Disconnecting from " + device.name + "...").c_str());
     
     _btManager.disconnect(device.address.c_str(), [this, device](bool success) {
         if (success) {
             lv_label_set_text(_messageLabel, String("Disconnected from " + device.name).c_str());
             updateBLEStatus();
             updateDeviceList();
         } else {
             lv_label_set_text(_messageLabel, String("Failed to disconnect from " + device.name).c_str());
         }
     });
 }
 
 void BLEManagerApp::pairWithDevice(int deviceIndex) {
     if (deviceIndex < 0 || deviceIndex >= (int)_deviceList_data.size()) {
         lv_label_set_text(_messageLabel, "Invalid device selected");
         return;
     }
     
     const BLEDeviceInfo& device = _deviceList_data[deviceIndex];
     
     if (device.isPaired) {
         lv_label_set_text(_messageLabel, "Device already paired");
         return;
     }
     
     lv_label_set_text(_messageLabel, String("Pairing with " + device.name + "...").c_str());
     
     _btManager.pair(device.address.c_str(), [this, device](bool success) {
         if (success) {
             lv_label_set_text(_messageLabel, String("Paired with " + device.name).c_str());
             updateBLEStatus();
             updateDeviceList();
         } else {
             lv_label_set_text(_messageLabel, String("Failed to pair with " + device.name).c_str());
         }
     });
 }
 
 void BLEManagerApp::unpairFromDevice(int deviceIndex) {
     if (deviceIndex < 0 || deviceIndex >= (int)_deviceList_data.size()) {
         lv_label_set_text(_messageLabel, "Invalid device selected");
         return;
     }
     
     const BLEDeviceInfo& device = _deviceList_data[deviceIndex];
     
     if (!device.isPaired) {
         lv_label_set_text(_messageLabel, "Device not paired");
         return;
     }
     
     lv_label_set_text(_messageLabel, String("Unpairing from " + device.name + "...").c_str());
     
     _btManager.unpair(device.address.c_str(), [this, device](bool success) {
         if (success) {
             lv_label_set_text(_messageLabel, String("Unpaired from " + device.name).c_str());
             updateBLEStatus();
             updateDeviceList();
         } else {
             lv_label_set_text(_messageLabel, String("Failed to unpair from " + device.name).c_str());
         }
     });
 }
 
 void BLEManagerApp::enableBluetooth(bool enable) {
     if (enable) {
         if (!_btManager.enable()) {
             lv_label_set_text(_messageLabel, "Failed to enable Bluetooth");
             lv_switch_off(_btEnableSwitch, LV_ANIM_ON);
             return;
         }
     } else {
         if (!_btManager.disable()) {
             lv_label_set_text(_messageLabel, "Failed to disable Bluetooth");
             lv_switch_on(_btEnableSwitch, LV_ANIM_ON);
             return;
         }
     }
     
     lv_label_set_text(_messageLabel, enable ? "Bluetooth enabled" : "Bluetooth disabled");
     updateBLEStatus();
 }
 
 void BLEManagerApp::setVisibility(bool visible) {
     if (!_btManager.isEnabled()) {
         lv_label_set_text(_messageLabel, "Enable Bluetooth first");
         lv_switch_off(_btVisibilitySwitch, LV_ANIM_ON);
         return;
     }
     
     if (visible) {
         if (!_btManager.setDiscoverable(true)) {
             lv_label_set_text(_messageLabel, "Failed to make device visible");
             lv_switch_off(_btVisibilitySwitch, LV_ANIM_ON);
             return;
         }
     } else {
         if (!_btManager.setDiscoverable(false)) {
             lv_label_set_text(_messageLabel, "Failed to hide device");
             lv_switch_on(_btVisibilitySwitch, LV_ANIM_ON);
             return;
         }
     }
     
     lv_label_set_text(_messageLabel, visible ? "Device is now visible" : "Device is now hidden");
     updateBLEStatus();