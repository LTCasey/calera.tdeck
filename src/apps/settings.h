/**
 * @file settings.h
 * @brief Settings application for T-Deck UI Firmware
 * 
 * This class implements the settings application, providing a UI for
 * configuring system parameters such as display brightness, power settings,
 * and other device configurations.
 */

 #ifndef TDECK_SETTINGS_H
 #define TDECK_SETTINGS_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include <ArduinoJson.h>
 #include "../config.h"
 
 /**
  * @class Settings
  * @brief Implements the settings application functionality
  */
 class Settings {
 public:
     /**
      * @brief Constructor
      */
     Settings();
     
     /**
      * @brief Destructor
      */
     ~Settings();
     
     /**
      * @brief Initialize the settings app
      * @param parent Parent LVGL container
      * @return true if successful, false otherwise
      */
     bool init(lv_obj_t* parent);
     
     /**
      * @brief Start the settings app (show it on screen)
      */
     void start();
     
     /**
      * @brief Exit the settings app and return to launcher
      */
     void exit();
     
     /**
      * @brief Get display brightness level
      * @return Brightness level (0-255)
      */
     uint8_t getBrightness() const;
     
     /**
      * @brief Set display brightness level
      * @param level Brightness level (0-255)
      */
     void setBrightness(uint8_t level);
     
     /**
      * @brief Get power save timeout in seconds
      * @return Timeout in seconds
      */
     uint32_t getPowerSaveTimeout() const;
     
     /**
      * @brief Set power save timeout in seconds
      * @param timeout Timeout in seconds
      */
     void setPowerSaveTimeout(uint32_t timeout);
     
     /**
      * @brief Get sleep timeout in seconds
      * @return Timeout in seconds
      */
     uint32_t getSleepTimeout() const;
     
     /**
      * @brief Set sleep timeout in seconds
      * @param timeout Timeout in seconds
      */
     void setSleepTimeout(uint32_t timeout);
     
     /**
      * @brief Check if dark mode is enabled
      * @return true if dark mode is enabled, false otherwise
      */
     bool isDarkModeEnabled() const;
     
     /**
      * @brief Enable or disable dark mode
      * @param enabled true to enable dark mode, false to disable
      */
     void setDarkMode(bool enabled);
     
     /**
      * @brief Save all settings to persistent storage
      * @return true if successful, false otherwise
      */
     bool saveSettings();
     
     /**
      * @brief Load settings from persistent storage
      * @return true if successful, false otherwise
      */
     bool loadSettings();
     
     /**
      * @brief Restore default settings
      */
     void restoreDefaults();
     
     /**
      * @brief Handle keyboard events
      * @param key Key code
      * @return true if key was handled, false otherwise
      */
     bool handleKeyPress(uint32_t key);
 
 private:
     lv_obj_t* parent;               // Parent LVGL container
     lv_obj_t* settingsContainer;    // Main settings container
     lv_obj_t* tabview;              // Tab view for different settings categories
     lv_obj_t* tabDisplay;           // Display settings tab
     lv_obj_t* tabPower;             // Power settings tab
     lv_obj_t* tabSystem;            // System settings tab
     lv_obj_t* brightnessSlider;     // Brightness control slider
     lv_obj_t* powerSaveDropdown;    // Power save timeout dropdown
     lv_obj_t* sleepDropdown;        // Sleep timeout dropdown
     lv_obj_t* darkModeSwitch;       // Dark mode toggle switch
     lv_obj_t* resetButton;          // Factory reset button
     lv_obj_t* saveButton;           // Save settings button
     lv_obj_t* exitButton;           // Exit button
     
     // Current settings values
     uint8_t brightness;             // Display brightness (0-255)
     uint32_t powerSaveTimeout;      // Power save timeout in seconds
     uint32_t sleepTimeout;          // Sleep timeout in seconds
     bool darkMode;                  // Dark mode enabled flag
     
     // Style definitions
     lv_style_t headerStyle;         // Header style
     lv_style_t containerStyle;      // Container style
     lv_style_t labelStyle;          // Label style
     lv_style_t settingRowStyle;     // Setting row style
     lv_style_t buttonStyle;         // Button style
     
     /**
      * @brief Create the settings UI
      */
     void createUI();
     
     /**
      * @brief Create display settings tab
      */
     void createDisplayTab();
     
     /**
      * @brief Create power settings tab
      */
     void createPowerTab();
     
     /**
      * @brief Create system settings tab
      */
     void createSystemTab();
     
     /**
      * @brief Update UI elements to reflect current settings
      */
     void updateUI();
     
     /**
      * @brief Update settings from UI elements
      */
     void updateSettingsFromUI();
     
     // Static event handlers
     static void onBrightnessChanged(lv_obj_t* slider, lv_event_t event);
     static void onPowerSaveChanged(lv_obj_t* dropdown, lv_event_t event);
     static void onSleepTimeoutChanged(lv_obj_t* dropdown, lv_event_t event);
     static void onDarkModeChanged(lv_obj_t* sw, lv_event_t event);
     static void onResetClicked(lv_obj_t* btn, lv_event_t event);
     static void onSaveClicked(lv_obj_t* btn, lv_event_t event);
     static void onExitClicked(lv_obj_t* btn, lv_event_t event);
     
     /**
      * @brief Apply current settings to hardware
      */
     void applySettings();
     
     /**
      * @brief Show confirmation dialog for factory reset
      */
     void showResetConfirmDialog();
 };
 
 // Global settings instance
 extern Settings settings;
 
 // Function to launch the settings app (for launcher registration)
 void launchSettingsApp();
 
 #endif // TDECK_SETTINGS_H