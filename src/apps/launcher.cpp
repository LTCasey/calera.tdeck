/**
 * @file launcher.cpp
 * @brief Implementation of the T-Deck UI Launcher
 */

 #include "launcher.h"
 #include "../hal/power.h"
 #include "../hal/keyboard.h"
 #include "../comms/wifi.h"
 #include "../comms/bluetooth.h"
 #include "../system/fs_manager.h"
 #include <time.h>
 
 // Default app icons (to be replaced with actual icon data)
 LV_IMG_DECLARE(icon_settings);
 LV_IMG_DECLARE(icon_filebrowser);
 LV_IMG_DECLARE(icon_terminal);
 LV_IMG_DECLARE(icon_wifi);
 LV_IMG_DECLARE(icon_bluetooth);
 LV_IMG_DECLARE(icon_lora);
 LV_IMG_DECLARE(icon_system);
 
 // External references to other apps
 extern void launchSettingsApp();
 extern void launchFileBrowserApp();
 extern void launchTerminalApp();
 extern void launchWiFiManagerApp();
 extern void launchBluetoothManagerApp();
 extern void launchLoRaMessengerApp();
 extern void launchSystemInfoApp();
 
 // Status bar icons
 LV_IMG_DECLARE(icon_battery_full);
 LV_IMG_DECLARE(icon_battery_low);
 LV_IMG_DECLARE(icon_wifi_connected);
 LV_IMG_DECLARE(icon_wifi_disconnected);
 LV_IMG_DECLARE(icon_bluetooth_on);
 LV_IMG_DECLARE(icon_bluetooth_off);
 
 /**
  * @brief Constructor
  */
 Launcher::Launcher() : 
     parent(nullptr),
     launcherContainer(nullptr),
     gridContainer(nullptr),
     statusBar(nullptr),
     statusClock(nullptr),
     statusBattery(nullptr),
     statusWifi(nullptr),
     statusBluetooth(nullptr),
     appInfoPanel(nullptr),
     appNameLabel(nullptr),
     appDescLabel(nullptr),
     selectedIndex(0),
     columns(3),
     rows(2)
 {
     // Initialize the apps vector with default capacity
     apps.reserve(TDECK_MAX_APPS);
 }
 
 /**
  * @brief Destructor
  */
 Launcher::~Launcher() {
     // Clean up any allocated resources
 }
 
 /**
  * @brief Initialize the launcher
  * @param parent Parent LVGL container
  * @return true if successful, false otherwise
  */
 bool Launcher::init(lv_obj_t* parent) {
     TDECK_LOG_I("Initializing Launcher");
     this->parent = parent;
     
     // Initialize styles
     lv_style_init(&gridStyle);
     lv_style_set_pad_inner(&gridStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&gridStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&gridStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_top(&gridStyle, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&gridStyle, LV_STATE_DEFAULT, 10);
     
     lv_style_init(&iconStyle);
     lv_style_set_border_width(&iconStyle, LV_STATE_DEFAULT, 1);
     lv_style_set_border_color(&iconStyle, LV_STATE_DEFAULT, lv_color_hex(0xCCCCCC));
     lv_style_set_border_opa(&iconStyle, LV_STATE_DEFAULT, LV_OPA_30);
     lv_style_set_bg_color(&iconStyle, LV_STATE_DEFAULT, lv_color_hex(0x2196F3));
     lv_style_set_bg_opa(&iconStyle, LV_STATE_DEFAULT, LV_OPA_10);
     lv_style_set_radius(&iconStyle, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     
     lv_style_init(&iconStyleSelected);
     lv_style_set_border_width(&iconStyleSelected, LV_STATE_DEFAULT, 2);
     lv_style_set_border_color(&iconStyleSelected, LV_STATE_DEFAULT, lv_color_hex(0x2196F3));
     lv_style_set_border_opa(&iconStyleSelected, LV_STATE_DEFAULT, LV_OPA_100);
     lv_style_set_bg_color(&iconStyleSelected, LV_STATE_DEFAULT, lv_color_hex(0x2196F3));
     lv_style_set_bg_opa(&iconStyleSelected, LV_STATE_DEFAULT, LV_OPA_30);
     lv_style_set_radius(&iconStyleSelected, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     
     lv_style_init(&statusBarStyle);
     lv_style_set_bg_color(&statusBarStyle, LV_STATE_DEFAULT, lv_color_hex(0x333333));
     lv_style_set_bg_opa(&statusBarStyle, LV_STATE_DEFAULT, LV_OPA_100);
     lv_style_set_text_color(&statusBarStyle, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
     lv_style_set_pad_left(&statusBarStyle, LV_STATE_DEFAULT, 5);
     lv_style_set_pad_right(&statusBarStyle, LV_STATE_DEFAULT, 5);
     
     lv_style_init(&infoBarStyle);
     lv_style_set_bg_color(&infoBarStyle, LV_STATE_DEFAULT, lv_color_hex(0xEEEEEE));
     lv_style_set_bg_opa(&infoBarStyle, LV_STATE_DEFAULT, LV_OPA_80);
     lv_style_set_text_color(&infoBarStyle, LV_STATE_DEFAULT, lv_color_hex(0x333333));
     lv_style_set_pad_all(&infoBarStyle, LV_STATE_DEFAULT, 5);
     
     // Register default apps
     registerApp("Settings", "System configuration", &icon_settings, 
                 (lv_img_dsc_t*)&icon_settings, launchSettingsApp);
     
     registerApp("Files", "File browser", &icon_filebrowser, 
                 (lv_img_dsc_t*)&icon_filebrowser, launchFileBrowserApp);
     
     registerApp("Terminal", "Command line interface", &icon_terminal, 
                 (lv_img_dsc_t*)&icon_terminal, launchTerminalApp);
     
     registerApp("WiFi", "WiFi configuration", &icon_wifi, 
                 (lv_img_dsc_t*)&icon_wifi, launchWiFiManagerApp);
     
     registerApp("Bluetooth", "Bluetooth settings", &icon_bluetooth, 
                 (lv_img_dsc_t*)&icon_bluetooth, launchBluetoothManagerApp);
     
     registerApp("LoRa", "LoRa messenger", &icon_lora, 
                 (lv_img_dsc_t*)&icon_lora, launchLoRaMessengerApp);
     
     registerApp("System", "System information", &icon_system, 
                 (lv_img_dsc_t*)&icon_system, launchSystemInfoApp);
     
     // Create launcher UI
     createUI();
     
     // Start a periodic timer for status updates
     lv_task_create(statusBarTimerCallback, 1000, LV_TASK_PRIO_LOW, this);
     
     TDECK_LOG_I("Launcher initialized with %d apps", apps.size());
     return true;
 }
 
 /**
  * @brief Start the launcher (show it on screen)
  */
 void Launcher::start() {
     TDECK_LOG_I("Starting Launcher");
     
     if (launcherContainer) {
         lv_obj_set_hidden(launcherContainer, false);
         updateStatusBar();
         updateAppGrid();
         updateAppInfoPanel();
     }
 }
 
 /**
  * @brief Hide the launcher
  */
 void Launcher::hide() {
     TDECK_LOG_I("Hiding Launcher");
     
     if (launcherContainer) {
         lv_obj_set_hidden(launcherContainer, true);
     }
 }
 
 /**
  * @brief Register a new app in the launcher
  */
 bool Launcher::registerApp(const String& name, const String& description, 
                           const void* icon, lv_img_dsc_t* iconDesc, 
                           void (*launchCallback)()) {
     TDECK_LOG_I("Registering app: %s", name.c_str());
     
     if (apps.size() >= TDECK_MAX_APPS) {
         TDECK_LOG_W("Cannot register app '%s', max apps limit reached", name.c_str());
         return false;
     }
     
     AppInfo app;
     app.name = name;
     app.description = description;
     app.icon = icon;
     app.iconDesc = iconDesc;
     app.launchCallback = launchCallback;
     
     apps.push_back(app);
     
     // Calculate new grid dimensions
     calculateGridDimensions();
     
     // Update the UI if already created
     if (gridContainer) {
         updateAppGrid();
     }
     
     return true;
 }
 
 /**
  * @brief Launch an app by index
  */
 bool Launcher::launchApp(uint8_t index) {
     if (index >= apps.size()) {
         TDECK_LOG_W("Cannot launch app, invalid index: %d", index);
         return false;
     }
     
     TDECK_LOG_I("Launching app: %s", apps[index].name.c_str());
     
     // Hide the launcher
     hide();
     
     // Call the app's launch callback
     if (apps[index].launchCallback) {
         apps[index].launchCallback();
         return true;
     }
     
     return false;
 }
 
 /**
  * @brief Launch an app by name
  */
 bool Launcher::launchApp(const String& name) {
     for (size_t i = 0; i < apps.size(); i++) {
         if (apps[i].name.equals(name)) {
             return launchApp(i);
         }
     }
     
     TDECK_LOG_W("Cannot launch app, app not found: %s", name.c_str());
     return false;
 }
 
 /**
  * @brief Set the currently selected app index
  */
 void Launcher::setSelectedIndex(uint8_t index) {
     if (index >= apps.size()) {
         index = apps.size() - 1;
     }
     
     selectedIndex = index;
     updateAppGrid();
     updateAppInfoPanel();
 }
 
 /**
  * @brief Keyboard handler for launcher navigation
  */
 bool Launcher::handleKeyPress(uint32_t key) {
     TDECK_LOG_I("Launcher key press: %u", key);
     
     switch (key) {
         case LV_KEY_UP: {
             if (selectedIndex >= columns) {
                 setSelectedIndex(selectedIndex - columns);
             }
             return true;
         }
         
         case LV_KEY_DOWN: {
             if (selectedIndex + columns < apps.size()) {
                 setSelectedIndex(selectedIndex + columns);
             }
             return true;
         }
         
         case LV_KEY_LEFT: {
             if (selectedIndex > 0) {
                 setSelectedIndex(selectedIndex - 1);
             }
             return true;
         }
         
         case LV_KEY_RIGHT: {
             if (selectedIndex < apps.size() - 1) {
                 setSelectedIndex(selectedIndex + 1);
             }
             return true;
         }
         
         case LV_KEY_ENTER: {
             launchApp(selectedIndex);
             return true;
         }
         
         default:
             return false;
     }
 }
 
 /**
  * @brief Create the launcher UI
  */
 void Launcher::createUI() {
     // Create main container
     launcherContainer = lv_obj_create(parent, NULL);
     lv_obj_set_size(launcherContainer, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT);
     lv_obj_set_pos(launcherContainer, 0, 0);
     
     // Create status bar
     createStatusBar();
     
     // Create app grid
     createAppGrid();
     
     // Create app info panel
     createAppInfoPanel();
 }
 
 /**
  * @brief Create the status bar
  */
 void Launcher::createStatusBar() {
     // Create status bar container
     statusBar = lv_obj_create(launcherContainer, NULL);
     lv_obj_set_size(statusBar, TDECK_DISPLAY_WIDTH, TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_pos(statusBar, 0, 0);
     lv_obj_add_style(statusBar, LV_OBJ_PART_MAIN, &statusBarStyle);
     
     // Create clock display
     statusClock = lv_label_create(statusBar, NULL);
     lv_obj_align(statusClock, statusBar, LV_ALIGN_IN_LEFT_MID, 5, 0);
     lv_label_set_text(statusClock, "00:00");
     
     // Create battery indicator
     statusBattery = lv_img_create(statusBar, NULL);
     lv_obj_align(statusBattery, statusBar, LV_ALIGN_IN_RIGHT_MID, -5, 0);
     lv_img_set_src(statusBattery, &icon_battery_full);
     
     // Create WiFi indicator
     statusWifi = lv_img_create(statusBar, NULL);
     lv_obj_align(statusWifi, statusBattery, LV_ALIGN_OUT_LEFT_MID, -10, 0);
     lv_img_set_src(statusWifi, &icon_wifi_disconnected);
     
     // Create Bluetooth indicator
     statusBluetooth = lv_img_create(statusBar, NULL);
     lv_obj_align(statusBluetooth, statusWifi, LV_ALIGN_OUT_LEFT_MID, -10, 0);
     lv_img_set_src(statusBluetooth, &icon_bluetooth_off);
 }
 
 /**
  * @brief Create the app grid
  */
 void Launcher::createAppGrid() {
     // Calculate grid dimensions
     calculateGridDimensions();
     
     // Create grid container
     gridContainer = lv_cont_create(launcherContainer, NULL);
     lv_obj_set_size(gridContainer, TDECK_DISPLAY_WIDTH, 
                    TDECK_DISPLAY_HEIGHT - TDECK_STATUS_BAR_HEIGHT - 40); // 40 for info panel
     lv_obj_set_pos(gridContainer, 0, TDECK_STATUS_BAR_HEIGHT);
     lv_obj_add_style(gridContainer, LV_CONT_PART_MAIN, &gridStyle);
     lv_cont_set_layout(gridContainer, LV_LAYOUT_GRID);
     lv_cont_set_grid_cell_count(gridContainer, columns);
     
     // Update the grid with app icons
     updateAppGrid();
 }
 
 /**
  * @brief Create the app info panel
  */
 void Launcher::createAppInfoPanel() {
     // Create app info panel container
     appInfoPanel = lv_obj_create(launcherContainer, NULL);
     lv_obj_set_size(appInfoPanel, TDECK_DISPLAY_WIDTH, 40);
     lv_obj_set_pos(appInfoPanel, 0, TDECK_DISPLAY_HEIGHT - 40);
     lv_obj_add_style(appInfoPanel, LV_OBJ_PART_MAIN, &infoBarStyle);
     
     // Create app name label
     appNameLabel = lv_label_create(appInfoPanel, NULL);
     lv_obj_align(appNameLabel, appInfoPanel, LV_ALIGN_IN_LEFT_MID, 10, -8);
     lv_label_set_text(appNameLabel, "");
     
     // Create app description label
     appDescLabel = lv_label_create(appInfoPanel, NULL);
     lv_obj_align(appDescLabel, appInfoPanel, LV_ALIGN_IN_LEFT_MID, 10, 8);
     lv_label_set_text(appDescLabel, "");
     lv_obj_set_style_local_text_color(appDescLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x666666));
     lv_obj_set_style_local_text_font(appDescLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &lv_font_montserrat_12);
     
     // Update info panel with selected app
     updateAppInfoPanel();
 }
 
 /**
  * @brief Update the app grid
  */
 void Launcher::updateAppGrid() {
     // First delete all existing children
     lv_obj_clean(gridContainer);
     
     // Create a button and image for each app
     for (size_t i = 0; i < apps.size(); i++) {
         // Create button
         lv_obj_t* btn = lv_btn_create(gridContainer, NULL);
         lv_obj_set_size(btn, TDECK_APP_ICON_SIZE, TDECK_APP_ICON_SIZE);
         lv_obj_t* img = lv_img_create(btn, NULL);
         lv_img_set_src(img, apps[i].iconDesc);
         lv_obj_align(img, btn, LV_ALIGN_CENTER, 0, 0);
         
         // Set button style based on selection
         if (i == selectedIndex) {
             lv_obj_add_style(btn, LV_BTN_PART_MAIN, &iconStyleSelected);
         } else {
             lv_obj_add_style(btn, LV_BTN_PART_MAIN, &iconStyle);
         }
         
         // Set button click handler
         lv_obj_set_user_data(btn, this);
         lv_obj_set_event_cb(btn, iconClickHandler);
         
         // Store app index in tag
         lv_obj_set_tag(btn, i);
     }
 }
 
 /**
  * @brief Update the status bar
  */
 void Launcher::updateStatusBar() {
     if (!statusBar) return;
     
     // Update clock
     char timeStr[6]; // HH:MM + null terminator
     time_t now;
     struct tm* timeinfo;
     time(&now);
     timeinfo = localtime(&now);
     sprintf(timeStr, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
     lv_label_set_text(statusClock, timeStr);
     
     // Update battery status
     extern PowerManager powerManager; // External reference to power manager
     int batteryPercent = powerManager.getBatteryPercentage();
     
     if (batteryPercent < TDECK_BATTERY_LOW_THRESHOLD) {
         lv_img_set_src(statusBattery, &icon_battery_low);
     } else {
         lv_img_set_src(statusBattery, &icon_battery_full);
     }
     
     // Update WiFi status
     extern WiFiManager wifiManager; // External reference to WiFi manager
     bool wifiConnected = wifiManager.isConnected();
     
     if (wifiConnected) {
         lv_img_set_src(statusWifi, &icon_wifi_connected);
     } else {
         lv_img_set_src(statusWifi, &icon_wifi_disconnected);
     }
     
     // Update Bluetooth status
     extern BluetoothManager btManager; // External reference to BT manager
     bool btActive = btManager.isActive();
     
     if (btActive) {
         lv_img_set_src(statusBluetooth, &icon_bluetooth_on);
     } else {
         lv_img_set_src(statusBluetooth, &icon_bluetooth_off);
     }
 }
 
 /**
  * @brief Update the app info panel
  */
 void Launcher::updateAppInfoPanel() {
     if (!appInfoPanel || selectedIndex >= apps.size()) return;
     
     // Update app name and description
     lv_label_set_text(appNameLabel, apps[selectedIndex].name.c_str());
     lv_label_set_text(appDescLabel, apps[selectedIndex].description.c_str());
 }
 
 /**
  * @brief Calculate grid dimensions based on number of apps
  */
 void Launcher::calculateGridDimensions() {
     // Calculate optimal grid dimensions based on number of apps
     // For now, keep it simple with 3 columns max
     columns = 3;
     rows = (apps.size() + columns - 1) / columns; // Ceiling division
 }
 
 /**
  * @brief Static event handler for icon click events
  */
 void Launcher::iconClickHandler(lv_obj_t* obj, lv_event_t event) {
     if (event != LV_EVENT_CLICKED) return;
     
     // Get launcher instance from user data
     Launcher* launcher = (Launcher*)lv_obj_get_user_data(obj);
     if (!launcher) return;
     
     // Get app index from tag
     uint8_t index = lv_obj_get_tag(obj);
     
     // Select and launch app
     launcher->setSelectedIndex(index);
     launcher->launchApp(index);
 }
 
 /**
  * @brief Periodic timer callback for updating status bar
  */
 void Launcher::statusBarTimerCallback(lv_task_t* task) {
     Launcher* launcher = (Launcher*)task->user_data;
     if (launcher) {
         launcher->updateStatusBar();
     }
 }