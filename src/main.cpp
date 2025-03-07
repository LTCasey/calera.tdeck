/**
 * @file main.cpp
 * @brief Main entry point for T-Deck UI Firmware
 */

 #include <Arduino.h>
 #include <lvgl.h>
 #include "config.h"
 #include "hal/display.h"
 #include "hal/keyboard.h"
 #include "hal/power.h"
 #include "hal/touchscreen.h"
 #include "hal/sdcard.h"
 #include "hal/audio.h"
 #include "hal/lora.h"
 #include "system/fs_manager.h"
 #include "system/battery.h"
 #include "system/config_storage.h"
 #include "system/ota.h"
 #include "ui/ui_manager.h"
 #include "ui/theme.h"
 #include "ui/styles.h"
 #include "apps/launcher.h"
 #include "apps/file_browser.h"
 #include "apps/wifi_manager.h"
 #include "apps/ble_manager.h"
 #include "apps/settings.h"
 #include "apps/system_info.h"
 #include "apps/terminal.h"
 #include "apps/lora_messenger.h"
 #include "comms/wifi.h"
 #include "comms/bluetooth.h"
 #include "comms/lora.h"
 
 // LVGL display buffer
 static lv_disp_draw_buf_t disp_buf;
 static lv_color_t buf[TDECK_DISPLAY_WIDTH * 10];
 
 // Hardware Managers
 Display display;
 TDeckKeyboard keyboard;
 Power powerManager;
 TouchScreen touchScreen;
 BatteryManager batteryManager;
 
 // System Services
 FSManager fsManager;
 ConfigStorage configStorage;
 OTAManager otaManager;
 
 // Communication Managers
 WiFiManager wifiManager;
 BluetoothManager btManager;
 LoRaManager loraManager;
 
 // UI Components
 UIManager uiManager;
 Launcher launcher;
 FileBrowser fileBrowser;
 Settings settings;
 Terminal terminal;
 SystemInfo systemInfo;
 WiFiManagerApp wifiManagerApp;
 BLEManagerApp bleManagerApp;
 LoRaMessenger loraMessenger;
 
 // Task handles
 TaskHandle_t uiTaskHandle = NULL;
 TaskHandle_t systemTaskHandle = NULL;
 
 // UI Task - handles LVGL UI updates
 void uiTask(void *pvParameters) {
     TDECK_LOG_I("UI Task started");
     
     while (1) {
         // Call LVGL task handler
         lv_task_handler();
         
         // Handle touch events
         touchScreen.update();
         
         // Handle keyboard events
         keyboard.update();
         
         // Update UI Manager (clock, notifications, etc.)
         uiManager.update();
         
         // Delay to yield to other tasks
         delay(TDECK_UI_REFRESH_RATE);
     }
 }
 
 // System Task - handles system monitoring and services
 void systemTask(void *pvParameters) {
     TDECK_LOG_I("System Task started");
     
     while (1) {
         // Check battery status
         batteryManager.update();
         
         // Update power manager
         powerManager.update();
         
         // Check OTA updates
         if (TDECK_FEATURE_OTA) {
             otaManager.update();
         }
         
         // Check connection statuses
         if (TDECK_FEATURE_WIFI) {
             wifiManager.update();
         }
         
         if (TDECK_FEATURE_BLUETOOTH) {
             btManager.update();
         }
         
         if (TDECK_FEATURE_LORA) {
             loraManager.update();
         }
         
         // Delay for system monitoring (slower than UI updates)
         delay(1000);
     }
 }
 
 void setup() {
     // Initialize serial for debugging
     TDECK_DEBUG_SERIAL.begin(115200);
     TDECK_LOG_I("T-Deck UI Firmware v%s starting...", TDECK_FIRMWARE_VERSION);
     
     // Initialize hardware components
     display.init();
     touchScreen.begin();
     keyboard.init();
     powerManager.begin();
     batteryManager.init();
     
     if (TDECK_FEATURE_SD_CARD) {
         sdcard.begin();
     }
     
     // Initialize file system and config storage
     if (!fsManager.init()) {
         TDECK_LOG_E("Failed to initialize file system");
     }
     
     configStorage.init();
     
     // Initialize LVGL
     lv_init();
     
     // Initialize display buffer
     lv_disp_draw_buf_init(&disp_buf, buf, NULL, TDECK_DISPLAY_WIDTH * 10);
     
     // Register display driver
     lv_disp_drv_t disp_drv;
     lv_disp_drv_init(&disp_drv);
     disp_drv.flush_cb = Display::flush_cb;
     disp_drv.draw_buf = &disp_buf;
     disp_drv.hor_res = TDECK_DISPLAY_WIDTH;
     disp_drv.ver_res = TDECK_DISPLAY_HEIGHT;
     lv_disp_drv_register(&disp_drv);
     
     // Register touch input device
     lv_indev_drv_t touch_drv;
     lv_indev_drv_init(&touch_drv);
     touch_drv.type = LV_INDEV_TYPE_POINTER;
     touch_drv.read_cb = TouchScreen::read_cb;
     lv_indev_drv_register(&touch_drv);
     
     // Register keyboard input device (if enabled)
     if (TDECK_FEATURE_KEYBOARD) {
         lv_indev_drv_t kb_drv;
         lv_indev_drv_init(&kb_drv);
         kb_drv.type = LV_INDEV_TYPE_KEYPAD;
         kb_drv.read_cb = TDeckKeyboard::read_cb;
         lv_indev_drv_register(&kb_drv);
     }
     
     // Initialize UI manager
     uiManager.init();
     
     // Initialize theme and styles
     theme.init();
     styles.init();
     
     // Initialize communication modules
     if (TDECK_FEATURE_WIFI) {
         wifiManager.init();
     }
     
     if (TDECK_FEATURE_BLUETOOTH) {
         btManager.init();
     }
     
     if (TDECK_FEATURE_LORA) {
         loraManager.init();
     }
     
     if (TDECK_FEATURE_OTA) {
         otaManager.init();
     }
     
     // Initialize applications
     launcher.init(uiManager.getMainContainer());
     fileBrowser.init();
     settings.init(uiManager.getMainContainer());
     terminal.init(uiManager.getMainContainer());
     systemInfo.init(uiManager.getMainContainer());
     wifiManagerApp.init(uiManager.getMainContainer());
     bleManagerApp.init(uiManager.getMainContainer());
     loraMessenger.init(uiManager.getMainContainer());
     
     // Register apps with launcher
     // The launcher.registerApp calls should go here based on 
     // the app function declarations seen in src/apps/launcher.cpp
     
     // Start UI task
     xTaskCreatePinnedToCore(
         uiTask,                    // Task function
         "UI_Task",                 // Name
         TDECK_UI_TASK_STACK_SIZE,  // Stack size
         NULL,                      // Parameters
         TDECK_UI_TASK_PRIORITY,    // Priority
         &uiTaskHandle,             // Task handle
         1                          // Core (1 = Arduino core)
     );
     
     // Start system task
     xTaskCreatePinnedToCore(
         systemTask,                    // Task function
         "System_Task",                 // Name
         TDECK_SYSTEM_TASK_STACK_SIZE,  // Stack size
         NULL,                          // Parameters
         TDECK_SYSTEM_TASK_PRIORITY,    // Priority
         &systemTaskHandle,             // Task handle
         0                              // Core (0 = Free RTOS core)
     );
     
     // Show launcher as default app
     launcher.start();
     
     TDECK_LOG_I("Setup complete");
 }
 
 void loop() {
     // Main loop is empty as tasks handle everything
     delay(1000);
 }