/**
 * @file usb.h
 * @brief USB communication for T-Deck
 * 
 * This file contains the USB Manager class for handling USB communication,
 * including CDC (serial) communications, data transfer, and USB device enumeration.
 * It supports file transfer protocols and system commands over USB.
 */

 #ifndef TDECK_USB_H
 #define TDECK_USB_H
 
 #include <Arduino.h>
 #include <USB.h>
 #include <USBCDC.h>
 #include "../config.h"
 #include "../system/fs_manager.h"
 
 // External reference to the file system manager
 extern FSManager fsManager;
 
 /**
  * @brief USB communication modes
  */
 enum class USBMode {
     NONE,           // No USB mode active
     CDC_CONSOLE,    // Console/Terminal mode
     DATA_TRANSFER,  // Data transfer mode
     FIRMWARE_UPDATE // Firmware update mode
 };
 
 // Command buffer size for USB commands
 #define USB_CMD_BUFFER_SIZE 256
 
 /**
  * @brief USB transfer protocol commands
  */
 enum class USBCommand {
     NONE,               // No command
     GET_VERSION,        // Get firmware version
     LIST_FILES,         // List files in directory
     READ_FILE,          // Read file data
     WRITE_FILE,         // Write file data
     DELETE_FILE,        // Delete file
     MAKE_DIR,           // Create directory
     REMOVE_DIR,         // Remove directory
     FILE_INFO,          // Get file information
     START_TRANSFER,     // Start data transfer
     END_TRANSFER,       // End data transfer
     REBOOT,             // Reboot device
     ENTER_UPDATE_MODE,  // Enter update mode
     SYS_INFO,           // Get system information
 };
 
 /**
  * @brief USB Manager class
  * 
  * Handles USB communication for the T-Deck including CDC (serial) interface
  * and data transfer protocols.
  */
 class USBManager {
 public:
     /**
      * @brief Construct a new USB Manager object
      */
     USBManager();
 
     /**
      * @brief Initialize the USB subsystem
      * 
      * @return true if initialization was successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Update USB state and process data
      * 
      * This function should be called periodically from the main loop or system task
      */
     void update();
 
     /**
      * @brief Set the USB Mode
      * 
      * @param mode The USB mode to set
      */
     void setMode(USBMode mode);
 
     /**
      * @brief Get current USB Mode
      * 
      * @return USBMode The current USB mode
      */
     USBMode getMode() const;
 
     /**
      * @brief Check if USB is connected
      * 
      * @return true if USB is connected
      * @return false if USB is not connected
      */
     bool isConnected() const;
 
     /**
      * @brief Send data over USB CDC
      * 
      * @param data The data to send
      * @param len The length of the data
      * @return size_t The number of bytes sent
      */
     size_t sendData(const uint8_t* data, size_t len);
 
     /**
      * @brief Send formatted string over USB CDC
      * 
      * @param format Format string (printf style)
      * @param ... Additional arguments for formatting
      * @return size_t The number of bytes sent
      */
     size_t sendFormatted(const char* format, ...);
 
     /**
      * @brief Enter firmware update mode
      * 
      * @return true if successful
      * @return false if failed
      */
     bool enterUpdateMode();
 
 private:
     // USB CDC instance for serial communication
     USBCDC _usbCDC;
     
     // Current USB mode
     USBMode _mode;
     
     // Command buffer for receiving commands
     char _cmdBuffer[USB_CMD_BUFFER_SIZE];
     size_t _cmdBufferIndex;
     
     // Data transfer state
     bool _transferActive;
     String _currentTransferPath;
     File _transferFile;
     size_t _totalTransferSize;
     size_t _transferredBytes;
     
     // Flag to track USB connection status
     bool _connected;
     
     // Last connection check time
     unsigned long _lastConnCheckTime;
 
     /**
      * @brief Process a received command
      * 
      * @param cmd The command string to process
      */
     void _processCommand(const char* cmd);
     
     /**
      * @brief Parse command parameters
      * 
      * @param cmd The full command string
      * @param params Output array for parsed parameters
      * @param maxParams Maximum number of parameters to parse
      * @return int Number of parameters parsed
      */
     int _parseParams(const char* cmd, String params[], int maxParams);
     
     /**
      * @brief Handle file listing command
      * 
      * @param path Directory path to list
      */
     void _handleListFiles(const String& path);
     
     /**
      * @brief Handle file read command
      * 
      * @param path File path to read
      */
     void _handleReadFile(const String& path);
     
     /**
      * @brief Handle file write command
      * 
      * @param path File path to write
      * @param size Size of the file to write
      * @return true if preparation was successful
      * @return false if preparation failed
      */
     bool _handleWriteFile(const String& path, size_t size);
     
     /**
      * @brief Handle file delete command
      * 
      * @param path File path to delete
      */
     void _handleDeleteFile(const String& path);
     
     /**
      * @brief Handle directory creation
      * 
      * @param path Directory path to create
      */
     void _handleMakeDir(const String& path);
     
     /**
      * @brief Handle directory removal
      * 
      * @param path Directory path to remove
      */
     void _handleRemoveDir(const String& path);
     
     /**
      * @brief Handle file info request
      * 
      * @param path File path to get info about
      */
     void _handleFileInfo(const String& path);
     
     /**
      * @brief Handle system information request
      */
     void _handleSysInfo();
     
     /**
      * @brief Send an acknowledgement response
      * 
      * @param success Whether the operation was successful
      * @param message Optional message to include
      */
     void _sendAck(bool success, const char* message = nullptr);
     
     /**
      * @brief Reset the command buffer
      */
     void _resetCmdBuffer();
 };
 
 #endif // TDECK_USB_H