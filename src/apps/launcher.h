/**
 * @file launcher.h
 * @brief Home screen and app launcher for T-Deck UI Firmware
 * 
 * This class implements the home screen and app launcher functionality,
 * providing a grid of app icons that can be navigated using touch or keyboard.
 */

 #ifndef TDECK_LAUNCHER_H
 #define TDECK_LAUNCHER_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include <vector>
 #include <string>
 #include "../config.h"
 
 /**
  * @struct AppInfo
  * @brief Structure to hold information about an application
  */
 struct AppInfo {
     String name;                 // App name
     String description;          // App description
     const void* icon;            // App icon (pointer to image data)
     lv_img_dsc_t* iconDesc;      // LVGL image descriptor
     void (*launchCallback)();    // Function to call when launching the app
 };
 
 /**
  * @class Launcher
  * @brief Implements the home screen and app launcher functionality
  */
 class Launcher {
 public:
     /**
      * @brief Constructor
      */
     Launcher();
     
     /**
      * @brief Destructor
      */
     ~Launcher();
     
     /**
      * @brief Initialize the launcher
      * @param parent Parent LVGL container
      * @return true if successful, false otherwise
      */
     bool init(lv_obj_t* parent);
     
     /**
      * @brief Start the launcher (show it on screen)
      */
     void start();
     
     /**
      * @brief Hide the launcher
      */
     void hide();
     
     /**
      * @brief Register a new app in the launcher
      * @param name App name
      * @param description App description
      * @param icon App icon
      * @param iconDesc LVGL image descriptor
      * @param launchCallback Function to call when launching the app
      * @return true if registration successful, false otherwise
      */
     bool registerApp(const String& name, const String& description, 
                      const void* icon, lv_img_dsc_t* iconDesc, 
                      void (*launchCallback)());
     
     /**
      * @brief Launch an app by index
      * @param index Index of the app to launch
      * @return true if successful, false otherwise
      */
     bool launchApp(uint8_t index);
     
     /**
      * @brief Launch an app by name
      * @param name Name of the app to launch
      * @return true if successful, false otherwise
      */
     bool launchApp(const String& name);
     
     /**
      * @brief Get the list of registered apps
      * @return Vector of registered AppInfo structures
      */
     const std::vector<AppInfo>& getApps() const { return apps; }
     
     /**
      * @brief Get the currently selected app index
      * @return Index of selected app
      */
     uint8_t getSelectedIndex() const { return selectedIndex; }
     
     /**
      * @brief Set the currently selected app index
      * @param index Index to select
      */
     void setSelectedIndex(uint8_t index);
     
     /**
      * @brief Keyboard handler for launcher navigation
      * @param key Key code
      * @return true if key was handled, false otherwise
      */
     bool handleKeyPress(uint32_t key);
 
 private:
     lv_obj_t* parent;                // Parent LVGL container
     lv_obj_t* launcherContainer;     // Main launcher container
     lv_obj_t* gridContainer;         // Grid container for app icons
     lv_obj_t* statusBar;             // Status bar object
     lv_obj_t* statusClock;           // Clock display in status bar
     lv_obj_t* statusBattery;         // Battery indicator in status bar
     lv_obj_t* statusWifi;            // WiFi indicator in status bar
     lv_obj_t* statusBluetooth;       // Bluetooth indicator in status bar
     lv_obj_t* appInfoPanel;          // Panel showing selected app info
     lv_obj_t* appNameLabel;          // Label showing app name
     lv_obj_t* appDescLabel;          // Label showing app description
     
     std::vector<AppInfo> apps;       // Vector of registered apps
     uint8_t selectedIndex;           // Currently selected app index
     uint8_t columns;                 // Number of columns in the grid
     uint8_t rows;                    // Number of rows in the grid
     
     lv_style_t gridStyle;            // Grid style
     lv_style_t iconStyle;            // Icon style
     lv_style_t iconStyleSelected;    // Selected icon style
     lv_style_t statusBarStyle;       // Status bar style
     lv_style_t infoBarStyle;         // Information bar style
     
     /**
      * @brief Create the launcher UI
      */
     void createUI();
     
     /**
      * @brief Create the status bar
      */
     void createStatusBar();
     
     /**
      * @brief Create the app grid
      */
     void createAppGrid();
     
     /**
      * @brief Create the app info panel
      */
     void createAppInfoPanel();
     
     /**
      * @brief Update the app grid
      */
     void updateAppGrid();
     
     /**
      * @brief Update the status bar
      */
     void updateStatusBar();
     
     /**
      * @brief Update the app info panel
      */
     void updateAppInfoPanel();
     
     /**
      * @brief Calculate grid dimensions based on number of apps
      */
     void calculateGridDimensions();
     
     /**
      * @brief Static event handler for icon click events
      * @param obj Clicked object
      * @param event Event data
      */
     static void iconClickHandler(lv_obj_t* obj, lv_event_t event);
     
     /**
      * @brief Periodic timer callback for updating status bar
      * @param timer Timer instance
      */
     static void statusBarTimerCallback(lv_task_t* task);
 };
 
 #endif // TDECK_LAUNCHER_H