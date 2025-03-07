/**
 * @file styles.h
 * @brief UI style definitions for T-Deck UI
 * 
 * This file defines additional specialized UI styles that complement
 * the basic theme system. It provides pre-configured styles for
 * common UI patterns and specialized components.
 */

 #ifndef TDECK_STYLES_H
 #define TDECK_STYLES_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include "theme.h"
 #include "../config.h"
 
 /**
  * @class Styles
  * @brief Manages additional UI styles for the T-Deck
  * 
  * This class provides specialized UI styles beyond the basic theme,
  * offering pre-configured styles for common UI patterns and components.
  */
 class Styles {
 public:
     /**
      * @brief Constructor
      */
     Styles();
     
     /**
      * @brief Initialize the styles system
      * @return true if successful, false otherwise
      */
     bool init();
     
     /**
      * @brief Update styles when theme changes
      * @param themeType The new theme type
      */
     void updateForTheme(ThemeType themeType);
     
     /**
      * @brief Apply app icon style to an image
      * @param obj The LVGL image object
      */
     void applyAppIconStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply launcher grid container style
      * @param obj The LVGL grid container object
      */
     void applyLauncherGridStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply status bar icon style
      * @param obj The LVGL icon object in status bar
      */
     void applyStatusBarIconStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply status bar text style
      * @param obj The LVGL label object in status bar
      */
     void applyStatusBarTextStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply table header style
      * @param obj The LVGL table object
      */
     void applyTableHeaderStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply settings item style
      * @param obj The LVGL container for a settings item
      */
     void applySettingsItemStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply battery indicator style
      * @param obj The LVGL battery indicator object
      * @param level Battery level (0-100)
      */
     void applyBatteryStyle(lv_obj_t* obj, uint8_t level);
     
     /**
      * @brief Apply tab style
      * @param obj The LVGL tab object
      * @param isActive Whether this tab is active
      */
     void applyTabStyle(lv_obj_t* obj, bool isActive = false);
     
     /**
      * @brief Apply file/folder icon style
      * @param obj The LVGL icon object
      * @param isFolder Whether this is a folder
      */
     void applyFileIconStyle(lv_obj_t* obj, bool isFolder = false);
     
     /**
      * @brief Apply dialog style to a container
      * @param obj The LVGL container object for a dialog
      */
     void applyDialogStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply keyboard style
      * @param obj The LVGL keyboard object
      */
     void applyKeyboardStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply toast notification style
      * @param obj The LVGL container for a toast notification
      */
     void applyToastStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply chart style
      * @param obj The LVGL chart object
      */
     void applyChartStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply progress bar style
      * @param obj The LVGL progress bar object
      */
     void applyProgressStyle(lv_obj_t* obj);
 
 private:
     /**
      * @brief Initialize all styles
      */
     void initStyles();
     
     // Specialized styles
     lv_style_t style_app_icon;        // App icon style
     lv_style_t style_launcher_grid;   // Launcher grid style
     lv_style_t style_status_bar_icon; // Status bar icon style
     lv_style_t style_status_bar_text; // Status bar text style
     lv_style_t style_table_header;    // Table header style
     lv_style_t style_settings_item;   // Settings item style
     lv_style_t style_battery_normal;  // Battery normal style
     lv_style_t style_battery_low;     // Battery low style
     lv_style_t style_battery_critical; // Battery critical style
     lv_style_t style_tab;             // Tab style
     lv_style_t style_tab_active;      // Active tab style
     lv_style_t style_file_icon;       // File icon style
     lv_style_t style_folder_icon;     // Folder icon style
     lv_style_t style_dialog;          // Dialog style
     lv_style_t style_keyboard;        // Keyboard style
     lv_style_t style_toast;           // Toast notification style
     lv_style_t style_chart;           // Chart style
     lv_style_t style_progress;        // Progress bar style
 };
 
 // Global styles instance
 extern Styles styles;
 
 #endif // TDECK_STYLES_H