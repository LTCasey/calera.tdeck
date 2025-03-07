/**
 * @file theme.h
 * @brief Theme definitions for T-Deck UI
 * 
 * This file defines the theme system for the T-Deck UI, providing light and dark themes
 * with consistent styling across all UI elements.
 */

 #ifndef TDECK_THEME_H
 #define TDECK_THEME_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include "../config.h"
 
 /**
  * @brief Theme type enumeration
  */
 typedef enum {
     THEME_LIGHT,    // Light theme with white background
     THEME_DARK      // Dark theme with dark background
 } ThemeType;
 
 /**
  * @brief Theme color palette structure
  */
 typedef struct {
     lv_color_t bg;           // Background color
     lv_color_t bg_alt;       // Alternative background color
     lv_color_t surface;      // Surface color for cards and containers
     lv_color_t primary;      // Primary color for important elements
     lv_color_t secondary;    // Secondary color for less important elements
     lv_color_t accent;       // Accent color for highlights
     lv_color_t text;         // Main text color
     lv_color_t text_secondary; // Secondary text color
     lv_color_t border;       // Border color
     lv_color_t disabled;     // Color for disabled elements
     lv_color_t success;      // Success indicator color
     lv_color_t warning;      // Warning indicator color
     lv_color_t error;        // Error indicator color
 } ThemeColors;
 
 /**
  * @class Theme
  * @brief Manages UI themes for the T-Deck
  * 
  * This class handles the creation and application of UI themes,
  * providing consistent styling across the application.
  */
 class Theme {
 public:
     /**
      * @brief Constructor
      */
     Theme();
     
     /**
      * @brief Initialize the theme system
      * @return true if successful, false otherwise
      */
     bool init();
     
     /**
      * @brief Set the active theme
      * @param type The theme type to set
      */
     void setTheme(ThemeType type);
     
     /**
      * @brief Get the current theme type
      * @return The current theme type
      */
     ThemeType getCurrentTheme() const;
     
     /**
      * @brief Get the current theme colors
      * @return Reference to the current theme colors
      */
     const ThemeColors& getColors() const;
     
     /**
      * @brief Apply default style to a widget
      * @param obj The LVGL object to style
      */
     void applyStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply card style to a container widget
      * @param obj The LVGL object to style as a card
      */
     void applyCardStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply button style to a widget
      * @param obj The LVGL object to style as a button
      * @param isPrimary Whether to use primary color (true) or secondary (false)
      */
     void applyButtonStyle(lv_obj_t* obj, bool isPrimary = true);
     
     /**
      * @brief Apply title style to a label
      * @param obj The LVGL label object to style as a title
      */
     void applyTitleStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply text field style to an input widget
      * @param obj The LVGL object to style as a text field
      */
     void applyTextFieldStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply switch style to a switch widget
      * @param obj The LVGL switch object to style
      */
     void applySwitchStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply slider style to a slider widget
      * @param obj The LVGL slider object to style
      */
     void applySliderStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply list style to a list widget
      * @param obj The LVGL list object to style
      */
     void applyListStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply list item style to a list button
      * @param obj The LVGL button object within a list to style
      */
     void applyListItemStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply dropdown style to a dropdown widget
      * @param obj The LVGL dropdown object to style
      */
     void applyDropdownStyle(lv_obj_t* obj);
     
     /**
      * @brief Apply status bar style to the status bar container
      * @param obj The LVGL object to style as status bar
      */
     void applyStatusBarStyle(lv_obj_t* obj);
 
 private:
     /**
      * @brief Initialize theme styles
      */
     void initStyles();
     
     /**
      * @brief Update theme colors based on the current theme type
      */
     void updateColors();
 
     ThemeType currentTheme;       // Current theme type
     ThemeColors colors;           // Current theme colors
     
     // Basic styles
     lv_style_t style_bg;          // Background style
     lv_style_t style_card;        // Card style
     lv_style_t style_btn;         // Button style
     lv_style_t style_btn_pressed; // Pressed button style
     lv_style_t style_btn_toggle;  // Toggle button style
     lv_style_t style_title;       // Title text style
     lv_style_t style_text;        // Normal text style
     lv_style_t style_text_secondary; // Secondary text style
     lv_style_t style_textfield_focused; // Focused text field style
     lv_style_t style_textfield;   // Normal text field style
     lv_style_t style_switch;      // Switch style
     lv_style_t style_slider;      // Slider style
     lv_style_t style_list;        // List style
     lv_style_t style_list_item;   // List item style
     lv_style_t style_dropdown;    // Dropdown style
     lv_style_t style_status_bar;  // Status bar style
 };
 
 // Global theme instance
 extern Theme theme;
 
 #endif // TDECK_THEME_H