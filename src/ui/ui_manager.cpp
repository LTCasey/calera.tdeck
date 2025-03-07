/**
 * @file ui_manager.cpp
 * @brief Implementation of UI Manager for T-Deck
 */

 #include "ui_manager.h"
 #include "../apps/app_base.h"
 #include <ctime>
 
 // Global UI manager instance
 UIManager uiManager;
 
 // Forward declaration of callback for dialog
 static void dialogButtonCallback(lv_obj_t* obj, lv_event_t event);
 
 UIManager::UIManager() : 
     mainScreen(NULL),
     statusBar(NULL),
     contentArea(NULL),
     notificationToast(NULL),
     dialogBox(NULL),
     keyboard(NULL),
     batteryIcon(NULL),
     chargingIcon(NULL),
     wifiIcon(NULL),
     btIcon(NULL),
     loraIcon(NULL),
     clockLabel(NULL),
     currentApp(NULL),
     currentTheme(THEME_LIGHT) {
     // Constructor initializes pointers to NULL
 }
 
 bool UIManager::init() {
     TDECK_LOG_I("Initializing UI Manager");
     
     // Initialize themes and styles
     theme.init();
     styles.init();
     
     // Create main screen
     mainScreen = lv_obj_create(NULL, NULL);
     lv_obj_set_size(mainScreen, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT);
     theme.applyStyle(mainScreen);
     
     // Initialize status bar
     initStatusBar();
     
     // Create content area below status bar
     contentArea = lv_obj_create(mainScreen, NULL);
     lv_obj_set_pos(contentArea, 0, TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_size(contentArea, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT - TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_style_local_bg_opa(contentArea, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
     lv_obj_set_style_local_border_width(contentArea, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
     lv_obj_set_style_local_pad_all(contentArea, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
     
     // Create virtual keyboard (hidden by default)
     keyboard = lv_keyboard_create(mainScreen, NULL);
     styles.applyKeyboardStyle(keyboard);
     lv_obj_set_size(keyboard, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT / 2);
     lv_obj_set_pos(keyboard, 0, TDECK_DISPLAY_HEIGHT);  // Position off-screen
     
     // Load the main screen
     lv_scr_load(mainScreen);
     
     return true;
 }
 
 lv_obj_t* UIManager::getMainContainer() {
     return mainScreen;
 }
 
 lv_obj_t* UIManager::getContentArea() {
     return contentArea;
 }
 
 void UIManager::showApp(AppBase* app) {
     if (!app) return;
     
     // Hide current app if exists
     if (currentApp) {
         currentApp->hide();
     }
     
     // Set and show new app
     currentApp = app;
     currentApp->show();
     
     TDECK_LOG_I("Showing app: %s", app->getName());
 }
 
 void UIManager::showLauncher() {
     // Implemented when Launcher class is built
     // This will be used to return to the home screen
     TDECK_LOG_I("Returning to launcher");
     
     if (currentApp) {
         currentApp->hide();
         currentApp = NULL;
     }
     
     // This will be properly implemented when launcher is built
     // launcher.show();
 }
 
 void UIManager::registerApp(AppBase* app) {
     if (!app) return;
     
     registeredApps.push_back(app);
     TDECK_LOG_I("Registered app: %s", app->getName());
 }
 
 void UIManager::showNotification(const char* message, NotificationType type, uint32_t duration) {
     // Remove existing notification if present
     if (notificationToast) {
         lv_obj_del(notificationToast);
         notificationToast = NULL;
     }
     
     // Create new notification toast
     notificationToast = lv_obj_create(mainScreen, NULL);
     styles.applyToastStyle(notificationToast);
     
     // Set notification color based on type
     const ThemeColors& colors = theme.getColors();
     lv_color_t bgColor;
     
     switch (type) {
         case NOTIFICATION_SUCCESS:
             bgColor = colors.success;
             break;
         case NOTIFICATION_WARNING:
             bgColor = colors.warning;
             break;
         case NOTIFICATION_ERROR:
             bgColor = colors.error;
             break;
         case NOTIFICATION_INFO:
         default:
             bgColor = colors.primary;
             break;
     }
     
     lv_obj_set_style_local_bg_color(notificationToast, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, bgColor);
     
     // Set size based on text content
     lv_obj_set_width(notificationToast, TDECK_DISPLAY_WIDTH - 40);
     
     // Create message label
     lv_obj_t* label = lv_label_create(notificationToast, NULL);
     lv_label_set_text(label, message);
     lv_obj_set_style_local_text_color(label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
     lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
     
     // Position toast at bottom of screen
     lv_obj_set_auto_realign(notificationToast, true);
     
     // Calculate height based on text
     lv_obj_set_height(notificationToast, LV_MATH_MAX(40, lv_obj_get_height(label) + 20));
     
     // Position at bottom of screen with animation
     lv_obj_set_pos(notificationToast, 20, TDECK_DISPLAY_HEIGHT + 10);
     lv_obj_align(notificationToast, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 10);
     
     // Create animation
     lv_anim_t a;
     lv_anim_init(&a);
     lv_anim_set_var(&a, notificationToast);
     lv_anim_set_values(&a, TDECK_DISPLAY_HEIGHT + 10, TDECK_DISPLAY_HEIGHT - lv_obj_get_height(notificationToast) - 10);
     lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
     lv_anim_set_time(&a, 300);
     lv_anim_start(&a);
     
     // Set timer to hide notification
     lv_timer_t* timer = lv_timer_create(hideNotificationCallback, duration, this);
     lv_timer_set_repeat_count(timer, 1);
 }
 
 void UIManager::hideNotificationCallback(lv_timer_t* timer) {
     UIManager* self = (UIManager*)timer->user_data;
     
     if (self->notificationToast) {
         // Create animation to slide out
         lv_anim_t a;
         lv_anim_init(&a);
         lv_anim_set_var(&a, self->notificationToast);
         lv_anim_set_values(&a, lv_obj_get_y(self->notificationToast), TDECK_DISPLAY_HEIGHT + 10);
         lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
         lv_anim_set_time(&a, 300);
         lv_anim_set_ready_cb(&a, [](lv_anim_t* a) {
             lv_obj_del((lv_obj_t*)a->var);
             ((UIManager*)a->user_data)->notificationToast = NULL;
         });
         lv_anim_set_user_data(&a, self);
         lv_anim_start(&a);
     }
 }
 
 void UIManager::showDialog(const char* title, const char* message, 
                           const char* confirmLabel, const char* cancelLabel,
                           std::function<void(bool)> callback) {
     // Remove existing dialog if present
     if (dialogBox) {
         lv_obj_del(dialogBox);
         dialogBox = NULL;
     }
     
     // Create new dialog box
     dialogBox = lv_obj_create(mainScreen, NULL);
     styles.applyDialogStyle(dialogBox);
     
     // Set dialog size
     int dialogWidth = TDECK_DISPLAY_WIDTH * 2 / 3;
     lv_obj_set_size(dialogBox, dialogWidth, LV_SIZE_CONTENT);
     
     // Create title label
     lv_obj_t* titleLabel = lv_label_create(dialogBox, NULL);
     theme.applyTitleStyle(titleLabel);
     lv_label_set_text(titleLabel, title);
     lv_obj_align(titleLabel, dialogBox, LV_ALIGN_IN_TOP_MID, 0, 10);
     
     // Create message label
     lv_obj_t* msgLabel = lv_label_create(dialogBox, NULL);
     lv_label_set_text(msgLabel, message);
     lv_label_set_long_mode(msgLabel, LV_LABEL_LONG_BREAK);
     lv_obj_set_width(msgLabel, dialogWidth - 40);
     lv_obj_align(msgLabel, titleLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
     
     // Create buttons container
     lv_obj_t* btnContainer = lv_cont_create(dialogBox, NULL);
     lv_obj_set_size(btnContainer, dialogWidth - 40, 50);
     lv_obj_set_style_local_bg_opa(btnContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
     lv_obj_set_style_local_border_width(btnContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
     lv_obj_set_style_local_pad_inner(btnContainer, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
     lv_obj_align(btnContainer, msgLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
     
     // Store callback for later
     static std::function<void(bool)> dialogCallback;
     dialogCallback = callback;
     
     // Create buttons based on parameters
     if (confirmLabel) {
         lv_obj_t* confirmBtn = lv_btn_create(btnContainer, NULL);
         theme.applyButtonStyle(confirmBtn, true); // Primary style
         lv_obj_set_event_cb(confirmBtn, dialogButtonCallback);
         lv_obj_set_user_data(confirmBtn, (void*)true); // Store result for callback
         
         lv_obj_t* confirmBtnLabel = lv_label_create(confirmBtn, NULL);
         lv_label_set_text(confirmBtnLabel, confirmLabel);
         
         if (cancelLabel) {
             // If both buttons, set half width
             lv_obj_set_size(confirmBtn, (dialogWidth - 60) / 2, 40);
             lv_obj_align(confirmBtn, btnContainer, LV_ALIGN_IN_LEFT_MID, 0, 0);
         } else {
             // If only confirm button, center it
             lv_obj_set_size(confirmBtn, dialogWidth - 80, 40);
             lv_obj_align(confirmBtn, btnContainer, LV_ALIGN_CENTER, 0, 0);
         }
     }
     
     if (cancelLabel) {
         lv_obj_t* cancelBtn = lv_btn_create(btnContainer, NULL);
         theme.applyButtonStyle(cancelBtn, false); // Secondary style
         lv_obj_set_event_cb(cancelBtn, dialogButtonCallback);
         lv_obj_set_user_data(cancelBtn, (void*)false); // Store result for callback
         
         lv_obj_t* cancelBtnLabel = lv_label_create(cancelBtn, NULL);
         lv_label_set_text(cancelBtnLabel, cancelLabel);
         
         if (confirmLabel) {
             // If both buttons, set half width
             lv_obj_set_size(cancelBtn, (dialogWidth - 60) / 2, 40);
             lv_obj_align(cancelBtn, btnContainer, LV_ALIGN_IN_RIGHT_MID, 0, 0);
         } else {
             // If only cancel button, center it
             lv_obj_set_size(cancelBtn, dialogWidth - 80, 40);
             lv_obj_align(cancelBtn, btnContainer, LV_ALIGN_CENTER, 0, 0);
         }
     }
     
     // Calculate dialog height and position in center of screen
     lv_obj_update_layout(dialogBox);
     int dialogHeight = lv_obj_get_height(dialogBox);
     lv_obj_set_height(dialogBox, dialogHeight);
     lv_obj_align(dialogBox, NULL, LV_ALIGN_CENTER, 0, 0);
     
     // Add fade background
     static lv_style_t bgStyle;
     lv_style_init(&bgStyle);
     lv_style_set_bg_color(&bgStyle, LV_STATE_DEFAULT, LV_COLOR_BLACK);
     lv_style_set_bg_opa(&bgStyle, LV_STATE_DEFAULT, LV_OPA_50);
     
     lv_obj_t* modalBg = lv_obj_create(mainScreen, NULL);
     lv_obj_reset_style_list(modalBg, LV_OBJ_PART_MAIN);
     lv_obj_add_style(modalBg, LV_OBJ_PART_MAIN, &bgStyle);
     lv_obj_set_size(modalBg, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT);
     lv_obj_set_pos(modalBg, 0, 0);
     lv_obj_set_event_cb(modalBg, [](lv_obj_t* obj, lv_event_t event) {
         // Prevent clicks behind dialog
         if (event == LV_EVENT_CLICKED) {
             // Optionally handle outside click here
         }
     });
     
     // Move modal bg behind dialog
     lv_obj_move_foreground(dialogBox);
 }
 
 // Dialog button event callback
 static void dialogButtonCallback(lv_obj_t* obj, lv_event_t event) {
     if (event == LV_EVENT_CLICKED) {
         // Get result from user data (true for confirm, false for cancel)
         bool result = (bool)lv_obj_get_user_data(obj);
         
         // Find parent dialog to close it
         lv_obj_t* dialog = lv_obj_get_parent(lv_obj_get_parent(obj));
         lv_obj_t* modalBg = lv_obj_get_child_back(lv_scr_act(), NULL);
         
         // Delete modal background and dialog
         if (modalBg && lv_obj_get_width(modalBg) == TDECK_DISPLAY_WIDTH) {
             lv_obj_del(modalBg);
         }
         lv_obj_del(dialog);
         
         // Get the stored callback and call it
         static std::function<void(bool)>* dialogCallback = NULL;
         if (dialogCallback && *dialogCallback) {
             (*dialogCallback)(result);
         }
         
         // Clear the dialog reference
         uiManager.dialogBox = NULL;
     }
 }
 
 void UIManager::updateWiFiStatus(bool connected, uint8_t strength) {
     if (!wifiIcon) return;
     
     // Set proper icon based on state
     if (connected) {
         // Here you would set different icon symbols based on strength
         // For simplicity, just showing connected vs not connected
         // In a real implementation you'd use different icons for signal levels
         lv_obj_set_hidden(wifiIcon, false);
     } else {
         // Not connected
         lv_obj_set_hidden(wifiIcon, true);
     }
 }
 
 void UIManager::updateBluetoothStatus(bool active, bool connected) {
     if (!btIcon) return;
     
     // Set proper icon visibility based on state
     if (active) {
         lv_obj_set_hidden(btIcon, false);
     } else {
         lv_obj_set_hidden(btIcon, true);
     }
 }
 
 void UIManager::updateLoRaStatus(bool active, uint8_t signal) {
     if (!loraIcon) return;
     
     // Set proper icon visibility based on state
     if (active) {
         lv_obj_set_hidden(loraIcon, false);
     } else {
         lv_obj_set_hidden(loraIcon, true);
     }
 }
 
 void UIManager::updateBatteryStatus(uint8_t percentage, bool charging) {
     if (!batteryIcon || !chargingIcon) return;
     
     // Update battery icon level
     // Here you would adjust the battery icon appearance
     // based on percentage
     
     // Show/hide charging icon
     lv_obj_set_hidden(chargingIcon, !charging);
 }
 
 void UIManager::updateClock(uint8_t hour, uint8_t minute) {
     if (!clockLabel) return;
     
     // Format and update clock display
     char timeStr[6]; // HH:MM\0
     snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
     lv_label_set_text(clockLabel, timeStr);
 }
 
 void UIManager::toggleTheme() {
     // Toggle between light and dark themes
     currentTheme = (currentTheme == THEME_LIGHT) ? THEME_DARK : THEME_LIGHT;
     
     // Update theme and styles
     theme.setTheme(currentTheme);
     styles.updateForTheme(currentTheme);
     
     TDECK_LOG_I("Theme switched to %s", (currentTheme == THEME_DARK) ? "dark" : "light");
     
     // Show notification about theme change
     const char* message = (currentTheme == THEME_DARK) ? 
         "Switched to dark theme" : "Switched to light theme";
     showNotification(message, NOTIFICATION_INFO, 2000);
 }
 
 ThemeType UIManager::getCurrentTheme() const {
     return currentTheme;
 }
 
 void UIManager::showKeyboard(lv_obj_t* textArea) {
     if (!keyboard) return;
     
     // Set target text area
     lv_keyboard_set_textarea(keyboard, textArea);
     
     // Animate keyboard into view
     lv_anim_t a;
     lv_anim_init(&a);
     lv_anim_set_var(&a, keyboard);
     lv_anim_set_values(&a, TDECK_DISPLAY_HEIGHT, TDECK_DISPLAY_HEIGHT / 2);
     lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
     lv_anim_set_time(&a, 300);
     lv_anim_start(&a);
 }
 
 void UIManager::hideKeyboard() {
     if (!keyboard) return;
     
     // Animate keyboard out of view
     lv_anim_t a;
     lv_anim_init(&a);
     lv_anim_set_var(&a, keyboard);
     lv_anim_set_values(&a, lv_obj_get_y(keyboard), TDECK_DISPLAY_HEIGHT);
     lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
     lv_anim_set_time(&a, 300);
     lv_anim_start(&a);
 }
 
 void UIManager::update() {
     // Update clock every minute
     static uint32_t lastClockUpdate = 0;
     uint32_t currentTime = millis();
     
     if (currentTime - lastClockUpdate > 60000) { // Update clock every minute
         // Get current time
         time_t now = time(NULL);
         struct tm* timeinfo = localtime(&now);
         
         updateClock(timeinfo->tm_hour, timeinfo->tm_min);
         lastClockUpdate = currentTime;
     }
     
     // Other periodic UI updates can go here
 }
 
 void UIManager::initStatusBar() {
     // Create status bar container
     statusBar = lv_obj_create(mainScreen, NULL);
     lv_obj_set_size(statusBar, TDECK_DISPLAY_WIDTH, TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_pos(statusBar, 0, 0);
     theme.applyStatusBarStyle(statusBar);
     
     // Create status icons
     createStatusIcons();
     
     // Create clock label
     clockLabel = lv_label_create(statusBar, NULL);
     styles.applyStatusBarTextStyle(clockLabel);
     lv_label_set_text(clockLabel, "00:00");
     lv_obj_align(clockLabel, NULL, LV_ALIGN_IN_RIGHT_MID, -10, 0);
     
     // Initialize with current time
     time_t now = time(NULL);
     struct tm* timeinfo = localtime(&now);
     updateClock(timeinfo->tm_hour, timeinfo->tm_min);
 }
 
 void UIManager::createStatusIcons() {
     // Initialize status icons
     // These would typically be image objects with icon graphics
     // For now we'll use simple objects as placeholders
     
     int iconSize = TDECK_STATUS_BAR_HEIGHT - 8;
     int iconSpacing = 5;
     int rightMargin = 50; // Space for clock
     
     // Battery icon (rightmost)
     batteryIcon = lv_obj_create(statusBar, NULL);
     lv_obj_set_size(batteryIcon, iconSize + 5, iconSize - 2);
     lv_obj_align(batteryIcon, NULL, LV_ALIGN_IN_RIGHT_MID, -rightMargin, 0);
     styles.applyBatteryStyle(batteryIcon, 100);
     
     // Charging icon
     chargingIcon = lv_obj_create(statusBar, NULL);
     lv_obj_set_size(chargingIcon, iconSize, iconSize);
     lv_obj_align(chargingIcon, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -iconSpacing, 0);
     lv_obj_set_hidden(chargingIcon, true); // Hidden by default
     
     // WiFi icon
     wifiIcon = lv_obj_create(statusBar, NULL);
     lv_obj_set_size(wifiIcon, iconSize, iconSize);
     lv_obj_align(wifiIcon, chargingIcon, LV_ALIGN_OUT_LEFT_MID, -iconSpacing, 0);
     
     // Bluetooth icon
     btIcon = lv_obj_create(statusBar, NULL);
     lv_obj_set_size(btIcon, iconSize, iconSize);
     lv_obj_align(btIcon, wifiIcon, LV_ALIGN_OUT_LEFT_MID, -iconSpacing, 0);
     
     // LoRa icon
     loraIcon = lv_obj_create(statusBar, NULL);
     lv_obj_set_size(loraIcon, iconSize, iconSize);
     lv_obj_align(loraIcon, btIcon, LV_ALIGN_OUT_LEFT_MID, -iconSpacing, 0);
     
     // Apply status bar icon styles
     styles.applyStatusBarIconStyle(wifiIcon);
     styles.applyStatusBarIconStyle(btIcon);
     styles.applyStatusBarIconStyle(loraIcon);
     styles.applyStatusBarIconStyle(chargingIcon);
     
     // Initially hide connection icons until their status is updated
     lv_obj_set_hidden(wifiIcon, true);
     lv_obj_set_hidden(btIcon, true);
     lv_obj_set_hidden(loraIcon, true);