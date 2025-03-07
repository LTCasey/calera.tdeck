/**
 * @file ota.h
 * @brief OTA (Over-The-Air) update system for T-Deck UI Firmware
 * 
 * This class handles firmware updates over WiFi, providing methods to check for
 * updates, download and apply updates, and manage update settings.
 */

 #ifndef TDECK_OTA_H
 #define TDECK_OTA_H
 
 #include <Arduino.h>
 #include <Update.h>
 #include <WiFi.h>
 #include <HTTPClient.h>
 #include <ArduinoJson.h>
 #include "../config.h"
 #include "config_storage.h"
 
 // OTA update status
 enum OTAUpdateStatus {
     OTA_IDLE,                // No update in progress
     OTA_CHECKING,            // Checking for updates
     OTA_UPDATE_AVAILABLE,    // Update available but not started
     OTA_DOWNLOADING,         // Downloading update
     OTA_APPLYING,            // Applying update
     OTA_UPDATE_COMPLETE,     // Update successfully completed
     OTA_ERROR                // Error during update process
 };
 
 // OTA error codes
 enum OTAErrorCode {
     OTA_ERROR_NONE,                  // No error
     OTA_ERROR_CONNECTION,            // Connection to server failed
     OTA_ERROR_SERVER_RESPONSE,       // Invalid response from server
     OTA_ERROR_DOWNLOAD_FAILED,       // Failed to download update
     OTA_ERROR_VERIFICATION_FAILED,   // Update verification failed
     OTA_ERROR_FLASH_FAILED,          // Failed to flash update
     OTA_ERROR_INSUFFICIENT_SPACE,    // Not enough space for update
     OTA_ERROR_BATTERY_LOW,           // Battery too low for update
     OTA_ERROR_ABORTED                // Update aborted by user
 };
 
 class OTAManager {
 public:
     /**
      * @brief Construct a new OTA Manager object
      */
     OTAManager();
 
     /**
      * @brief Initialize the OTA update system
      * 
      * @return true if initialization successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Update function to be called periodically
      */
     void update();
 
     /**
      * @brief Check for firmware updates
      * 
      * @param url Optional URL to check for updates. If NULL, the default URL will be used.
      * @return true if update check initiated
      * @return false if failed to initiate check
      */
     bool checkForUpdates(const char* url = NULL);
 
     /**
      * @brief Start downloading and applying an update
      * 
      * @param url URL of the update file
      * @param expectedMD5 Expected MD5 hash of the firmware file (optional)
      * @param version Version of the firmware (optional)
      * @return true if update process started
      * @return false if failed to start update
      */
     bool startUpdate(const char* url, const char* expectedMD5 = NULL, const char* version = NULL);
 
     /**
      * @brief Abort current update process
      * 
      * @return true if update aborted
      * @return false if no update in progress or cannot abort
      */
     bool abortUpdate();
 
     /**
      * @brief Get current update status
      * 
      * @return OTAUpdateStatus Current status
      */
     OTAUpdateStatus getStatus() const;
 
     /**
      * @brief Get last error code (if status is OTA_ERROR)
      * 
      * @return OTAErrorCode Error code
      */
     OTAErrorCode getErrorCode() const;
 
     /**
      * @brief Get error message for current error
      * 
      * @return const char* Error message
      */
     const char* getErrorMessage() const;
 
     /**
      * @brief Get download progress (0-100)
      * 
      * @return int Progress percentage
      */
     int getDownloadProgress() const;
 
     /**
      * @brief Set callback for update progress
      * 
      * @param callback Function to call with progress updates
      */
     void setProgressCallback(void (*callback)(int progress, OTAUpdateStatus status));
 
     /**
      * @brief Set callback for update completion
      * 
      * @param callback Function to call when update completes or fails
      */
     void setCompletionCallback(void (*callback)(bool success, OTAErrorCode error));
 
     /**
      * @brief Enable or disable automatic update checks
      * 
      * @param enable True to enable, false to disable
      */
     void enableAutoCheck(bool enable);
 
     /**
      * @brief Set update check interval (in hours)
      * 
      * @param intervalHours Hours between update checks
      */
     void setCheckInterval(int intervalHours);
 
     /**
      * @brief Set update server URL
      * 
      * @param url Server URL
      */
     void setUpdateServerUrl(const char* url);
 
 private:
     // Status tracking
     OTAUpdateStatus _status;
     OTAErrorCode _errorCode;
     String _errorMessage;
     
     // Update parameters
     String _updateUrl;
     String _defaultUpdateUrl;
     String _expectedMD5;
     String _updateVersion;
     int _downloadProgress;
     size_t _updateSize;
     size_t _writtenSize;
     
     // Auto update settings
     bool _autoCheckEnabled;
     int _checkIntervalHours;
     unsigned long _lastCheckTime;
     
     // Callbacks
     void (*_progressCallback)(int progress, OTAUpdateStatus status);
     void (*_completionCallback)(bool success, OTAErrorCode error);
     
     // Update task handle and status
     TaskHandle_t _updateTaskHandle;
     bool _updateTaskRunning;
     
     // Helper methods
     bool beginUpdate();
     void processDownload();
     void finishUpdate(bool success, OTAErrorCode error = OTA_ERROR_NONE);
     bool checkBatteryLevel();
     void saveUpdateSettings();
     void loadUpdateSettings();
     
     // Update task function
     static void updateTask(void* parameter);
 };
 
 extern OTAManager otaManager;
 
 #endif // TDECK_OTA_H