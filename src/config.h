/**
 * @file config.h
 * @brief Global configuration for T-Deck UI Firmware
 * 
 * This file contains all global configuration parameters, pin definitions,
 * feature flags, and other constants used throughout the project.
 */

 #ifndef TDECK_CONFIG_H
 #define TDECK_CONFIG_H
 
 #include <Arduino.h>
 
 // Version information
 #define TDECK_FIRMWARE_VERSION "1.0.0"
 #define TDECK_FIRMWARE_NAME "T-Deck UI"
 
 // Debug configuration
 #define TDECK_DEBUG 1                // Enable/disable debug output
 #define TDECK_DEBUG_SERIAL Serial    // Serial port for debug output
 
 // Hardware configuration (most pins are defined in platformio.ini)
 // Some additional hardware-specific parameters
 #define TDECK_DISPLAY_BL_PIN 38      // Display backlight control pin
 #define TDECK_DISPLAY_BL_FREQ 5000   // PWM frequency for backlight
 #define TDECK_DISPLAY_BL_CHANNEL 0   // PWM channel for backlight
 #define TDECK_DISPLAY_BL_RESOLUTION 8 // PWM resolution for backlight
 #define TDECK_DISPLAY_BL_MAX 255     // Maximum backlight value
 
 // Battery
 #define TDECK_BATTERY_MIN_VOLTAGE 3.3f  // Minimum battery voltage
 #define TDECK_BATTERY_MAX_VOLTAGE 4.2f  // Maximum battery voltage
 #define TDECK_BATTERY_LOW_THRESHOLD 15  // Low battery warning threshold (%)
 #define TDECK_BATTERY_CRITICAL_THRESHOLD 5 // Critical battery threshold (%)
 #define TDECK_POWER_SAVE_TIMEOUT 60000  // Power save timeout in milliseconds (1 minute)
 #define TDECK_SLEEP_TIMEOUT 300000      // Sleep timeout in milliseconds (5 minutes)
 
 // LoRa Configuration
 #define TDECK_LORA_FREQUENCY 915E6   // Default LoRa frequency in Hz (915MHz)
 #define TDECK_LORA_BANDWIDTH 125E3   // Default bandwidth (125kHz)
 #define TDECK_LORA_SPREADING_FACTOR 7 // Default spreading factor
 #define TDECK_LORA_CODING_RATE 5     // Default coding rate (4/5)
 #define TDECK_LORA_SYNC_WORD 0x12    // Default sync word
 
 // WiFi Configuration
 #define TDECK_WIFI_AP_SSID "T-Deck"  // Default Access Point SSID
 #define TDECK_WIFI_AP_PASSWORD "tdeck1234" // Default Access Point password
 #define TDECK_WIFI_AP_CHANNEL 1      // Default Access Point channel
 #define TDECK_WIFI_AP_MAX_CONNECTIONS 4 // Maximum AP connections
 #define TDECK_WIFI_AP_HIDE_SSID false // Hide SSID
 #define TDECK_WIFI_SCAN_TIMEOUT 10000 // WiFi scan timeout in milliseconds
 
 // Bluetooth Configuration
 #define TDECK_BT_DEVICE_NAME "T-Deck" // Bluetooth device name
 
 // UI Configuration
 #define TDECK_UI_REFRESH_RATE 20     // UI refresh rate in milliseconds
 #define TDECK_STATUS_BAR_HEIGHT 24   // Status bar height in pixels
 #define TDECK_MAX_APPS 16            // Maximum number of apps in launcher
 #define TDECK_APP_ICON_SIZE 64       // App icon size in pixels
 #define TDECK_UI_ANIMATION_SPEED 200 // Animation duration in milliseconds
 #define TDECK_UI_BUTTON_PADDING 10   // Default padding for buttons
 #define TDECK_UI_DEFAULT_CORNER_RADIUS 5 // Default corner radius for UI elements
 
 // File System Configuration
 #define TDECK_FS_MAX_PATH_LENGTH 128 // Maximum file path length
 #define TDECK_FS_CONFIG_DIR "/config" // Configuration directory
 #define TDECK_FS_APPS_DIR "/apps"    // Applications directory
 #define TDECK_FS_TEMP_DIR "/temp"    // Temporary files directory
 #define TDECK_FS_DOWNLOADS_DIR "/downloads" // Downloads directory
 #define TDECK_FS_CONFIG_FILE "/config/system.json" // System configuration file
 #define TDECK_FS_WIFI_CONFIG_FILE "/config/wifi.json" // WiFi configuration file
 #define TDECK_FS_BT_CONFIG_FILE "/config/bluetooth.json" // Bluetooth configuration file
 #define TDECK_FS_LORA_CONFIG_FILE "/config/lora.json" // LoRa configuration file
 
 // Feature flags
 #define TDECK_FEATURE_WIFI 1         // Enable/disable WiFi functionality
 #define TDECK_FEATURE_BLUETOOTH 1    // Enable/disable Bluetooth functionality
 #define TDECK_FEATURE_LORA 1         // Enable/disable LoRa functionality
 #define TDECK_FEATURE_SD_CARD 1      // Enable/disable SD card functionality
 #define TDECK_FEATURE_OTA 1          // Enable/disable OTA updates
 #define TDECK_FEATURE_BATTERY_MONITOR 1 // Enable/disable battery monitoring
 #define TDECK_FEATURE_KEYBOARD 1     // Enable/disable physical keyboard
 #define TDECK_FEATURE_TOUCH 1        // Enable/disable touchscreen
 #define TDECK_FEATURE_DARK_MODE 1    // Enable/disable dark mode option
 
 // System parameters
 #define TDECK_SYSTEM_TASK_STACK_SIZE 4096 // Stack size for system tasks
 #define TDECK_SYSTEM_TASK_PRIORITY 1  // Priority for system tasks
 #define TDECK_UI_TASK_STACK_SIZE 8192 // Stack size for UI task
 #define TDECK_UI_TASK_PRIORITY 2      // Priority for UI task
 #define TDECK_COMMS_TASK_STACK_SIZE 4096 // Stack size for communications tasks
 #define TDECK_COMMS_TASK_PRIORITY 1   // Priority for communications tasks
 
 // Memory allocation
 #define TDECK_HEAP_ALLOCATION_THRESHOLD 1024 // Warning threshold for heap allocations
 
 // Debug macros
 #if TDECK_DEBUG
   #define TDECK_LOG(fmt, ...) TDECK_DEBUG_SERIAL.printf("[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
   #define TDECK_LOG_I(fmt, ...) TDECK_DEBUG_SERIAL.printf("[INFO] " fmt "\n", ##__VA_ARGS__)
   #define TDECK_LOG_W(fmt, ...) TDECK_DEBUG_SERIAL.printf("[WARN] " fmt "\n", ##__VA_ARGS__)
   #define TDECK_LOG_E(fmt, ...) TDECK_DEBUG_SERIAL.printf("[ERROR] " fmt "\n", ##__VA_ARGS__)
 #else
   #define TDECK_LOG(fmt, ...)
   #define TDECK_LOG_I(fmt, ...)
   #define TDECK_LOG_W(fmt, ...)
   #define TDECK_LOG_E(fmt, ...)
 #endif
 
 #endif // TDECK_CONFIG_H