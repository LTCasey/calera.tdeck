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
 #include "system/fs_manager.h"
 #include "ui/ui_manager.h"
 #include "apps/launcher.h"
 #include "apps/file_browser.h"
 #include "comms/wifi.h"
 #include "comms/bluetooth.h"
 #include "comms/lora.h"
 
 // LVGL display buffer
 #define DISP_BUF_SIZE (TDECK_DISPLAY_WIDTH * 10)
 static lv_disp_buf_t disp_buf;
 static lv_color_t buf[DISP_BUF_SIZE];
 
 // UI Components
 UIManager uiManager;
 Launcher launcher;
 FileBrowser fileBrowser;
 
 // Hardware Components
 Display display;
 Keyboard keyboard;
 PowerManager powerManager;
 TouchScreen touchScreen;
 WiFiManager wifiManager;
 BluetoothManager btManager;
 LoRaManager loraManager;
 
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
         
         // Delay to yield to other tasks
         delay(TDECK_UI_REFRESH_RATE);
     }
 }
 
 // System Task - handles system monitoring and services
 void systemTask(void *pvParameters) {
     TDECK_LOG_I("System Task started");
     
     while (1) {
         // Check battery status
         powerManager.updateBatteryStatus();
         
         // Check connection statuses
         wifiManager.update();
         btManager.update();
         loraManager.update();
         
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
     touchScreen.init();
     keyboard.init();
     powerManager.init();
     
     // Initialize file system
     if (!fsManager.init()) {
         TDECK_LOG_E("Failed to initialize file system");
     }
     
     // Initialize LVGL
     lv_init();
     
     // Initialize display buffer
     lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);
     
     // Register display driver
     lv_disp_drv_t disp_drv;
     lv_disp_drv_init(&disp_drv);
     disp_drv.flush_cb = display.lvglFlushCb;
     disp_drv.buffer = &disp_buf;
     lv_disp_drv_register(&disp_drv);
     
     // Register input device drivers
     lv_indev_drv_t touch_drv;
     lv_indev_drv_init(&touch_drv);
     touch_drv.type = LV_INDEV_TYPE_POINTER;
     touch_drv.read_cb = touchScreen.lvglReadCb;
     lv_indev_drv_register(&touch_drv);
     
     lv_indev_drv_t kb_drv;
     lv_indev_drv_init(&kb_drv);
     kb_drv.type = LV_INDEV_TYPE_KEYPAD;
     kb_drv.read_cb = keyboard.lvglReadCb;
     lv_indev_drv_register(&kb_drv);
     
     // Initialize UI manager
     uiManager.init();
     
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
     
     // Initialize launcher
     launcher.init(uiManager.getMainContainer());
     
     // Initialize file browser
     fileBrowser.init(uiManager.getMainContainer());
     
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
 
 // Demonstration of accessing the filesystem from main
 void testFileSystem() {
     TDECK_LOG_I("Testing file system operations");
     
     // Create a test file
     const char* testFilePath = "/test_file.txt";
     const char* testData = "This is a test file created by T-Deck UI firmware.\n"
                            "It demonstrates the file system functionality.\n";
     
     if (fsManager.writeFile(testFilePath, testData)) {
         TDECK_LOG_I("Test file created successfully");
         
         // Read the file back
         char buffer[256];
         int bytesRead = fsManager.readFile(testFilePath, buffer, sizeof(buffer));
         
         if (bytesRead > 0) {
             TDECK_LOG_I("Read %d bytes from test file", bytesRead);
             TDECK_LOG_I("Content: %s", buffer);
         } else {
             TDECK_LOG_E("Failed to read test file");
         }
         
         // Get file info
         size_t size;
         time_t modTime;
         if (fsManager.getFileInfo(testFilePath, &size, &modTime)) {
             TDECK_LOG_I("File size: %d bytes", size);
         }
         
         // Delete the file
         if (fsManager.deleteFile(testFilePath)) {
             TDECK_LOG_I("Test file deleted successfully");
         } else {
             TDECK_LOG_E("Failed to delete test file");
         }
     } else {
         TDECK_LOG_E("Failed to create test file");
     }
     
     // Create a directory
     const char* testDirPath = "/test_dir";
     if (fsManager.createDir(testDirPath)) {
         TDECK_LOG_I("Test directory created successfully");
         
         // Create a file in the directory
         const char* testFileInDirPath = "/test_dir/test.txt";
         if (fsManager.writeFile(testFileInDirPath, "File in directory")) {
             TDECK_LOG_I("Created file in test directory");
             
             // List directory contents
             String dirContents;
             if (fsManager.listDir(testDirPath, dirContents)) {
                 TDECK_LOG_I("Directory contents:\n%s", dirContents.c_str());
             }
             
             // Clean up
             fsManager.deleteFile(testFileInDirPath);
         }
         
         // Remove the directory
         if (fsManager.removeDir(testDirPath)) {
             TDECK_LOG_I("Test directory removed successfully");
         } else {
             TDECK_LOG_E("Failed to remove test directory");
         }
     } else {
         TDECK_LOG_E("Failed to create test directory");
     }
     
     // Test JSON configuration
     DynamicJsonDocument doc(256);
     doc["test_value"] = "hello world";
     doc["number"] = 42;
     
     const char* jsonPath = "/test_config.json";
     if (fsManager.saveJsonToFile(jsonPath, doc)) {
         TDECK_LOG_I("JSON configuration saved successfully");
         
         // Clear and reload
         doc.clear();
         if (fsManager.loadJsonFromFile(jsonPath, doc)) {
             String value = doc["test_value"].as<String>();
             int number = doc["number"].as<int>();
             
             TDECK_LOG_I("Loaded JSON config: value='%s', number=%d", value.c_str(), number);
         }
         
         // Clean up
         fsManager.deleteFile(jsonPath);
     }
     
     TDECK_LOG_I("File system test complete");
 }