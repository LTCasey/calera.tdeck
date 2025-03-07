/**
 * @file ui_manager.h
 * @brief UI initialization and management for T-Deck
 * 
 * This file defines the UI Manager which coordinates UI initialization,
 * screen transitions, and handles the status bar and other global UI elements.
 */

 #ifndef TDECK_UI_MANAGER_H
 #define TDECK_UI_MANAGER_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include <vector>
 #include <functional>
 #include <string>
 #include "../config.h"
 #include "theme.h"
 #include "styles.h"
 
 // Forward declarations
 class AppBase;
 
 /**
  * @brief UI notification type
  */
 typedef enum {
     NOTIFICATION_INFO,    // Informational notification
     NOTIFICATION_SUCCESS, // Success notification
     NOTIFICATION_WARNING, // Warning notification
     NOTIFICATION_ERROR    // Error notification
 } NotificationType;
 
 /**
  * @class UIManager
  * @brief Manages the UI initialization and navigation
  * 
  * This class handles the overall UI setup, screen transitions,
  * status bar management, and other global UI elements.
  */
 class UIManager {
 public:
     /**
      * @brief Constructor
      */
     UIManager();
     
     /**
      * @brief Initialize the UI system
      * @return true if successful, false otherwise
      */
     bool init();
     
     /**
      * @brief Get main screen container
      * @return Pointer to the main screen container object
      */
     lv_obj_t* getMainContainer();
     
     /**
      * @brief Get content area (below status bar)
      * @return Pointer to the content area container
      */
     lv_obj_t* getContentArea();
     
     /**
      * @brief Show a specific app screen
      * @param app Pointer to the app to display
      */
     void showApp(AppBase* app);
     
     /**
      * @brief Return to the launcher screen
      */
     void showLauncher();
     
     /**
      * @brief Register an app with the UI manager
      * @param app Pointer to the app to register
      */
     void registerApp(AppBase* app);
     
     /**
      * @brief Show a notification toast
      * @param message The notification message
      * @param type The notification type
      * @param duration Duration to show the notification in milliseconds
      */
     void showNotification(const char* message, NotificationType type = NOTIFICATION_INFO, uint32_t duration = 3000);
     
     /**
      * @brief Show a dialog box
      * @param title Dialog title
      * @param message Dialog message
      * @param confirmLabel Label for confirm button, NULL for no button
      * @param cancelLabel Label for cancel button, NULL for no button
      * @param callback Function to call when dialog is dismissed
      */
     void showDialog(const char* title, const char* message, 
                     const char* confirmLabel, const char* cancelLabel,
                     std::function<void(bool)> callback);
     
     /**
      * @brief Update WiFi status icon
      * @param connected Whether WiFi is connected
      * @param strength WiFi signal strength (0-100)
      */
     void updateWiFiStatus(bool connected, uint8_t strength = 0);
     
     /**
      * @brief Update Bluetooth status icon
      * @param active Whether Bluetooth is active
      * @param connected Whether a device is connected
      */
     void updateBluetoothStatus(bool active, bool connected = false);
     
     /**
      * @brief Update LoRa status icon
      * @param active Whether LoRa is active
      * @param signal Signal strength (0-100)
      */
     void updateLoRaStatus(bool active, uint8_t signal = 0);
     
     /**
      * @brief Update battery status display
      * @param percentage Battery percentage (0-100)
      * @param charging Whether the battery is charging
      */
     void updateBatteryStatus(uint8_t percentage, bool charging = false);
     
     /**
      * @brief Update clock in status bar
      * @param hour Hour (0-23)
      * @param minute Minute (0-59)
      */
     void updateClock(uint8_t hour, uint8_t minute);
     
     /**
      * @brief Toggle between light and dark themes
      */
     void toggleTheme();
     
     /**
      * @brief Get current theme type
      * @return The current theme type
      */
     ThemeType getCurrentTheme() const;
     
     /**
      * @brief Show the virtual keyboard
      * @param textArea Text area to input to
      */
     void showKeyboard(lv_obj_t* textArea);
     
     /**
      * @brief Hide the virtual keyboard
      */
     void hideKeyboard();
     
     /**
      * @brief Process UI events
      * This should be called regularly from the UI task
      */
     void update();
 
 private:
     /**
      * @brief Initialize the status bar
      */
     void initStatusBar();
     
     /**
      * @brief Create status icons
      */
     void createStatusIcons();
     
     /**
      * @brief Toast notification timer callback
      * @param timer LVGL timer instance
      */
     static void hideNotificationCallback(lv_timer_t* timer);
 
     // LVGL objects
     lv_obj_t* mainScreen;       // Main screen object
     lv_obj_t* statusBar;        // Status bar container
     lv_obj_t* contentArea;      // Content area container
     lv_obj_t* notificationToast; // Notification toast container
     lv_obj_t* dialogBox;        // Dialog box container
     lv_obj_t* keyboard;         // Virtual keyboard
     
     // Status bar elements
     lv_obj_t* batteryIcon;      // Battery level indicator
     lv_obj_t* chargingIcon;     // Charging indicator
     lv_obj_t* wifiIcon;         // WiFi status indicator
     lv_obj_t* btIcon;           // Bluetooth status indicator
     lv_obj_t* loraIcon;         // LoRa status indicator
     lv_obj_t* clockLabel;       // Clock display
     
     // App management
     std::vector<AppBase*> registeredApps; // List of registered applications
     AppBase* currentApp;        // Currently active app
     
     // Theme state
     ThemeType currentTheme;     // Current theme type
 };
 
 // Global UI manager instance
 extern UIManager uiManager;
 
 #endif // TDECK_UI_MANAGER_H