/**
 * @file system_info.h
 * @brief System Information application for T-Deck UI
 * 
 * This application displays system information including:
 * - Firmware version and build
 * - Battery status
 * - Memory usage (RAM, PSRAM, Flash)
 * - CPU information
 * - Connected peripherals
 * - Network status
 */

 #ifndef SYSTEM_INFO_H
 #define SYSTEM_INFO_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include "../config.h"
 #include "../hal/power.h"
 #include "../ui/ui_manager.h"
 #include "../system/battery.h"
 
 class SystemInfo {
 public:
     /**
      * Constructor
      */
     SystemInfo();
     
     /**
      * Destructor
      */
     ~SystemInfo();
     
     /**
      * Initialize the System Info application
      * @param parent Parent container for the application
      * @return true if initialization successful, false otherwise
      */
     bool init(lv_obj_t* parent);
     
     /**
      * Start the System Info application
      */
     void start();
     
     /**
      * Stop the System Info application
      */
     void stop();
 
     /**
      * Updates the system information displayed
      * Should be called periodically to refresh data
      */
     void update();
 
 private:
     // UI elements
     lv_obj_t* _mainPage;
     lv_obj_t* _tabView;
     lv_obj_t* _tabSystem;
     lv_obj_t* _tabMemory;
     lv_obj_t* _tabNetwork;
     lv_obj_t* _tabHardware;
     
     // Data labels
     lv_obj_t* _lblVersion;
     lv_obj_t* _lblBuildDate;
     lv_obj_t* _lblUptime;
     lv_obj_t* _lblBatteryStatus;
     lv_obj_t* _lblBatteryVoltage;
     lv_obj_t* _lblBatteryPercent;
     lv_obj_t* _lblChargingStatus;
     
     // Memory information
     lv_obj_t* _lblFreeHeap;
     lv_obj_t* _lblMinFreeHeap;
     lv_obj_t* _lblMaxAllocHeap;
     lv_obj_t* _barHeapUsage;
     lv_obj_t* _lblPsramSize;
     lv_obj_t* _lblFreePsram;
     lv_obj_t* _barPsramUsage;
     lv_obj_t* _lblFlashSize;
     lv_obj_t* _lblFlashUsed;
     lv_obj_t* _barFlashUsage;
     
     // Network information
     lv_obj_t* _lblWifiStatus;
     lv_obj_t* _lblWifiSsid;
     lv_obj_t* _lblWifiIp;
     lv_obj_t* _lblWifiMac;
     lv_obj_t* _lblBtStatus;
     lv_obj_t* _lblBtName;
     lv_obj_t* _lblLoraStatus;
     lv_obj_t* _lblLoraFreq;
     
     // Hardware information
     lv_obj_t* _lblCpuType;
     lv_obj_t* _lblCpuFreq;
     lv_obj_t* _lblCpuTemp;
     lv_obj_t* _lblCpuCores;
     lv_obj_t* _barCpu0Usage;
     lv_obj_t* _barCpu1Usage;
     lv_obj_t* _lblDisplayInfo;
     lv_obj_t* _lblSdCardInfo;
     
     // References to other system components
     PowerManager* _powerManager;
     
     // Internal methods
     /**
      * Creates the system information tab
      */
     void _createSystemTab();
     
     /**
      * Creates the memory information tab
      */
     void _createMemoryTab();
     
     /**
      * Creates the network information tab
      */
     void _createNetworkTab();
     
     /**
      * Creates the hardware information tab
      */
     void _createHardwareTab();
     
     /**
      * Updates the system tab information
      */
     void _updateSystemTab();
     
     /**
      * Updates the memory tab information
      */
     void _updateMemoryTab();
     
     /**
      * Updates the network tab information
      */
     void _updateNetworkTab();
     
     /**
      * Updates the hardware tab information
      */
     void _updateHardwareTab();
     
     /**
      * Creates a label pair (title and value)
      * @param parent Parent container
      * @param title Title text
      * @param yPos Y position for the label
      * @return Pointer to the value label
      */
     lv_obj_t* _createLabelPair(lv_obj_t* parent, const char* title, int yPos);
     
     /**
      * Creates a usage bar with label
      * @param parent Parent container
      * @param title Title text
      * @param yPos Y position for the bar
      * @return Pointer to the bar object
      */
     lv_obj_t* _createUsageBar(lv_obj_t* parent, const char* title, int yPos);
     
     /**
      * Gets the device uptime as formatted string
      * @param buffer Buffer to store the formatted string
      * @param size Size of the buffer
      */
     void _getUptimeString(char* buffer, size_t size);
     
     /**
      * Formats a size in bytes to a human-readable string
      * @param bytes Size in bytes
      * @param buffer Buffer to store the formatted string
      * @param size Size of the buffer
      */
     void _formatByteSize(uint32_t bytes, char* buffer, size_t size);
     
     bool _isInitialized;
     bool _isActive;
     unsigned long _lastUpdateTime;
 };
 
 #endif // SYSTEM_INFO_H