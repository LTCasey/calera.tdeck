/**
 * @file settings.cpp
 * @brief Implementation of the T-Deck UI Settings application
 */

 #include "settings.h"
 #include "../system/fs_manager.h"
 #include "../hal/display.h"
 #include "../hal/power.h"
 #include "launcher.h"
 #include <ArduinoJson.h>
 
 // Define the global settings instance
 Settings settings;
 
 // External reference to launcher (from main)
 extern Launcher launcher;
 
 // External reference to display and power manager
 extern Display display;
 extern PowerManager powerManager;
 extern FSManager fsManager;
 
 // Power save timeout options in seconds
 const uint32_t powerSaveOptions[] = {30, 60, 120, 300, 600, 1800, 3600, 0}; // 0 = Never
 const char* powerSaveOptionLabels[] = {"30 sec", "1 min", "2 min", "5 min", "10 min", "30 min", "1 hour", "Never"};
 const uint8_t powerSaveOptionCount = sizeof(powerSaveOptions) / sizeof(powerSaveOptions[0]);
 
 // Sleep timeout options in seconds
 const uint32_t sleepOptions[] = {60, 300, 600, 1800, 3600, 7200, 0}; // 0 = Never
 const char* sleepOptionLabels[] = {"1 min", "5 min", "10 min", "30 min", "1 hour", "2 hours", "Never"};
 const uint8_t sleepOptionCount = sizeof(sleepOptions) / sizeof(sleepOptions[0]);
 
 /**
  * @brief Function to launch the settings app (for launcher registration)
  */
 void launchSettingsApp() {
     settings.start();
 }
 
 /**
  * @brief Constructor
  */
 Settings::Settings() :
     parent(nullptr),
     settingsContainer(nullptr),
     tabview(nullptr),
     tabDisplay(nullptr),
     tabPower(nullptr),
     tabSystem(nullptr),
     brightnessSlider(nullptr),
     powerSaveDropdown(nullptr),
     sleepDropdown(nullptr),
     darkModeSwitch(nullptr),
     resetButton(nullptr),
     saveButton(nullptr),
     exitButton(nullptr),
     brightness(TDECK_DISPLAY_BL_MAX),          // Default full brightness
     powerSaveTimeout(TDECK_POWER_SAVE_TIMEOUT / 1000), // Convert ms to seconds
     sleepTimeout(TDECK_SLEEP_TIMEOUT / 1000),  // Convert ms to seconds
     darkMode(false)                            // Default light mode
 {
 }
 
 /**
  * @brief Destructor
  */
 Settings::~Settings() {
     // Cleanup resources if needed
 }
 
 /**
  * @brief Initialize the settings app
  */
 bool Settings::init(lv_obj_t* parent) {
     TDECK_LOG_I("Initializing Settings app");
     this->parent = parent;
     
     // Initialize styles
     lv_style_init(&headerStyle);
     lv_style_set_text_font(&headerStyle, LV_STATE_DEFAULT, &lv_font_montserrat_18);
     lv_style_set_pad_all(&headerStyle, LV_STATE_DEFAULT, 10);
     
     lv_style_init(&containerStyle);
     lv_style_set_pad_inner(&containerStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&containerStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&containerStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_top(&containerStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&containerStyle, LV_STATE_DEFAULT, 10);
     
     lv_style_init(&labelStyle);
     lv_style_set_text_font(&labelStyle, LV_STATE_DEFAULT, &lv_font_montserrat_14);
     
     lv_style_init(&settingRowStyle);
     lv_style_set_pad_all(&settingRowStyle, LV_STATE_DEFAULT, 5);
     lv_style_set_margin_bottom(&settingRowStyle, LV_STATE_DEFAULT, 10);
     
     lv_style_init(&buttonStyle);
     lv_style_set_pad_all(&buttonStyle, LV_STATE_DEFAULT, TDECK_UI_BUTTON_PADDING);
     lv_style_set_radius(&buttonStyle, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     
     // Load saved settings
     loadSettings();
     
     // Create the settings UI
     createUI();
     
     // Hide the container by default
     lv_obj_set_hidden(settingsContainer, true);
     
     TDECK_LOG_I("Settings app initialized");
     return true;
 }
 
 /**
  * @brief Start the settings app
  */
 void Settings::start() {
     TDECK_LOG_I("Starting Settings app");
     
     if (settingsContainer) {
         // Update UI to reflect current settings
         updateUI();
         
         // Show the container
         lv_obj_set_hidden(settingsContainer, false);
     }
 }
 
 /**
  * @brief Exit the settings app and return to launcher
  */
 void Settings::exit() {
     TDECK_LOG_I("Exiting Settings app");
     
     if (settingsContainer) {
         // Hide the container
         lv_obj_set_hidden(settingsContainer, true);
         
         // Return to launcher
         launcher.start();
     }
 }
 
 /**
  * @brief Get display brightness level
  */
 uint8_t Settings::getBrightness() const {
     return brightness;
 }
 
 /**
  * @brief Set display brightness level
  */
 void Settings::setBrightness(uint8_t level) {
     brightness = level;
     
     // Update the UI if initialized
     if (brightnessSlider) {
         lv_slider_set_value(brightnessSlider, brightness, LV_ANIM_OFF);
     }
     
     // Apply to hardware
     display.setBacklight(brightness);
 }
 
 /**
  * @brief Get power save timeout in seconds
  */
 uint32_t Settings::getPowerSaveTimeout() const {
     return powerSaveTimeout;
 }
 
 /**
  * @brief Set power save timeout in seconds
  */
 void Settings::setPowerSaveTimeout(uint32_t timeout) {
     powerSaveTimeout = timeout;
     
     // Update the UI if initialized
     if (powerSaveDropdown) {
         // Find matching option index
         uint8_t selectedIndex = 0;
         for (uint8_t i = 0; i < powerSaveOptionCount; i++) {
             if (powerSaveOptions[i] == powerSaveTimeout) {
                 selectedIndex = i;
                 break;
             }
         }
         lv_dropdown_set_selected(powerSaveDropdown, selectedIndex);
     }
     
     // Apply to hardware
     powerManager.setPowerSaveTimeout(powerSaveTimeout * 1000); // Convert to ms
 }
 
 /**
  * @brief Get sleep timeout in seconds
  */
 uint32_t Settings::getSleepTimeout() const {
     return sleepTimeout;
 }
 
 /**
  * @brief Set sleep timeout in seconds
  */
 void Settings::setSleepTimeout(uint32_t timeout) {
     sleepTimeout = timeout;
     
     // Update the UI if initialized
     if (sleepDropdown) {
         // Find matching option index
         uint8_t selectedIndex = 0;
         for (uint8_t i = 0; i < sleepOptionCount; i++) {
             if (sleepOptions[i] == sleepTimeout) {
                 selectedIndex = i;
                 break;
             }
         }
         lv_dropdown_set_selected(sleepDropdown, selectedIndex);
     }
     
     // Apply to hardware
     powerManager.setSleepTimeout(sleepTimeout * 1000); // Convert to ms
 }
 
 /**
  * @brief Check if dark mode is enabled
  */
 bool Settings::isDarkModeEnabled() const {
     return darkMode;
 }
 
 /**
  * @brief Enable or disable dark mode
  */
 void Settings::setDarkMode(bool enabled) {
     darkMode = enabled;
     
     // Update the UI if initialized
     if (darkModeSwitch) {
         lv_switch_set_state(darkModeSwitch, darkMode);
     }
     
     // Apply to UI theme (implementation depends on UI manager)
     // This would typically update the global theme
     // uiManager.setTheme(darkMode ? UI_THEME_DARK : UI_THEME_LIGHT);
 }
 
 /**
  * @brief Save all settings to persistent storage
  */
 bool Settings::saveSettings() {
     TDECK_LOG_I("Saving settings to persistent storage");
     
     // Create JSON document for settings
     DynamicJsonDocument doc(512);
     
     doc["brightness"] = brightness;
     doc["powerSaveTimeout"] = powerSaveTimeout;
     doc["sleepTimeout"] = sleepTimeout;
     doc["darkMode"] = darkMode;
     
     // Save to file
     bool success = fsManager.saveJsonToFile(TDECK_FS_CONFIG_FILE, doc);
     
     if (success) {
         TDECK_LOG_I("Settings saved successfully");
     } else {
         TDECK_LOG_E("Failed to save settings");
     }
     
     return success;
 }
 
 /**
  * @brief Load settings from persistent storage
  */
 bool Settings::loadSettings() {
     TDECK_LOG_I("Loading settings from persistent storage");
     
     // Create JSON document for settings
     DynamicJsonDocument doc(512);
     
     // Load from file
     bool success = fsManager.loadJsonFromFile(TDECK_FS_CONFIG_FILE, doc);
     
     if (success) {
         TDECK_LOG_I("Settings loaded successfully");
         
         // Extract settings
         brightness = doc["brightness"] | TDECK_DISPLAY_BL_MAX;
         powerSaveTimeout = doc["powerSaveTimeout"] | (TDECK_POWER_SAVE_TIMEOUT / 1000);
         sleepTimeout = doc["sleepTimeout"] | (TDECK_SLEEP_TIMEOUT / 1000);
         darkMode = doc["darkMode"] | false;
         
         // Apply loaded settings
         applySettings();
     } else {
         TDECK_LOG_W("Failed to load settings, using defaults");
         
         // Use default values
         brightness = TDECK_DISPLAY_BL_MAX;
         powerSaveTimeout = TDECK_POWER_SAVE_TIMEOUT / 1000;
         sleepTimeout = TDECK_SLEEP_TIMEOUT / 1000;
         darkMode = false;
         
         // Apply default settings
         applySettings();
     }
     
     return success;
 }
 
 /**
  * @brief Restore default settings
  */
 void Settings::restoreDefaults() {
     TDECK_LOG_I("Restoring default settings");
     
     // Set default values
     brightness = TDECK_DISPLAY_BL_MAX;
     powerSaveTimeout = TDECK_POWER_SAVE_TIMEOUT / 1000;
     sleepTimeout = TDECK_SLEEP_TIMEOUT / 1000;
     darkMode = false;
     
     // Apply default settings
     applySettings();
     
     // Update UI
     updateUI();
     
     // Save to persistent storage
     saveSettings();
 }
 
 /**
  * @brief Handle keyboard events
  */
 bool Settings::handleKeyPress(uint32_t key) {
     TDECK_LOG_I("Settings key press: %u", key);
     
     // Handle keyboard navigation
     switch (key) {
         case LV_KEY_ESC:
             exit();
             return true;
             
         // Add more key handling as needed
         
         default:
             return false;
     }
 }
 
 /**
  * @brief Create the settings UI
  */
 void Settings::createUI() {
     // Create main container
     settingsContainer = lv_obj_create(parent, NULL);
     lv_obj_set_size(settingsContainer, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT);
     lv_obj_set_pos(settingsContainer, 0, 0);
     
     // Create header
     lv_obj_t* header = lv_cont_create(settingsContainer, NULL);
     lv_obj_set_size(header, TDECK_DISPLAY_WIDTH, 40);
     lv_obj_set_pos(header, 0, 0);
     lv_obj_set_style_local_bg_color(header, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x2196F3));
     lv_obj_set_style_local_bg_opa(header, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_100);
     
     lv_obj_t* headerLabel = lv_label_create(header, NULL);
     lv_label_set_text(headerLabel, "Settings");
     lv_obj_set_style_local_text_color(headerLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
     lv_obj_add_style(headerLabel, LV_LABEL_PART_MAIN, &headerStyle);
     lv_obj_align(headerLabel, header, LV_ALIGN_IN_LEFT_MID, 10, 0);
     
     // Create exit button
     exitButton = lv_btn_create(header, NULL);
     lv_obj_set_size(exitButton, 60, 30);
     lv_obj_align(exitButton, header, LV_ALIGN_IN_RIGHT_MID, -10, 0);
     lv_obj_add_style(exitButton, LV_BTN_PART_MAIN, &buttonStyle);
     lv_obj_t* exitLabel = lv_label_create(exitButton, NULL);
     lv_label_set_text(exitLabel, "Exit");
     lv_obj_set_user_data(exitButton, this);
     lv_obj_set_event_cb(exitButton, onExitClicked);
     
     // Create tab view for different setting categories
     tabview = lv_tabview_create(settingsContainer, NULL);
     lv_obj_set_size(tabview, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT - 40 - 50); // Leave space for header and buttons
     lv_obj_set_pos(tabview, 0, 40);
     
     // Create tabs
     tabDisplay = lv_tabview_add_tab(tabview, "Display");
     tabPower = lv_tabview_add_tab(tabview, "Power");
     tabSystem = lv_tabview_add_tab(tabview, "System");
     
     // Add styles to tabs
     lv_obj_add_style(tabDisplay, LV_CONT_PART_MAIN, &containerStyle);
     lv_obj_add_style(tabPower, LV_CONT_PART_MAIN, &containerStyle);
     lv_obj_add_style(tabSystem, LV_CONT_PART_MAIN, &containerStyle);
     
     // Create tab contents
     createDisplayTab();
     createPowerTab();
     createSystemTab();
     
     // Create bottom button bar
     lv_obj_t* buttonBar = lv_cont_create(settingsContainer, NULL);
     lv_obj_set_size(buttonBar, TDECK_DISPLAY_WIDTH, 50);
     lv_obj_set_pos(buttonBar, 0, TDECK_DISPLAY_HEIGHT - 50);
     lv_obj_set_style_local_pad_all(buttonBar, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
     lv_cont_set_layout(buttonBar, LV_LAYOUT_PRETTY_MID);
     
     // Create save button
     saveButton = lv_btn_create(buttonBar, NULL);
     lv_obj_set_size(saveButton, 100, 40);
     lv_obj_add_style(saveButton, LV_BTN_PART_MAIN, &buttonStyle);
     lv_obj_t* saveLabel = lv_label_create(saveButton, NULL);
     lv_label_set_text(saveLabel, "Save");
     lv_obj_set_user_data(saveButton, this);
     lv_obj_set_event_cb(saveButton, onSaveClicked);
     
     // Create reset button
     resetButton = lv_btn_create(buttonBar, NULL);
     lv_obj_set_size(resetButton, 100, 40);
     lv_obj_add_style(resetButton, LV_BTN_PART_MAIN, &buttonStyle);
     lv_obj_set_style_local_bg_color(resetButton, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xF44336));
     lv_obj_t* resetLabel = lv_label_create(resetButton, NULL);
     lv_label_set_text(resetLabel, "Reset");
     lv_obj_set_user_data(resetButton, this);
     lv_obj_set_event_cb(resetButton, onResetClicked);
 }
 
 /**
  * @brief Create display settings tab
  */
 void Settings::createDisplayTab() {
     // Brightness setting
     lv_obj_t* brightnessRow = lv_cont_create(tabDisplay, NULL);
     lv_obj_set_size(brightnessRow, lv_obj_get_width(tabDisplay) - 20, 50);
     lv_obj_add_style(brightnessRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* brightnessLabel = lv_label_create(brightnessRow, NULL);
     lv_label_set_text(brightnessLabel, "Brightness");
     lv_obj_add_style(brightnessLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(brightnessLabel, brightnessRow, LV_ALIGN_IN_TOP_LEFT, 0, 0);
     
     brightnessSlider = lv_slider_create(brightnessRow, NULL);
     lv_obj_set_size(brightnessSlider, lv_obj_get_width(brightnessRow) - 20, 20);
     lv_obj_align(brightnessSlider, brightnessLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
     lv_slider_set_range(brightnessSlider, 10, 255); // Minimum 10 to avoid completely dark screen
     lv_obj_set_user_data(brightnessSlider, this);
     lv_obj_set_event_cb(brightnessSlider, onBrightnessChanged);
     
     // Dark mode setting
     lv_obj_t* darkModeRow = lv_cont_create(tabDisplay, NULL);
     lv_obj_set_size(darkModeRow, lv_obj_get_width(tabDisplay) - 20, 40);
     lv_obj_add_style(darkModeRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* darkModeLabel = lv_label_create(darkModeRow, NULL);
     lv_label_set_text(darkModeLabel, "Dark Mode");
     lv_obj_add_style(darkModeLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(darkModeLabel, darkModeRow, LV_ALIGN_IN_LEFT_MID, 0, 0);
     
     darkModeSwitch = lv_switch_create(darkModeRow, NULL);
     lv_obj_align(darkModeSwitch, darkModeRow, LV_ALIGN_IN_RIGHT_MID, -10, 0);
     lv_obj_set_user_data(darkModeSwitch, this);
     lv_obj_set_event_cb(darkModeSwitch, onDarkModeChanged);
 }
 
 /**
  * @brief Create power settings tab
  */
 void Settings::createPowerTab() {
     // Power save timeout
     lv_obj_t* powerSaveRow = lv_cont_create(tabPower, NULL);
     lv_obj_set_size(powerSaveRow, lv_obj_get_width(tabPower) - 20, 60);
     lv_obj_add_style(powerSaveRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* powerSaveLabel = lv_label_create(powerSaveRow, NULL);
     lv_label_set_text(powerSaveLabel, "Dim Screen After");
     lv_obj_add_style(powerSaveLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(powerSaveLabel, powerSaveRow, LV_ALIGN_IN_TOP_LEFT, 0, 0);
     
     powerSaveDropdown = lv_dropdown_create(powerSaveRow, NULL);
     lv_obj_set_size(powerSaveDropdown, lv_obj_get_width(powerSaveRow) - 20, 30);
     lv_obj_align(powerSaveDropdown, powerSaveLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
     
     // Add options to dropdown
     String options = "";
     for (uint8_t i = 0; i < powerSaveOptionCount; i++) {
         options += powerSaveOptionLabels[i];
         if (i < powerSaveOptionCount - 1) {
             options += "\n";
         }
     }
     lv_dropdown_set_options(powerSaveDropdown, options.c_str());
     lv_obj_set_user_data(powerSaveDropdown, this);
     lv_obj_set_event_cb(powerSaveDropdown, onPowerSaveChanged);
     
     // Sleep timeout
     lv_obj_t* sleepRow = lv_cont_create(tabPower, NULL);
     lv_obj_set_size(sleepRow, lv_obj_get_width(tabPower) - 20, 60);
     lv_obj_add_style(sleepRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* sleepLabel = lv_label_create(sleepRow, NULL);
     lv_label_set_text(sleepLabel, "Sleep After");
     lv_obj_add_style(sleepLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(sleepLabel, sleepRow, LV_ALIGN_IN_TOP_LEFT, 0, 0);
     
     sleepDropdown = lv_dropdown_create(sleepRow, NULL);
     lv_obj_set_size(sleepDropdown, lv_obj_get_width(sleepRow) - 20, 30);
     lv_obj_align(sleepDropdown, sleepLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
     
     // Add options to dropdown
     options = "";
     for (uint8_t i = 0; i < sleepOptionCount; i++) {
         options += sleepOptionLabels[i];
         if (i < sleepOptionCount - 1) {
             options += "\n";
         }
     }
     lv_dropdown_set_options(sleepDropdown, options.c_str());
     lv_obj_set_user_data(sleepDropdown, this);
     lv_obj_set_event_cb(sleepDropdown, onSleepTimeoutChanged);
     
     // Add battery information section
     lv_obj_t* batteryRow = lv_cont_create(tabPower, NULL);
     lv_obj_set_size(batteryRow, lv_obj_get_width(tabPower) - 20, 40);
     lv_obj_add_style(batteryRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* batteryLabel = lv_label_create(batteryRow, NULL);
     lv_label_set_text(batteryLabel, "Battery Status");
     lv_obj_add_style(batteryLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(batteryLabel, batteryRow, LV_ALIGN_IN_LEFT_MID, 0, 0);
     
     // Battery percentage - this will be updated dynamically
     lv_obj_t* batteryValue = lv_label_create(batteryRow, NULL);
     lv_label_set_text_fmt(batteryValue, "%d%%", powerManager.getBatteryPercentage());
     lv_obj_align(batteryValue, batteryRow, LV_ALIGN_IN_RIGHT_MID, -10, 0);
 }
 
 /**
  * @brief Create system settings tab
  */
 void Settings::createSystemTab() {
     // Device information
     lv_obj_t* deviceInfoRow = lv_cont_create(tabSystem, NULL);
     lv_obj_set_size(deviceInfoRow, lv_obj_get_width(tabSystem) - 20, 80);
     lv_obj_add_style(deviceInfoRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* deviceInfoLabel = lv_label_create(deviceInfoRow, NULL);
     lv_label_set_text(deviceInfoLabel, "Device Information");
     lv_obj_set_style_local_text_font(deviceInfoLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
     lv_obj_align(deviceInfoLabel, deviceInfoRow, LV_ALIGN_IN_TOP_LEFT, 0, 0);
     
     lv_obj_t* firmwareVersionLabel = lv_label_create(deviceInfoRow, NULL);
     lv_label_set_text_fmt(firmwareVersionLabel, "Firmware: %s", TDECK_FIRMWARE_VERSION);
     lv_obj_add_style(firmwareVersionLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(firmwareVersionLabel, deviceInfoLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
     
     lv_obj_t* boardLabel = lv_label_create(deviceInfoRow, NULL);
     lv_label_set_text(boardLabel, "Board: LilyGO T-Deck");
     lv_obj_add_style(boardLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(boardLabel, firmwareVersionLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
     
     // Storage information
     lv_obj_t* storageRow = lv_cont_create(tabSystem, NULL);
     lv_obj_set_size(storageRow, lv_obj_get_width(tabSystem) - 20, 60);
     lv_obj_add_style(storageRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* storageLabel = lv_label_create(storageRow, NULL);
     lv_label_set_text(storageLabel, "Storage");
     lv_obj_set_style_local_text_font(storageLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
     lv_obj_align(storageLabel, storageRow, LV_ALIGN_IN_TOP_LEFT, 0, 0);
     
     // Get storage info from file system manager
     uint64_t totalSpace, usedSpace;
     fsManager.getStorageInfo(&totalSpace, &usedSpace);
     
     lv_obj_t* storageInfoLabel = lv_label_create(storageRow, NULL);
     lv_label_set_text_fmt(storageInfoLabel, "Used: %.1f MB / %.1f MB", 
                           (float)usedSpace / (1024 * 1024), (float)totalSpace / (1024 * 1024));
     lv_obj_add_style(storageInfoLabel, LV_LABEL_PART_MAIN, &labelStyle);
     lv_obj_align(storageInfoLabel, storageLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
     
     // System maintenance
     lv_obj_t* maintenanceRow = lv_cont_create(tabSystem, NULL);
     lv_obj_set_size(maintenanceRow, lv_obj_get_width(tabSystem) - 20, 60);
     lv_obj_add_style(maintenanceRow, LV_CONT_PART_MAIN, &settingRowStyle);
     
     lv_obj_t* maintenanceLabel = lv_label_create(maintenanceRow, NULL);
     lv_label_set_text(maintenanceLabel, "System Maintenance");
     lv_obj_set_style_local_text_font(maintenanceLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_16);
     lv_obj_align(maintenanceLabel, maintenanceRow, LV_ALIGN_IN_TOP_LEFT, 0, 0);
     
     // Create clear cache button
     lv_obj_t* clearCacheBtn = lv_btn_create(maintenanceRow, NULL);
     lv_obj_set_size(clearCacheBtn, 120, 30);
     lv_obj_align(clearCacheBtn, maintenanceLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
     lv_obj_add_style(clearCacheBtn, LV_BTN_PART_MAIN, &buttonStyle);
     
     lv_obj_t* clearCacheLabel = lv_label_create(clearCacheBtn, NULL);
     lv_label_set_text(clearCacheLabel, "Clear Cache");
     
     // Add event handler (implementation not shown)
     // lv_obj_set_user_data(clearCacheBtn, this);
     // lv_obj_set_event_cb(clearCacheBtn, onClearCacheClicked);
 }
 
 /**
  * @brief Update UI elements to reflect current settings
  */
 void Settings::updateUI() {
     // Update brightness slider
     if (brightnessSlider) {
         lv_slider_set_value(brightnessSlider, brightness, LV_ANIM_OFF);
     }
     
     // Update power save dropdown
     if (powerSaveDropdown) {
         // Find matching option index
         uint8_t selectedIndex = 0;
         for (uint8_t i = 0; i < powerSaveOptionCount; i++) {
             if (powerSaveOptions[i] == powerSaveTimeout) {
                 selectedIndex = i;
                 break;
             }
         }
         lv_dropdown_set_selected(powerSaveDropdown, selectedIndex);
     }
     
     // Update sleep dropdown
     if (sleepDropdown) {
         // Find matching option index
         uint8_t selectedIndex = 0;
         for (uint8_t i = 0; i < sleepOptionCount; i++) {
             if (sleepOptions[i] == sleepTimeout) {
                 selectedIndex = i;
                 break;
             }
         }
         lv_dropdown_set_selected(sleepDropdown, selectedIndex);
     }
     
     // Update dark mode switch
     if (darkModeSwitch) {
         lv_switch_set_state(darkModeSwitch, darkMode);
     }
 }
 
 /**
  * @brief Update settings from UI elements
  */
 void Settings::updateSettingsFromUI() {
     // Update brightness from slider
     if (brightnessSlider) {
         brightness = lv_slider_get_value(brightnessSlider);
     }
     
     // Update power save timeout from dropdown
     if (powerSaveDropdown) {
         uint8_t selectedIndex = lv_dropdown_get_selected(powerSaveDropdown);
         powerSaveTimeout = powerSaveOptions[selectedIndex];
     }
     
     // Update sleep timeout from dropdown
     if (sleepDropdown) {
         uint8_t selectedIndex = lv_dropdown_get_selected(sleepDropdown);
         sleepTimeout = sleepOptions[selectedIndex];
     }
     
     // Update dark mode from switch
     if (darkModeSwitch) {
         darkMode = lv_switch_get_state(darkModeSwitch);
     }
 }
 
 /**
  * @brief Apply current settings to hardware
  */
 void Settings::applySettings() {
     // Apply brightness
     display.setBacklight(brightness);
     
     // Apply power timeouts
     powerManager.setPowerSaveTimeout(powerSaveTimeout * 1000); // Convert to ms
     powerManager.setSleepTimeout(sleepTimeout * 1000); // Convert to ms
     
     // Apply dark mode (implementation depends on UI manager)
     // This would typically update the global theme
     // uiManager.setTheme(darkMode ? UI_THEME_DARK : UI_THEME_LIGHT);
 }
 
 /**
  * @brief Show confirmation dialog for factory reset
  */
 void Settings::showResetConfirmDialog() {
     static Settings* instance = this;
     
     // Create confirmation dialog
     lv_obj_t* mbox = lv_msgbox_create(lv_scr_act(), NULL);
     lv_msgbox_set_text(mbox, "Reset to factory defaults?");
     lv_msgbox_add_btns(mbox, "Yes\nNo", NULL);
     lv_obj_set_width(mbox, 200);
     lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
     
     // Set up event handler for message box buttons
     lv_obj_set_event_cb(mbox, [](lv_obj_t* obj, lv_event_t event) {
         if (event == LV_EVENT_VALUE_CHANGED) {
             uint16_t btn_id = lv_msgbox_get_active_btn(obj);
             if (btn_id == 0) { // "Yes" button
                 // Reset settings to defaults
                 instance->restoreDefaults();
             }
             // Close message box
             lv_msgbox_start_auto_close(obj, 0);
         }
     });
 }
 
 /**
  * @brief Static event handler for brightness slider
  */
 void Settings::onBrightnessChanged(lv_obj_t* slider, lv_event_t event) {
     if (event == LV_EVENT_VALUE_CHANGED) {
         Settings* settings = (Settings*)lv_obj_get_user_data(slider);
         if (settings) {
             uint8_t value = lv_slider_get_value(slider);
             settings->setBrightness(value);
         }
     }
 }
 
 /**
  * @brief Static event handler for power save dropdown
  */
 void Settings::onPowerSaveChanged(lv_obj_t* dropdown, lv_event_t event) {
     if (event == LV_EVENT_VALUE_CHANGED) {
         Settings* settings = (Settings*)lv_obj_get_user_data(dropdown);
         if (settings) {
             uint8_t selectedIndex = lv_dropdown_get_selected(dropdown);
             settings->setPowerSaveTimeout(powerSaveOptions[selectedIndex]);
         }
     }
 }
 
 /**
  * @brief Static event handler for sleep timeout dropdown
  */
 void Settings::onSleepTimeoutChanged(lv_obj_t* dropdown, lv_event_t event) {
     if (event == LV_EVENT_VALUE_CHANGED) {
         Settings* settings = (Settings*)lv_obj_get_user_data(dropdown);
         if (settings) {
             uint8_t selectedIndex = lv_dropdown_get_selected(dropdown);
             settings->setSleepTimeout(sleepOptions[selectedIndex]);
         }
     }
 }
 
 /**
  * @brief Static event handler for dark mode switch
  */
 void Settings::onDarkModeChanged(lv_obj_t* sw, lv_event_t event) {
     if (event == LV_EVENT_VALUE_CHANGED) {
         Settings* settings = (Settings*)lv_obj_get_user_data(sw);
         if (settings) {
             bool state = lv_switch_get_state(sw);
             settings->setDarkMode(state);
         }
     }
 }
 
 /**
  * @brief Static event handler for reset button
  */
 void Settings::onResetClicked(lv_obj_t* btn, lv_event_t event) {
     if (event == LV_EVENT_CLICKED) {
         Settings* settings = (Settings*)lv_obj_get_user_data(btn);
         if (settings) {
             settings->showResetConfirmDialog();
         }
     }
 }
 
 /**
  * @brief Static event handler for save button
  */
 void Settings::onSaveClicked(lv_obj_t* btn, lv_event_t event) {
     if (event == LV_EVENT_CLICKED) {
         Settings* settings = (Settings*)lv_obj_get_user_data(btn);
         if (settings) {
             // Update settings from UI elements
             settings->updateSettingsFromUI();
             
             // Apply settings
             settings->applySettings();
             
             // Save settings to persistent storage
             settings->saveSettings();
         }
     }
 }
 
 /**
  * @brief Static event handler for exit button
  */
 void Settings::onExitClicked(lv_obj_t* btn, lv_event_t event) {
     if (event == LV_EVENT_CLICKED) {
         Settings* settings = (Settings*)lv_obj_get_user_data(btn);
         if (settings) {
             settings->exit();
         }
     }
 }