/**
 * @file usb.cpp
 * @brief Implementation of USB communication for T-Deck
 */

 #include "usb.h"
 #include <ArduinoJson.h>
 #include <Update.h>
 #include <esp_ota_ops.h>
 #include <stdarg.h>
 #include "../hal/power.h"
 
 // External reference to the power manager
 extern PowerManager powerManager;
 
 // Command response strings
 #define USB_RESP_OK "OK:"
 #define USB_RESP_ERROR "ERROR:"
 #define USB_RESP_DATA "DATA:"
 #define USB_RESP_INFO "INFO:"
 #define USB_CMD_SEP " "
 
 // Connection check interval in ms
 #define USB_CONN_CHECK_INTERVAL 1000
 
 // Timeout for data transfers (ms)
 #define USB_TRANSFER_TIMEOUT 10000
 
 // External reference to the filesystem manager defined in main.cpp
 
 /**
  * @brief Construct a new USB Manager object
  */
 USBManager::USBManager() 
     : _mode(USBMode::CDC_CONSOLE),
       _cmdBufferIndex(0),
       _transferActive(false),
       _connected(false),
       _lastConnCheckTime(0) {
     // Initialize command buffer
     memset(_cmdBuffer, 0, USB_CMD_BUFFER_SIZE);
 }
 
 /**
  * @brief Initialize the USB subsystem
  * 
  * @return true if initialization was successful
  * @return false if initialization failed
  */
 bool USBManager::init() {
     TDECK_LOG_I("Initializing USB subsystem");
     
     // USB is already initialized by Arduino's USB.begin() in main.cpp
     // We just need to set up our CDC serial interface
     _usbCDC.begin(115200);
     
     // USB subsystem is initialized
     
     _mode = USBMode::CDC_CONSOLE;
     _connected = false;
     
     // Send initialization message once USB is connected
     sendFormatted("%s USB interface initialized. Firmware: %s v%s\r\n", 
                   USB_RESP_INFO, TDECK_FIRMWARE_NAME, TDECK_FIRMWARE_VERSION);
     
     TDECK_LOG_I("USB subsystem initialized");
     return true;
 }
 
 /**
  * @brief Update USB state and process data
  * 
  * This function should be called periodically from the main loop or system task
  */
 void USBManager::update() {
     // Check if USB is connected periodically
     unsigned long now = millis();
     if (now - _lastConnCheckTime > USB_CONN_CHECK_INTERVAL) {
         _lastConnCheckTime = now;
         bool wasConnected = _connected;
         _connected = _usbCDC.connected();
         
         // If connection state changed
         if (wasConnected != _connected) {
             if (_connected) {
                 TDECK_LOG_I("USB connected");
                 // Reset state
                 _resetCmdBuffer();
                 _transferActive = false;
                 
                 // Send welcome message
                 sendFormatted("%s T-Deck connected. Firmware: %s v%s\r\n", 
                               USB_RESP_INFO, TDECK_FIRMWARE_NAME, TDECK_FIRMWARE_VERSION);
             } else {
                 TDECK_LOG_I("USB disconnected");
                 
                 // If a transfer was active, close the file
                 if (_transferActive && _transferFile) {
                     _transferFile.close();
                     _transferActive = false;
                 }
             }
         }
     }
     
     // Process received data
     if (_connected && _usbCDC.available()) {
         // Read data from USB
         while (_usbCDC.available()) {
             char c = _usbCDC.read();
             
             // If in data transfer mode, handle binary data
             if (_transferActive) {
                 // Write to file directly
                 if (_transferFile) {
                     _transferFile.write(c);
                     _transferredBytes++;
                     
                     // Check if transfer is complete
                     if (_transferredBytes >= _totalTransferSize) {
                         _transferFile.close();
                         _transferActive = false;
                         _sendAck(true, "Transfer complete");
                         TDECK_LOG_I("File transfer complete: %s (%d bytes)", 
                                     _currentTransferPath.c_str(), _transferredBytes);
                     }
                 } else {
                     // Something went wrong with the file
                     _transferActive = false;
                     _sendAck(false, "File transfer error");
                     TDECK_LOG_E("File transfer error: Invalid file handle");
                 }
             } else {
                 // Handle command mode (line-based)
                 if (c == '\n' || c == '\r') {
                     // Process command if buffer contains data
                     if (_cmdBufferIndex > 0) {
                         _cmdBuffer[_cmdBufferIndex] = '\0';
                         _processCommand(_cmdBuffer);
                         _resetCmdBuffer();
                     }
                 } else if (_cmdBufferIndex < USB_CMD_BUFFER_SIZE - 1) {
                     // Add to buffer
                     _cmdBuffer[_cmdBufferIndex++] = c;
                 }
             }
         }
     }
     
     // Check for transfer timeout
     if (_transferActive) {
         if (millis() - _lastConnCheckTime > USB_TRANSFER_TIMEOUT) {
             // Transfer timed out
             _transferFile.close();
             _transferActive = false;
             _sendAck(false, "Transfer timeout");
             TDECK_LOG_E("File transfer timeout");
         }
     }
 }
 
 /**
  * @brief Set the USB Mode
  * 
  * @param mode The USB mode to set
  */
 void USBManager::setMode(USBMode mode) {
     if (_mode != mode) {
         _mode = mode;
         
         // Reset state when changing mode
         _resetCmdBuffer();
         _transferActive = false;
         
         if (_transferFile) {
             _transferFile.close();
         }
         
         switch (_mode) {
             case USBMode::CDC_CONSOLE:
                 TDECK_LOG_I("USB mode: CDC Console");
                 sendFormatted("%s Mode set to CDC Console\r\n", USB_RESP_INFO);
                 break;
                 
             case USBMode::DATA_TRANSFER:
                 TDECK_LOG_I("USB mode: Data Transfer");
                 sendFormatted("%s Mode set to Data Transfer\r\n", USB_RESP_INFO);
                 break;
                 
             case USBMode::FIRMWARE_UPDATE:
                 TDECK_LOG_I("USB mode: Firmware Update");
                 sendFormatted("%s Mode set to Firmware Update\r\n", USB_RESP_INFO);
                 break;
                 
             default:
                 TDECK_LOG_W("USB mode: Unknown (%d)", static_cast<int>(_mode));
                 break;
         }
     }
 }
 
 /**
  * @brief Get current USB Mode
  * 
  * @return USBMode The current USB mode
  */
 USBMode USBManager::getMode() const {
     return _mode;
 }
 
 /**
  * @brief Check if USB is connected
  * 
  * @return true if USB is connected
  * @return false if USB is not connected
  */
 bool USBManager::isConnected() const {
     return _connected;
 }
 
 /**
  * @brief Send data over USB CDC
  * 
  * @param data The data to send
  * @param len The length of the data
  * @return size_t The number of bytes sent
  */
 size_t USBManager::sendData(const uint8_t* data, size_t len) {
     if (!_connected) {
         return 0;
     }
     
     return _usbCDC.write(data, len);
 }
 
 /**
  * @brief Send formatted string over USB CDC
  * 
  * @param format Format string (printf style)
  * @param ... Additional arguments for formatting
  * @return size_t The number of bytes sent
  */
 size_t USBManager::sendFormatted(const char* format, ...) {
     if (!_connected) {
         return 0;
     }
     
     char buffer[512];
     va_list args;
     va_start(args, format);
     int len = vsnprintf(buffer, sizeof(buffer), format, args);
     va_end(args);
     
     if (len > 0) {
         return _usbCDC.write((uint8_t*)buffer, len);
     }
     
     return 0;
 }
 
 /**
  * @brief Enter firmware update mode
  * 
  * @return true if successful
  * @return false if failed
  */
 bool USBManager::enterUpdateMode() {
     TDECK_LOG_I("Entering firmware update mode");
     
     // Set update mode
     setMode(USBMode::FIRMWARE_UPDATE);
     
     // Notify user
     sendFormatted("%s Entering firmware update mode. Device will reboot.\r\n", USB_RESP_INFO);
     
     // Small delay to allow USB message to be sent
     delay(500);
     
     // Restart in download mode
     esp_restart();
     
     return true;
 }
 
 /**
  * @brief Process a received command
  * 
  * @param cmd The command string to process
  */
 void USBManager::_processCommand(const char* cmd) {
     TDECK_LOG_I("USB command: %s", cmd);
     
     // Parse command and parameters
     String params[8];
     String cmdStr = String(cmd);
     int paramCount = _parseParams(cmd, params, 8);
     
     if (paramCount == 0) {
         _sendAck(false, "Empty command");
         return;
     }
     
     // Convert command to uppercase for comparison
     params[0].toUpperCase();
     
     // Handle commands
     if (params[0] == "VERSION") {
         // Get firmware version
         sendFormatted("%s %s v%s\r\n", USB_RESP_OK, TDECK_FIRMWARE_NAME, TDECK_FIRMWARE_VERSION);
     } 
     else if (params[0] == "LIST") {
         // List files in directory
         if (paramCount < 2) {
             _handleListFiles("/");
         } else {
             _handleListFiles(params[1]);
         }
     }
     else if (params[0] == "READ") {
         // Read file
         if (paramCount < 2) {
             _sendAck(false, "Missing file path");
         } else {
             _handleReadFile(params[1]);
         }
     }
     else if (params[0] == "WRITE") {
         // Write file
         if (paramCount < 3) {
             _sendAck(false, "Missing parameters (path, size)");
         } else {
             size_t size = params[2].toInt();
             _handleWriteFile(params[1], size);
         }
     }
     else if (params[0] == "DELETE") {
         // Delete file
         if (paramCount < 2) {
             _sendAck(false, "Missing file path");
         } else {
             _handleDeleteFile(params[1]);
         }
     }
     else if (params[0] == "MKDIR") {
         // Create directory
         if (paramCount < 2) {
             _sendAck(false, "Missing directory path");
         } else {
             _handleMakeDir(params[1]);
         }
     }
     else if (params[0] == "RMDIR") {
         // Remove directory
         if (paramCount < 2) {
             _sendAck(false, "Missing directory path");
         } else {
             _handleRemoveDir(params[1]);
         }
     }
     else if (params[0] == "INFO") {
         // Get file info
         if (paramCount < 2) {
             _sendAck(false, "Missing file path");
         } else {
             _handleFileInfo(params[1]);
         }
     }
     else if (params[0] == "SYSINFO") {
         // Get system information
         _handleSysInfo();
     }
     else if (params[0] == "REBOOT") {
         // Reboot device
         sendFormatted("%s Rebooting device...\r\n", USB_RESP_OK);
         delay(500);
         ESP.restart();
     }
     else if (params[0] == "UPDATE") {
         // Enter firmware update mode
         enterUpdateMode();
     }
     else if (params[0] == "MODE") {
         // Change USB mode
         if (paramCount < 2) {
             sendFormatted("%s Current mode: %d\r\n", USB_RESP_OK, static_cast<int>(_mode));
         } else {
             int mode = params[1].toInt();
             if (mode >= 0 && mode <= 2) {
                 setMode(static_cast<USBMode>(mode));
                 _sendAck(true, "Mode changed");
             } else {
                 _sendAck(false, "Invalid mode");
             }
         }
     }
     else if (params[0] == "HELP") {
         // Show available commands
         sendFormatted("%s Available commands:\r\n", USB_RESP_INFO);
         sendFormatted("VERSION - Get firmware version\r\n");
         sendFormatted("LIST [path] - List files in directory\r\n");
         sendFormatted("READ path - Read file content\r\n");
         sendFormatted("WRITE path size - Write file (binary data follows)\r\n");
         sendFormatted("DELETE path - Delete a file\r\n");
         sendFormatted("MKDIR path - Create a directory\r\n");
         sendFormatted("RMDIR path - Remove a directory\r\n");
         sendFormatted("INFO path - Get file information\r\n");
         sendFormatted("SYSINFO - Get system information\r\n");
         sendFormatted("MODE [mode] - Get/set USB mode (0=CDC, 1=Transfer, 2=Update)\r\n");
         sendFormatted("UPDATE - Enter firmware update mode\r\n");
         sendFormatted("REBOOT - Reboot device\r\n");
         sendFormatted("HELP - Show this help\r\n");
     }
     else {
         // Unknown command
         _sendAck(false, "Unknown command");
     }
 }
 
 /**
  * @brief Parse command parameters
  * 
  * @param cmd The full command string
  * @param params Output array for parsed parameters
  * @param maxParams Maximum number of parameters to parse
  * @return int Number of parameters parsed
  */
 int USBManager::_parseParams(const char* cmd, String params[], int maxParams) {
     String cmdStr = String(cmd);
     int paramCount = 0;
     int start = 0;
     int end = 0;
     
     // Parse until we reach end of string or max params
     while (end >= 0 && paramCount < maxParams) {
         end = cmdStr.indexOf(' ', start);
         
         if (end < 0) {
             // Last parameter
             params[paramCount++] = cmdStr.substring(start);
         } else {
             // More parameters
             params[paramCount++] = cmdStr.substring(start, end);
             start = end + 1;
         }
     }
     
     return paramCount;
 }
 
 /**
  * @brief Handle file listing command
  * 
  * @param path Directory path to list
  */
 void USBManager::_handleListFiles(const String& path) {
     String dirContents;
     
     if (fsManager.listDir(path.c_str(), dirContents)) {
         sendFormatted("%s %s\r\n", USB_RESP_OK, dirContents.c_str());
     } else {
         _sendAck(false, "Failed to list directory");
     }
 }
 
 /**
  * @brief Handle file read command
  * 
  * @param path File path to read
  */
 void USBManager::_handleReadFile(const String& path) {
     // Open file for reading
     File file = fsManager.open(path.c_str(), FILE_READ);
     
     if (!file || !file.available()) {
         _sendAck(false, "Failed to open file");
         return;
     }
     
     // Get file size
     size_t fileSize = file.size();
     
     // Send file size and prepare client for data
     sendFormatted("%s %d\r\n", USB_RESP_DATA, fileSize);
     
     // Small delay to ensure client is ready
     delay(100);
     
     // Read and send file in chunks
     const size_t bufSize = 1024;
     uint8_t buf[bufSize];
     size_t bytesRead = 0;
     
     while (bytesRead < fileSize) {
         size_t toRead = min(bufSize, fileSize - bytesRead);
         size_t read = file.read(buf, toRead);
         
         if (read == 0) {
             break;
         }
         
         sendData(buf, read);
         bytesRead += read;
     }
     
     file.close();
     
     // Log success
     TDECK_LOG_I("Sent file: %s (%d bytes)", path.c_str(), bytesRead);
 }
 
 /**
  * @brief Handle file write command
  * 
  * @param path File path to write
  * @param size Size of the file to write
  * @return true if preparation was successful
  * @return false if preparation failed
  */
 bool USBManager::_handleWriteFile(const String& path, size_t size) {
     // Ensure transfer is not active
     if (_transferActive) {
         _sendAck(false, "Transfer already active");
         return false;
     }
     
     // Create necessary directories if they don't exist
     String dirPath = path;
     int lastSlash = dirPath.lastIndexOf('/');
     if (lastSlash > 0) {
         dirPath = dirPath.substring(0, lastSlash);
         fsManager.createDir(dirPath.c_str());
     }
     
     // Open file for writing
     _transferFile = fsManager.open(path.c_str(), FILE_WRITE);
     
     if (!_transferFile) {
         _sendAck(false, "Failed to create file");
         return false;
     }
     
     // Set up transfer state
     _currentTransferPath = path;
     _totalTransferSize = size;
     _transferredBytes = 0;
     _transferActive = true;
     _lastConnCheckTime = millis(); // Reset timeout counter
     
     // Acknowledge and start receiving data
     _sendAck(true, "Ready for data");
     TDECK_LOG_I("Starting file transfer to: %s (%d bytes)", path.c_str(), size);
     
     return true;
 }
 
 /**
  * @brief Handle file delete command
  * 
  * @param path File path to delete
  */
 void USBManager::_handleDeleteFile(const String& path) {
     if (fsManager.deleteFile(path.c_str())) {
         _sendAck(true, "File deleted");
         TDECK_LOG_I("Deleted file: %s", path.c_str());
     } else {
         _sendAck(false, "Failed to delete file");
     }
 }
 
 /**
  * @brief Handle directory creation
  * 
  * @param path Directory path to create
  */
 void USBManager::_handleMakeDir(const String& path) {
     if (fsManager.createDir(path.c_str())) {
         _sendAck(true, "Directory created");
         TDECK_LOG_I("Created directory: %s", path.c_str());
     } else {
         _sendAck(false, "Failed to create directory");
     }
 }
 
 /**
  * @brief Handle directory removal
  * 
  * @param path Directory path to remove
  */
 void USBManager::_handleRemoveDir(const String& path) {
     if (fsManager.removeDir(path.c_str())) {
         _sendAck(true, "Directory removed");
         TDECK_LOG_I("Removed directory: %s", path.c_str());
     } else {
         _sendAck(false, "Failed to remove directory");
     }
 }
 
 /**
  * @brief Handle file info request
  * 
  * @param path File path to get info about
  */
 void USBManager::_handleFileInfo(const String& path) {
     size_t size;
     time_t modTime;
     
     if (fsManager.getFileInfo(path.c_str(), &size, &modTime)) {
         // Create JSON response with file info
         DynamicJsonDocument doc(256);
         doc["path"] = path;
         doc["size"] = size;
         doc["modified"] = modTime;
         doc["exists"] = true;
         
         // Serialize JSON to string
         String jsonStr;
         serializeJson(doc, jsonStr);
         
         // Send response
         sendFormatted("%s %s\r\n", USB_RESP_OK, jsonStr.c_str());
     } else {
         _sendAck(false, "File not found");
     }
 }
 
 /**
  * @brief Handle system information request
  */
 void USBManager::_handleSysInfo() {
     // Create JSON document for system info
     DynamicJsonDocument doc(1024);
     
     // General info
     doc["firmware"] = TDECK_FIRMWARE_NAME;
     doc["version"] = TDECK_FIRMWARE_VERSION;
     
     // ESP32 info
     doc["chip"] = "ESP32-S3";
     doc["cpu_freq"] = ESP.getCpuFreqMHz();
     doc["flash_size"] = ESP.getFlashChipSize();
     doc["heap_size"] = ESP.getHeapSize();
     doc["free_heap"] = ESP.getFreeHeap();
     doc["sdk_version"] = ESP.getSdkVersion();
     
     // Battery info
     doc["battery"] = powerManager.getBatteryPercentage();
     doc["battery_voltage"] = powerManager.getBatteryVoltage();
     doc["charging"] = powerManager.isCharging();
     
     // File system info
     size_t total = 0, used = 0;
     if (fsManager.getFSInfo(&total, &used)) {
         doc["fs_total"] = total;
         doc["fs_used"] = used;
         doc["fs_free"] = total - used;
     }
     
     // Features
     JsonObject features = doc.createNestedObject("features");
     features["wifi"] = TDECK_FEATURE_WIFI == 1;
     features["bluetooth"] = TDECK_FEATURE_BLUETOOTH == 1;
     features["lora"] = TDECK_FEATURE_LORA == 1;
     features["sd_card"] = TDECK_FEATURE_SD_CARD == 1;
     features["ota"] = TDECK_FEATURE_OTA == 1;
     
     // Serialize JSON to string
     String jsonStr;
     serializeJson(doc, jsonStr);
     
     // Send response
     sendFormatted("%s %s\r\n", USB_RESP_OK, jsonStr.c_str());
 }
 
 /**
  * @brief Send an acknowledgement response
  * 
  * @param success Whether the operation was successful
  * @param message Optional message to include
  */
 void USBManager::_sendAck(bool success, const char* message) {
     if (success) {
         if (message) {
             sendFormatted("%s %s\r\n", USB_RESP_OK, message);
         } else {
             sendFormatted("%s\r\n", USB_RESP_OK);
         }
     } else {
         if (message) {
             sendFormatted("%s %s\r\n", USB_RESP_ERROR, message);
         } else {
             sendFormatted("%s Unknown error\r\n", USB_RESP_ERROR);
         }
     }
 }
 
 /**
  * @brief Reset the command buffer
  */
 void USBManager::_resetCmdBuffer() {
     memset(_cmdBuffer, 0, USB_CMD_BUFFER_SIZE);
     _cmdBufferIndex = 0;
 }