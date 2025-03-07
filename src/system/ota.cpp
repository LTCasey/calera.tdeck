/**
 * @file ota.cpp
 * @brief Implementation of OTA update system for T-Deck UI Firmware
 */

 #include "ota.h"
 #include "battery.h"
 
 // Create global instance
 OTAManager otaManager;
 
 // Default update server URL
 #define DEFAULT_UPDATE_SERVER "https://firmware.example.com/tdeck"
 
 // Minimum battery percentage required for OTA update
 #define OTA_MIN_BATTERY_PERCENTAGE 30
 
 // Update task stack size and priority
 #define OTA_TASK_STACK_SIZE 8192
 #define OTA_TASK_PRIORITY 1
 
 // OTA configuration file
 #define OTA_CONFIG_FILE "/config/ota.json"
 
 // Timeout for HTTP connections (milliseconds)
 #define HTTP_TIMEOUT 10000
 
 // Update check interval (hours)
 #define DEFAULT_CHECK_INTERVAL 24
 
 OTAManager::OTAManager() :
     _status(OTA_IDLE),
     _errorCode(OTA_ERROR_NONE),
     _errorMessage(""),
     _updateUrl(""),
     _defaultUpdateUrl(DEFAULT_UPDATE_SERVER),
     _expectedMD5(""),
     _updateVersion(""),
     _downloadProgress(0),
     _updateSize(0),
     _writtenSize(0),
     _autoCheckEnabled(true),
     _checkIntervalHours(DEFAULT_CHECK_INTERVAL),
     _lastCheckTime(0),
     _progressCallback(nullptr),
     _completionCallback(nullptr),
     _updateTaskHandle(nullptr),
     _updateTaskRunning(false) {
 }
 
 bool OTAManager::init() {
     TDECK_LOG_I("Initializing OTA update system");
     
     // Load update settings
     loadUpdateSettings();
     
     // Get time since last update check
     unsigned long currentTime = millis() / 1000; // Convert to seconds
     unsigned long timeSinceLastCheck = currentTime - _lastCheckTime;
     
     // Convert check interval to seconds
     unsigned long checkIntervalSeconds = _checkIntervalHours * 3600;
     
     TDECK_LOG_I("Auto check: %s, Last check: %lu seconds ago, Interval: %d hours", 
                 _autoCheckEnabled ? "enabled" : "disabled", 
                 timeSinceLastCheck, 
                 _checkIntervalHours);
     
     // Schedule automatic check if enabled and interval has passed
     if (_autoCheckEnabled && timeSinceLastCheck >= checkIntervalSeconds) {
         TDECK_LOG_I("Scheduling automatic update check");
         // We don't check immediately but set a flag to check soon
         // This prevents blocking the initialization process
     }
     
     return true;
 }
 
 void OTAManager::update() {
     // Check if we need to auto-check for updates
     if (_status == OTA_IDLE && _autoCheckEnabled) {
         unsigned long currentTime = millis() / 1000; // Convert to seconds
         unsigned long timeSinceLastCheck = currentTime - _lastCheckTime;
         unsigned long checkIntervalSeconds = _checkIntervalHours * 3600;
         
         if (timeSinceLastCheck >= checkIntervalSeconds) {
             // Time to check for updates
             TDECK_LOG_I("Auto checking for updates");
             checkForUpdates();
         }
     }
     
     // Handle update status changes
     if (_status == OTA_DOWNLOADING && !_updateTaskRunning) {
         // If we're supposed to be downloading but the task isn't running,
         // something went wrong - reset the status
         TDECK_LOG_E("Update task not running but status is DOWNLOADING");
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_DOWNLOAD_FAILED;
         _errorMessage = "Download task stopped unexpectedly";
         
         if (_completionCallback) {
             _completionCallback(false, _errorCode);
         }
     }
 }
 
 bool OTAManager::checkForUpdates(const char* url) {
     // Cannot check if already updating
     if (_status != OTA_IDLE && _status != OTA_ERROR && _status != OTA_UPDATE_COMPLETE) {
         TDECK_LOG_E("Cannot check for updates while update in progress");
         return false;
     }
     
     // Cannot check if WiFi is not connected
     if (WiFi.status() != WL_CONNECTED) {
         TDECK_LOG_E("Cannot check for updates, WiFi not connected");
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_CONNECTION;
         _errorMessage = "WiFi not connected";
         return false;
     }
     
     TDECK_LOG_I("Checking for updates");
     
     // Update status
     _status = OTA_CHECKING;
     _errorCode = OTA_ERROR_NONE;
     _errorMessage = "";
     
     if (_progressCallback) {
         _progressCallback(0, _status);
     }
     
     // Set URL to use
     String checkUrl;
     if (url != NULL) {
         checkUrl = url;
     } else {
         checkUrl = _defaultUpdateUrl;
     }
     
     // Append '/version.json' if needed
     if (!checkUrl.endsWith("/version.json")) {
         if (!checkUrl.endsWith("/")) {
             checkUrl += "/";
         }
         checkUrl += "version.json";
     }
     
     TDECK_LOG_I("Checking version at URL: %s", checkUrl.c_str());
     
     // Check version
     HTTPClient http;
     http.setTimeout(HTTP_TIMEOUT);
     http.begin(checkUrl);
     
     int httpCode = http.GET();
     if (httpCode != HTTP_CODE_OK) {
         TDECK_LOG_E("HTTP GET failed, error: %d", httpCode);
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_CONNECTION;
         _errorMessage = "Server connection failed: " + String(httpCode);
         
         if (_completionCallback) {
             _completionCallback(false, _errorCode);
         }
         
         http.end();
         return false;
     }
     
     // Parse version JSON
     DynamicJsonDocument doc(1024);
     DeserializationError error = deserializeJson(doc, http.getString());
     http.end();
     
     if (error) {
         TDECK_LOG_E("Failed to parse version JSON: %s", error.c_str());
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_SERVER_RESPONSE;
         _errorMessage = "Invalid server response: " + String(error.c_str());
         
         if (_completionCallback) {
             _completionCallback(false, _errorCode);
         }
         
         return false;
     }
     
     // Check if update is available
     const char* serverVersion = doc["version"];
     if (!serverVersion) {
         TDECK_LOG_E("Version information missing in response");
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_SERVER_RESPONSE;
         _errorMessage = "Version information missing";
         
         if (_completionCallback) {
             _completionCallback(false, _errorCode);
         }
         
         return false;
     }
     
     TDECK_LOG_I("Server version: %s, Current version: %s", serverVersion, TDECK_FIRMWARE_VERSION);
     
     // Compare versions (simple string compare for now)
     if (strcmp(serverVersion, TDECK_FIRMWARE_VERSION) <= 0) {
         // No update needed
         TDECK_LOG_I("No update available");
         _status = OTA_IDLE;
         
         // Update last check time
         _lastCheckTime = millis() / 1000;
         saveUpdateSettings();
         
         if (_completionCallback) {
             _completionCallback(true, OTA_ERROR_NONE);
         }
         
         return true;
     }
     
     // Update is available
     TDECK_LOG_I("Update available: %s", serverVersion);
     _status = OTA_UPDATE_AVAILABLE;
     _updateVersion = serverVersion;
     
     // Get update URL
     const char* updateUrl = doc["url"];
     if (!updateUrl) {
         // If URL not provided, construct it
         if (url != NULL) {
             _updateUrl = url;
         } else {
             _updateUrl = _defaultUpdateUrl;
         }
         
         // Remove version.json and append firmware name
         if (_updateUrl.endsWith("/version.json")) {
             _updateUrl = _updateUrl.substring(0, _updateUrl.length() - 12);
         }
         
         if (!_updateUrl.endsWith("/")) {
             _updateUrl += "/";
         }
         
         _updateUrl += "firmware.bin";
     } else {
         _updateUrl = updateUrl;
     }
     
     // Get expected MD5 if available
     const char* md5 = doc["md5"];
     if (md5) {
         _expectedMD5 = md5;
     } else {
         _expectedMD5 = "";
     }
     
     // Update last check time
     _lastCheckTime = millis() / 1000;
     saveUpdateSettings();
     
     if (_progressCallback) {
         _progressCallback(0, _status);
     }
     
     return true;
 }
 
 bool OTAManager::startUpdate(const char* url, const char* expectedMD5, const char* version) {
     // Cannot start update if already updating
     if (_status == OTA_DOWNLOADING || _status == OTA_APPLYING) {
         TDECK_LOG_E("Update already in progress");
         return false;
     }
     
     // Cannot update if WiFi is not connected
     if (WiFi.status() != WL_CONNECTED) {
         TDECK_LOG_E("Cannot start update, WiFi not connected");
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_CONNECTION;
         _errorMessage = "WiFi not connected";
         return false;
     }
     
     // Check battery level
     if (!checkBatteryLevel()) {
         TDECK_LOG_E("Battery level too low for update");
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_BATTERY_LOW;
         _errorMessage = "Battery level too low for update";
         
         if (_completionCallback) {
             _completionCallback(false, _errorCode);
         }
         
         return false;
     }
     
     // Set update parameters
     if (url != NULL) {
         _updateUrl = url;
     }
     
     if (expectedMD5 != NULL) {
         _expectedMD5 = expectedMD5;
     }
     
     if (version != NULL) {
         _updateVersion = version;
     }
     
     TDECK_LOG_I("Starting update from URL: %s", _updateUrl.c_str());
     
     // Reset progress
     _downloadProgress = 0;
     _updateSize = 0;
     _writtenSize = 0;
     
     // Update status
     _status = OTA_DOWNLOADING;
     _errorCode = OTA_ERROR_NONE;
     _errorMessage = "";
     
     if (_progressCallback) {
         _progressCallback(_downloadProgress, _status);
     }
     
     // Create update task
     _updateTaskRunning = true;
     BaseType_t result = xTaskCreatePinnedToCore(
         updateTask,                // Task function
         "OTA_Update_Task",         // Name
         OTA_TASK_STACK_SIZE,       // Stack size
         this,                      // Parameter
         OTA_TASK_PRIORITY,         // Priority
         &_updateTaskHandle,        // Task handle
         0                          // Core
     );
     
     if (result != pdPASS) {
         TDECK_LOG_E("Failed to create update task");
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_DOWNLOAD_FAILED;
         _errorMessage = "Failed to create update task";
         _updateTaskRunning = false;
         
         if (_completionCallback) {
             _completionCallback(false, _errorCode);
         }
         
         return false;
     }
     
     return true;
 }
 
 bool OTAManager::abortUpdate() {
     // Can only abort if update is in progress
     if (_status != OTA_DOWNLOADING && _status != OTA_APPLYING) {
         TDECK_LOG_E("No update in progress to abort");
         return false;
     }
     
     TDECK_LOG_I("Aborting update");
     
     // If update task is running, delete it
     if (_updateTaskRunning && _updateTaskHandle != nullptr) {
         vTaskDelete(_updateTaskHandle);
         _updateTaskHandle = nullptr;
         _updateTaskRunning = false;
     }
     
     // End update
     Update.abort();
     
     // Update status
     _status = OTA_ERROR;
     _errorCode = OTA_ERROR_ABORTED;
     _errorMessage = "Update aborted by user";
     
     if (_completionCallback) {
         _completionCallback(false, _errorCode);
     }
     
     return true;
 }
 
 OTAUpdateStatus OTAManager::getStatus() const {
     return _status;
 }
 
 OTAErrorCode OTAManager::getErrorCode() const {
     return _errorCode;
 }
 
 const char* OTAManager::getErrorMessage() const {
     return _errorMessage.c_str();
 }
 
 int OTAManager::getDownloadProgress() const {
     return _downloadProgress;
 }
 
 void OTAManager::setProgressCallback(void (*callback)(int progress, OTAUpdateStatus status)) {
     _progressCallback = callback;
 }
 
 void OTAManager::setCompletionCallback(void (*callback)(bool success, OTAErrorCode error)) {
     _completionCallback = callback;
 }
 
 void OTAManager::enableAutoCheck(bool enable) {
     _autoCheckEnabled = enable;
     saveUpdateSettings();
 }
 
 void OTAManager::setCheckInterval(int intervalHours) {
     if (intervalHours > 0) {
         _checkIntervalHours = intervalHours;
         saveUpdateSettings();
     }
 }
 
 void OTAManager::setUpdateServerUrl(const char* url) {
     if (url != NULL) {
         _defaultUpdateUrl = url;
         saveUpdateSettings();
     }
 }
 
 bool OTAManager::beginUpdate() {
     // Cannot initialize update if already in progress
     if (Update.isRunning()) {
         TDECK_LOG_E("Update already in progress");
         return false;
     }
     
     // Calculate required space
     uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
     
     if (_updateSize > maxSketchSpace) {
         TDECK_LOG_E("Not enough space for update. Required: %u, Available: %u", _updateSize, maxSketchSpace);
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_INSUFFICIENT_SPACE;
         _errorMessage = "Insufficient space for update";
         return false;
     }
     
     TDECK_LOG_I("Beginning update. Size: %u bytes", _updateSize);
     
     // Initialize update
     if (!Update.begin(_updateSize)) {
         TDECK_LOG_E("Failed to begin update: %s", Update.errorString());
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_FLASH_FAILED;
         _errorMessage = "Failed to begin update: " + String(Update.errorString());
         return false;
     }
     
     // Set expected MD5 if provided
     if (_expectedMD5.length() > 0) {
         Update.setMD5(_expectedMD5.c_str());
         TDECK_LOG_I("Expected MD5: %s", _expectedMD5.c_str());
     }
     
     return true;
 }
 
 void OTAManager::processDownload() {
     TDECK_LOG_I("Starting firmware download from: %s", _updateUrl.c_str());
     
     // Create HTTP client
     HTTPClient http;
     http.setTimeout(HTTP_TIMEOUT);
     http.begin(_updateUrl);
     
     // Send HTTP GET request
     int httpCode = http.GET();
     
     // Check HTTP response
     if (httpCode != HTTP_CODE_OK) {
         TDECK_LOG_E("HTTP GET failed, error: %d", httpCode);
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_DOWNLOAD_FAILED;
         _errorMessage = "Download failed: HTTP " + String(httpCode);
         
         http.end();
         return;
     }
     
     // Get content length
     _updateSize = http.getSize();
     TDECK_LOG_I("Update size: %u bytes", _updateSize);
     
     if (_updateSize == 0) {
         TDECK_LOG_E("Invalid update size");
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_DOWNLOAD_FAILED;
         _errorMessage = "Invalid update size";
         
         http.end();
         return;
     }
     
     // Begin update process
     if (!beginUpdate()) {
         http.end();
         return;
     }
     
     // Get file stream
     WiFiClient* stream = http.getStreamPtr();
     
     // Read all data from server and write to flash
     _writtenSize = 0;
     uint8_t buff[1024] = {0};
     size_t bytesRead = 0;
     
     while (http.connected() && (_writtenSize < _updateSize)) {
         size_t available = stream->available();
         
         if (available) {
             // Read data from stream
             bytesRead = stream->readBytes(buff, min(available, sizeof(buff)));
             
             // Write data to update
             if (Update.write(buff, bytesRead) != bytesRead) {
                 TDECK_LOG_E("Write error: %s", Update.errorString());
                 _status = OTA_ERROR;
                 _errorCode = OTA_ERROR_FLASH_FAILED;
                 _errorMessage = "Flash write error: " + String(Update.errorString());
                 
                 http.end();
                 return;
             }
             
             // Update progress
             _writtenSize += bytesRead;
             _downloadProgress = (_writtenSize * 100) / _updateSize;
             
             if (_progressCallback) {
                 _progressCallback(_downloadProgress, _status);
             }
             
             // Log progress periodically
             if (_writtenSize % 51200 == 0) { // Log every 50KB
                 TDECK_LOG_I("Download progress: %d%% (%u/%u bytes)", 
                             _downloadProgress, _writtenSize, _updateSize);
             }
         }
         
         // Small delay to prevent watchdog issues
         delay(1);
     }
     
     // End HTTP connection
     http.end();
     
     // Verify download completed
     if (_writtenSize != _updateSize) {
         TDECK_LOG_E("Download incomplete. Received: %u, Expected: %u", _writtenSize, _updateSize);
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_DOWNLOAD_FAILED;
         _errorMessage = "Download incomplete";
         return;
     }
     
     TDECK_LOG_I("Download complete. Verifying...");
     
     // Update status
     _status = OTA_APPLYING;
     
     if (_progressCallback) {
         _progressCallback(100, _status);
     }
     
     // Finish update
     if (!Update.end(true)) {
         TDECK_LOG_E("Update verify failed: %s", Update.errorString());
         _status = OTA_ERROR;
         _errorCode = OTA_ERROR_VERIFICATION_FAILED;
         _errorMessage = "Verification failed: " + String(Update.errorString());
         return;
     }
     
     TDECK_LOG_I("Update verified successfully. Rebooting...");
     _status = OTA_UPDATE_COMPLETE;
     
     // Delay to allow status to propagate
     delay(1000);
     
     // Reboot
     ESP.restart();
 }
 
 void OTAManager::finishUpdate(bool success, OTAErrorCode error) {
     if (success) {
         _status = OTA_UPDATE_COMPLETE;
     } else {
         _status = OTA_ERROR;
         _errorCode = error;
     }
     
     if (_completionCallback) {
         _completionCallback(success, error);
     }
 }
 
 bool OTAManager::checkBatteryLevel() {
     // Check battery percentage
     int batteryPercentage = batteryManager.getPercentage();
     
     if (batteryPercentage < OTA_MIN_BATTERY_PERCENTAGE) {
         TDECK_LOG_W("Battery level too low for update: %d%%, required: %d%%", 
                     batteryPercentage, OTA_MIN_BATTERY_PERCENTAGE);
         return false;
     }
     
     // Check if device is charging
     bool isCharging = batteryManager.isCharging();
     
     // If battery is below 50% and not charging, warn but allow update
     if (batteryPercentage < 50 && !isCharging) {
         TDECK_LOG_W("Battery level low (%d%%) and not charging. Update may fail.", batteryPercentage);
     }
     
     return true;
 }
 
 void OTAManager::saveUpdateSettings() {
     // Create JSON document
     DynamicJsonDocument doc(512);
     
     // Fill settings
     doc["auto_check"] = _autoCheckEnabled;
     doc["check_interval_hours"] = _checkIntervalHours;
     doc["last_check_time"] = _lastCheckTime;
     doc["update_server"] = _defaultUpdateUrl;
     
     // Save to file
     if (!fsManager.saveJsonToFile(OTA_CONFIG_FILE, doc)) {
         TDECK_LOG_E("Failed to save OTA settings");
     }
 }
 
 void OTAManager::loadUpdateSettings() {
     // Create JSON document
     DynamicJsonDocument doc(512);
     
     // Load from file
     if (fsManager.loadJsonFromFile(OTA_CONFIG_FILE, doc)) {
         // Get settings
         _autoCheckEnabled = doc["auto_check"] | true;
         _checkIntervalHours = doc["check_interval_hours"] | DEFAULT_CHECK_INTERVAL;
         _lastCheckTime = doc["last_check_time"] | 0;
         
         const char* server = doc["update_server"];
         if (server && strlen(server) > 0) {
             _defaultUpdateUrl = server;
         }
         
         TDECK_LOG_I("Loaded OTA settings. Auto check: %s, Interval: %d hours", 
                     _autoCheckEnabled ? "enabled" : "disabled", _checkIntervalHours);
     } else {
         TDECK_LOG_I("No OTA settings found, using defaults");
         // Use defaults
         _autoCheckEnabled = true;
         _checkIntervalHours = DEFAULT_CHECK_INTERVAL;
         _lastCheckTime = 0;
         _defaultUpdateUrl = DEFAULT_UPDATE_SERVER;
         
         // Save default settings
         saveUpdateSettings();
     }
 }
 
 // Static update task function
 void OTAManager::updateTask(void* parameter) {
     OTAManager* otaManager = static_cast<OTAManager*>(parameter);
     
     // Process download
     otaManager->processDownload();
     
     // Mark task as completed
     otaManager->_updateTaskRunning = false;
     otaManager->_updateTaskHandle = nullptr;
     
     // Delete task
     vTaskDelete(NULL);
 }