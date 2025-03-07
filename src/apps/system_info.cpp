/**
 * @file system_info.cpp
 * @brief Implementation of the System Information application
 */

 #include "system_info.h"
 #include <esp_system.h>
 #include <esp_chip_info.h>
 #include <esp_flash.h>
 #include <esp_psram.h>
 #include <esp_wifi.h>
 #include <esp_bt.h>
 #include <Ticker.h>
 #include "../comms/wifi.h"
 #include "../comms/bluetooth.h"
 #include "../comms/lora.h"
 #include "../hal/display.h"
 #include "../hal/sdcard.h"
 #include "../ui/theme.h"
 
 // Update interval in milliseconds
 #define SYSTEM_INFO_UPDATE_INTERVAL 1000
 
 // Static event handler for back button
 static void _backBtnEventHandler(lv_obj_t* obj, lv_event_t event) {
     if (event == LV_EVENT_CLICKED) {
         // Get the user data pointer which points to our SystemInfo instance
         SystemInfo* instance = (SystemInfo*)lv_obj_get_user_data(obj);
         if (instance) {
             instance->stop();
         }
     }
 }
 
 SystemInfo::SystemInfo() : 
     _mainPage(nullptr),
     _tabView(nullptr),
     _tabSystem(nullptr),
     _tabMemory(nullptr),
     _tabNetwork(nullptr),
     _tabHardware(nullptr),
     _powerManager(nullptr),
     _isInitialized(false),
     _isActive(false),
     _lastUpdateTime(0) {
 }
 
 SystemInfo::~SystemInfo() {
     stop();
 }
 
 bool SystemInfo::init(lv_obj_t* parent) {
     if (_isInitialized) {
         return true;
     }
     
     // Create main page container
     _mainPage = lv_obj_create(parent, NULL);
     lv_obj_set_size(_mainPage, lv_obj_get_width(parent), lv_obj_get_height(parent));
     lv_obj_set_pos(_mainPage, 0, 0);
     lv_obj_add_style(_mainPage, LV_OBJ_PART_MAIN, theme_get_style_bg());
     lv_obj_set_hidden(_mainPage, true);
     
     // Create a back button
     lv_obj_t* backBtn = lv_btn_create(_mainPage, NULL);
     lv_obj_set_size(backBtn, 80, 40);
     lv_obj_set_pos(backBtn, 10, 10);
     lv_obj_set_user_data(backBtn, this);
     lv_obj_set_event_cb(backBtn, _backBtnEventHandler);
     
     lv_obj_t* backBtnLabel = lv_label_create(backBtn, NULL);
     lv_label_set_text(backBtnLabel, "Back");
     
     // Create a title
     lv_obj_t* title = lv_label_create(_mainPage, NULL);
     lv_obj_add_style(title, LV_LABEL_PART_MAIN, theme_get_style_title());
     lv_label_set_text(title, "System Information");
     lv_obj_align(title, NULL, LV_ALIGN_IN_TOP_MID, 0, 15);
     
     // Create tabview
     _tabView = lv_tabview_create(_mainPage, NULL);
     lv_obj_set_pos(_tabView, 0, 60);
     lv_obj_set_size(_tabView, lv_obj_get_width(_mainPage), lv_obj_get_height(_mainPage) - 60);
     
     // Create tabs
     _tabSystem = lv_tabview_add_tab(_tabView, "System");
     _tabMemory = lv_tabview_add_tab(_tabView, "Memory");
     _tabNetwork = lv_tabview_add_tab(_tabView, "Network");
     _tabHardware = lv_tabview_add_tab(_tabView, "Hardware");
     
     // Initialize tabs with content
     _createSystemTab();
     _createMemoryTab();
     _createNetworkTab();
     _createHardwareTab();
     
     // Get references to system components
     extern PowerManager powerManager;
     _powerManager = &powerManager;
     
     _isInitialized = true;
     return true;
 }
 
 void SystemInfo::start() {
     if (!_isInitialized) {
         TDECK_LOG_E("SystemInfo not initialized");
         return;
     }
     
     if (_isActive) {
         return;
     }
     
     // Show the main page
     lv_obj_set_hidden(_mainPage, false);
     
     // Initial update of all information
     update();
     
     _isActive = true;
 }
 
 void SystemInfo::stop() {
     if (!_isActive) {
         return;
     }
     
     // Hide the main page
     if (_mainPage) {
         lv_obj_set_hidden(_mainPage, true);
     }
     
     _isActive = false;
 }
 
 void SystemInfo::update() {
     if (!_isActive) {
         return;
     }
     
     unsigned long currentTime = millis();
     
     // Update only at the defined interval
     if (currentTime - _lastUpdateTime >= SYSTEM_INFO_UPDATE_INTERVAL) {
         _updateSystemTab();
         _updateMemoryTab();
         _updateNetworkTab();
         _updateHardwareTab();
         
         _lastUpdateTime = currentTime;
     }
 }
 
 void SystemInfo::_createSystemTab() {
     int yPos = 10;
     const int yStep = 30;
     
     // Firmware information
     _lblVersion = _createLabelPair(_tabSystem, "Firmware Version:", yPos);
     yPos += yStep;
     
     _lblBuildDate = _createLabelPair(_tabSystem, "Build Date:", yPos);
     yPos += yStep;
     
     _lblUptime = _createLabelPair(_tabSystem, "Uptime:", yPos);
     yPos += yStep;
     
     // Battery information
     yPos += 10; // Add a little extra spacing
     _lblBatteryStatus = _createLabelPair(_tabSystem, "Battery Status:", yPos);
     yPos += yStep;
     
     _lblBatteryVoltage = _createLabelPair(_tabSystem, "Battery Voltage:", yPos);
     yPos += yStep;
     
     _lblBatteryPercent = _createLabelPair(_tabSystem, "Battery Level:", yPos);
     yPos += yStep;
     
     _lblChargingStatus = _createLabelPair(_tabSystem, "Charging:", yPos);
 }
 
 void SystemInfo::_createMemoryTab() {
     int yPos = 10;
     const int yStep = 35;
     
     // Heap memory information
     _lblFreeHeap = _createLabelPair(_tabMemory, "Free Heap:", yPos);
     yPos += yStep - 10;
     
     _barHeapUsage = _createUsageBar(_tabMemory, "Heap Usage:", yPos);
     yPos += yStep;
     
     _lblMinFreeHeap = _createLabelPair(_tabMemory, "Min Free Heap:", yPos);
     yPos += yStep - 10;
     
     _lblMaxAllocHeap = _createLabelPair(_tabMemory, "Max Alloc Heap:", yPos);
     yPos += yStep;
     
     // PSRAM information
     _lblPsramSize = _createLabelPair(_tabMemory, "PSRAM Size:", yPos);
     yPos += yStep - 10;
     
     _barPsramUsage = _createUsageBar(_tabMemory, "PSRAM Usage:", yPos);
     yPos += yStep;
     
     _lblFreePsram = _createLabelPair(_tabMemory, "Free PSRAM:", yPos);
     yPos += yStep;
     
     // Flash information
     _lblFlashSize = _createLabelPair(_tabMemory, "Flash Size:", yPos);
     yPos += yStep - 10;
     
     _barFlashUsage = _createUsageBar(_tabMemory, "Flash Usage:", yPos);
     yPos += yStep;
     
     _lblFlashUsed = _createLabelPair(_tabMemory, "Flash Used:", yPos);
 }
 
 void SystemInfo::_createNetworkTab() {
     int yPos = 10;
     const int yStep = 30;
     
     // WiFi information
     _lblWifiStatus = _createLabelPair(_tabNetwork, "WiFi Status:", yPos);
     yPos += yStep;
     
     _lblWifiSsid = _createLabelPair(_tabNetwork, "WiFi SSID:", yPos);
     yPos += yStep;
     
     _lblWifiIp = _createLabelPair(_tabNetwork, "WiFi IP:", yPos);
     yPos += yStep;
     
     _lblWifiMac = _createLabelPair(_tabNetwork, "WiFi MAC:", yPos);
     yPos += yStep + 10;
     
     // Bluetooth information
     _lblBtStatus = _createLabelPair(_tabNetwork, "Bluetooth Status:", yPos);
     yPos += yStep;
     
     _lblBtName = _createLabelPair(_tabNetwork, "Bluetooth Name:", yPos);
     yPos += yStep + 10;
     
     // LoRa information
     _lblLoraStatus = _createLabelPair(_tabNetwork, "LoRa Status:", yPos);
     yPos += yStep;
     
     _lblLoraFreq = _createLabelPair(_tabNetwork, "LoRa Frequency:", yPos);
 }
 
 void SystemInfo::_createHardwareTab() {
     int yPos = 10;
     const int yStep = 30;
     
     // CPU information
     _lblCpuType = _createLabelPair(_tabHardware, "CPU Type:", yPos);
     yPos += yStep;
     
     _lblCpuFreq = _createLabelPair(_tabHardware, "CPU Frequency:", yPos);
     yPos += yStep;
     
     _lblCpuTemp = _createLabelPair(_tabHardware, "CPU Temperature:", yPos);
     yPos += yStep;
     
     _lblCpuCores = _createLabelPair(_tabHardware, "CPU Cores:", yPos);
     yPos += yStep + 10;
     
     // CPU Usage
     _barCpu0Usage = _createUsageBar(_tabHardware, "CPU Core 0 Usage:", yPos);
     yPos += yStep + 5;
     
     _barCpu1Usage = _createUsageBar(_tabHardware, "CPU Core 1 Usage:", yPos);
     yPos += yStep + 10;
     
     // Other hardware
     _lblDisplayInfo = _createLabelPair(_tabHardware, "Display:", yPos);
     yPos += yStep;
     
     _lblSdCardInfo = _createLabelPair(_tabHardware, "SD Card:", yPos);
 }
 
 void SystemInfo::_updateSystemTab() {
     // Update firmware information
     lv_label_set_text(_lblVersion, TDECK_FIRMWARE_NAME " v" TDECK_FIRMWARE_VERSION);
     lv_label_set_text(_lblBuildDate, __DATE__ " " __TIME__);
     
     // Update uptime
     char uptimeStr[32];
     _getUptimeString(uptimeStr, sizeof(uptimeStr));
     lv_label_set_text(_lblUptime, uptimeStr);
     
     // Update battery information
     if (_powerManager) {
         float voltage = _powerManager->getBatteryVoltage();
         int percentage = _powerManager->getBatteryPercentage();
         bool isCharging = _powerManager->isCharging();
         
         // Battery status
         if (percentage <= TDECK_BATTERY_CRITICAL_THRESHOLD) {
             lv_label_set_text(_lblBatteryStatus, "Critical");
         } else if (percentage <= TDECK_BATTERY_LOW_THRESHOLD) {
             lv_label_set_text(_lblBatteryStatus, "Low");
         } else {
             lv_label_set_text(_lblBatteryStatus, "Normal");
         }
         
         // Format voltage and percentage
         char voltageStr[16];
         snprintf(voltageStr, sizeof(voltageStr), "%.2f V", voltage);
         lv_label_set_text(_lblBatteryVoltage, voltageStr);
         
         char percentStr[16];
         snprintf(percentStr, sizeof(percentStr), "%d%%", percentage);
         lv_label_set_text(_lblBatteryPercent, percentStr);
         
         // Charging status
         lv_label_set_text(_lblChargingStatus, isCharging ? "Yes" : "No");
     } else {
         lv_label_set_text(_lblBatteryStatus, "Unknown");
         lv_label_set_text(_lblBatteryVoltage, "N/A");
         lv_label_set_text(_lblBatteryPercent, "N/A");
         lv_label_set_text(_lblChargingStatus, "Unknown");
     }
 }
 
 void SystemInfo::_updateMemoryTab() {
     // Get memory information
     uint32_t freeHeap = esp_get_free_heap_size();
     uint32_t totalHeap = freeHeap + esp_get_minimum_free_heap_size();
     uint32_t minFreeHeap = esp_get_minimum_free_heap_size();
     uint32_t maxAllocHeap = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
     
     // Format heap information
     char buffer[32];
     
     _formatByteSize(freeHeap, buffer, sizeof(buffer));
     lv_label_set_text(_lblFreeHeap, buffer);
     
     _formatByteSize(minFreeHeap, buffer, sizeof(buffer));
     lv_label_set_text(_lblMinFreeHeap, buffer);
     
     _formatByteSize(maxAllocHeap, buffer, sizeof(buffer));
     lv_label_set_text(_lblMaxAllocHeap, buffer);
     
     // Update heap usage bar
     int heapUsagePercent = 100 - ((freeHeap * 100) / totalHeap);
     lv_bar_set_value(_barHeapUsage, heapUsagePercent, LV_ANIM_OFF);
     
     // PSRAM information
     size_t psramSize = 0;
     size_t freePsram = 0;
     
     if (esp_psram_get_size() > 0) {
         psramSize = esp_psram_get_size();
         freePsram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
         
         _formatByteSize(psramSize, buffer, sizeof(buffer));
         lv_label_set_text(_lblPsramSize, buffer);
         
         _formatByteSize(freePsram, buffer, sizeof(buffer));
         lv_label_set_text(_lblFreePsram, buffer);
         
         // Update PSRAM usage bar
         int psramUsagePercent = 100 - ((freePsram * 100) / psramSize);
         lv_bar_set_value(_barPsramUsage, psramUsagePercent, LV_ANIM_OFF);
     } else {
         lv_label_set_text(_lblPsramSize, "Not available");
         lv_label_set_text(_lblFreePsram, "N/A");
         lv_bar_set_value(_barPsramUsage, 0, LV_ANIM_OFF);
     }
     
     // Flash information
     size_t flashSize = 0;
     if (esp_flash_get_size(NULL, &flashSize) == ESP_OK) {
         _formatByteSize(flashSize, buffer, sizeof(buffer));
         lv_label_set_text(_lblFlashSize, buffer);
         
         // For flash usage, we can only give an estimate
         // This should be replaced with actual filesystem usage when available
         size_t flashUsed = flashSize / 3; // Example estimate
         _formatByteSize(flashUsed, buffer, sizeof(buffer));
         lv_label_set_text(_lblFlashUsed, buffer);
         
         // Update flash usage bar
         int flashUsagePercent = (flashUsed * 100) / flashSize;
         lv_bar_set_value(_barFlashUsage, flashUsagePercent, LV_ANIM_OFF);
     } else {
         lv_label_set_text(_lblFlashSize, "Unknown");
         lv_label_set_text(_lblFlashUsed, "Unknown");
         lv_bar_set_value(_barFlashUsage, 0, LV_ANIM_OFF);
     }
 }
 
 void SystemInfo::_updateNetworkTab() {
     extern WiFiManager wifiManager;
     extern BluetoothManager btManager;
     extern LoRaManager loraManager;
     
     // WiFi information
     if (TDECK_FEATURE_WIFI) {
         wifi_mode_t mode;
         esp_wifi_get_mode(&mode);
         
         if (mode == WIFI_MODE_NULL) {
             lv_label_set_text(_lblWifiStatus, "Disabled");
             lv_label_set_text(_lblWifiSsid, "N/A");
             lv_label_set_text(_lblWifiIp, "N/A");
             lv_label_set_text(_lblWifiMac, "N/A");
         } else if (mode == WIFI_MODE_STA) {
             if (wifiManager.isConnected()) {
                 lv_label_set_text(_lblWifiStatus, "Connected (STA)");
                 lv_label_set_text(_lblWifiSsid, wifiManager.getSSID().c_str());
                 lv_label_set_text(_lblWifiIp, wifiManager.getIP().c_str());
             } else {
                 lv_label_set_text(_lblWifiStatus, "Disconnected (STA)");
                 lv_label_set_text(_lblWifiSsid, "Not connected");
                 lv_label_set_text(_lblWifiIp, "Not assigned");
             }
             
             lv_label_set_text(_lblWifiMac, wifiManager.getMac().c_str());
         } else if (mode == WIFI_MODE_AP) {
             lv_label_set_text(_lblWifiStatus, "Active (AP)");
             lv_label_set_text(_lblWifiSsid, TDECK_WIFI_AP_SSID);
             lv_label_set_text(_lblWifiIp, wifiManager.getIP().c_str());
             lv_label_set_text(_lblWifiMac, wifiManager.getMac().c_str());
         } else if (mode == WIFI_MODE_APSTA) {
             lv_label_set_text(_lblWifiStatus, "Active (AP+STA)");
             lv_label_set_text(_lblWifiSsid, wifiManager.getSSID().c_str());
             lv_label_set_text(_lblWifiIp, wifiManager.getIP().c_str());
             lv_label_set_text(_lblWifiMac, wifiManager.getMac().c_str());
         }
     } else {
         lv_label_set_text(_lblWifiStatus, "Disabled (Feature)");
         lv_label_set_text(_lblWifiSsid, "N/A");
         lv_label_set_text(_lblWifiIp, "N/A");
         lv_label_set_text(_lblWifiMac, "N/A");
     }
     
     // Bluetooth information
     if (TDECK_FEATURE_BLUETOOTH) {
         if (btManager.isEnabled()) {
             lv_label_set_text(_lblBtStatus, "Enabled");
             lv_label_set_text(_lblBtName, TDECK_BT_DEVICE_NAME);
         } else {
             lv_label_set_text(_lblBtStatus, "Disabled");
             lv_label_set_text(_lblBtName, "N/A");
         }
     } else {
         lv_label_set_text(_lblBtStatus, "Disabled (Feature)");
         lv_label_set_text(_lblBtName, "N/A");
     }
     
     // LoRa information
     if (TDECK_FEATURE_LORA) {
         if (loraManager.isInitialized()) {
             lv_label_set_text(_lblLoraStatus, "Initialized");
             
             char freqStr[16];
             snprintf(freqStr, sizeof(freqStr), "%.1f MHz", TDECK_LORA_FREQUENCY / 1E6);
             lv_label_set_text(_lblLoraFreq, freqStr);
         } else {
             lv_label_set_text(_lblLoraStatus, "Not Initialized");
             lv_label_set_text(_lblLoraFreq, "N/A");
         }
     } else {
         lv_label_set_text(_lblLoraStatus, "Disabled (Feature)");
         lv_label_set_text(_lblLoraFreq, "N/A");
     }
 }
 
 lv_obj_t* SystemInfo::_createLabelPair(lv_obj_t* parent, const char* title, int yPos) {
     // Create title label
     lv_obj_t* titleLabel = lv_label_create(parent, NULL);
     lv_label_set_text(titleLabel, title);
     lv_obj_set_pos(titleLabel, 10, yPos);
     lv_obj_add_style(titleLabel, LV_LABEL_PART_MAIN, theme_get_style_label());
     
     // Create value label
     lv_obj_t* valueLabel = lv_label_create(parent, NULL);
     lv_label_set_text(valueLabel, "");
     lv_obj_set_pos(valueLabel, 160, yPos);
     lv_obj_add_style(valueLabel, LV_LABEL_PART_MAIN, theme_get_style_value());
     
     return valueLabel;
 }
 
 lv_obj_t* SystemInfo::_createUsageBar(lv_obj_t* parent, const char* title, int yPos) {
     // Create title label
     lv_obj_t* titleLabel = lv_label_create(parent, NULL);
     lv_label_set_text(titleLabel, title);
     lv_obj_set_pos(titleLabel, 10, yPos);
     lv_obj_add_style(titleLabel, LV_LABEL_PART_MAIN, theme_get_style_label());
     
     // Create progress bar
     lv_obj_t* bar = lv_bar_create(parent, NULL);
     lv_obj_set_pos(bar, 10, yPos + 20);
     lv_obj_set_size(bar, lv_obj_get_width(parent) - 20, 10);
     lv_bar_set_range(bar, 0, 100);
     lv_bar_set_value(bar, 0, LV_ANIM_OFF);
     
     return bar;
 }
 
 void SystemInfo::_getUptimeString(char* buffer, size_t size) {
     unsigned long uptime = millis() / 1000; // Convert to seconds
     unsigned long days = uptime / 86400;
     uptime %= 86400;
     unsigned long hours = uptime / 3600;
     uptime %= 3600;
     unsigned long minutes = uptime / 60;
     unsigned long seconds = uptime % 60;
     
     if (days > 0) {
         snprintf(buffer, size, "%lud %luh %lum %lus", days, hours, minutes, seconds);
     } else if (hours > 0) {
         snprintf(buffer, size, "%luh %lum %lus", hours, minutes, seconds);
     } else if (minutes > 0) {
         snprintf(buffer, size, "%lum %lus", minutes, seconds);
     } else {
         snprintf(buffer, size, "%lus", seconds);
     }
 }
 
 void SystemInfo::_formatByteSize(uint32_t bytes, char* buffer, size_t size) {
     const char* units[] = {"B", "KB", "MB", "GB", "TB"};
     int unitIndex = 0;
     float size_f = bytes;
     
     while (size_f >= 1024.0f && unitIndex < 4) {
         size_f /= 1024.0f;
         unitIndex++;
     }
     
     if (unitIndex == 0) {
         snprintf(buffer, size, "%u %s", (unsigned int)size_f, units[unitIndex]);
     } else {
         snprintf(buffer, size, "%.2f %s", size_f, units[unitIndex]);
     }
 }
 
 void SystemInfo::_updateHardwareTab() {
     // CPU information
     esp_chip_info_t chipInfo;
     esp_chip_info(&chipInfo);
     
     // CPU type
     const char* chipModel;
     switch (chipInfo.model) {
         case CHIP_ESP32:
             chipModel = "ESP32";
             break;
         case CHIP_ESP32S2:
             chipModel = "ESP32-S2";
             break;
         case CHIP_ESP32S3:
             chipModel = "ESP32-S3";
             break;
         case CHIP_ESP32C3:
             chipModel = "ESP32-C3";
             break;
         default:
             chipModel = "Unknown";
     }
     
     char cpuTypeStr[64];
     snprintf(cpuTypeStr, sizeof(cpuTypeStr), "%s Rev %d", chipModel, chipInfo.revision);
     lv_label_set_text(_lblCpuType, cpuTypeStr);
     
     // CPU frequency
     uint32_t cpuFreq = esp_clk_cpu_freq() / 1000000;
     char cpuFreqStr[16];
     snprintf(cpuFreqStr, sizeof(cpuFreqStr), "%u MHz", cpuFreq);
     lv_label_set_text(_lblCpuFreq, cpuFreqStr);
     
     // CPU temperature - ESP32-S3 doesn't have a built-in temperature sensor
     // This is a placeholder
     lv_label_set_text(_lblCpuTemp, "N/A");
     
     // CPU cores
     char coresStr[16];
     snprintf(coresStr, sizeof(coresStr), "%d", chipInfo.cores);
     lv_label_set_text(_lblCpuCores, coresStr);
     
     // CPU usage - This is a placeholder
     // Proper CPU usage monitoring requires additional code
     lv_bar_set_value(_barCpu0Usage, 50, LV_ANIM_OFF); // Example value
     lv_bar_set_value(_barCpu1Usage, 30, LV_ANIM_OFF); // Example value
     
     // Display information
     extern Display display;
     if (display.isInitialized()) {
         char displayStr[32];
         snprintf(displayStr, sizeof(displayStr), "%dx%d IPS Touch Display", 
                  TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT);
         lv_label_set_text(_lblDisplayInfo, displayStr);
     } else {
         lv_label_set_text(_lblDisplayInfo, "Not initialized");
     }
     
     // SD card information
     extern SDCard sdCard;
     if (TDECK_FEATURE_SD_CARD && sdCard.isInitialized()) {
         char sdCardStr[64];
         uint64_t totalBytes = sdCard.getTotalBytes();
         uint64_t usedBytes = sdCard.getUsedBytes();
         
         char totalSizeStr[16];
         char usedSizeStr[16];
         _formatByteSize(totalBytes, totalSizeStr, sizeof(totalSizeStr));
         _formatByteSize(usedBytes, usedSizeStr, sizeof(usedSizeStr));
         
         snprintf(sdCardStr, sizeof(sdCardStr), "%s used of %s", usedSizeStr, totalSizeStr);
         lv_label_set_text(_lblSdCardInfo, sdCardStr);
     } else {
         lv_label_set_text(_lblSdCardInfo, "Not mounted");
     }