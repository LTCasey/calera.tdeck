/**
 * @file theme.cpp
 * @brief Implementation of theme system for T-Deck UI
 */

 #include "theme.h"

 // Global theme instance
 Theme theme;
 
 // Define some color constants
 #define COLOR_WHITE        lv_color_make(0xFF, 0xFF, 0xFF)
 #define COLOR_BLACK        lv_color_make(0x00, 0x00, 0x00)
 #define COLOR_GRAY         lv_color_make(0x80, 0x80, 0x80)
 #define COLOR_LIGHT_GRAY   lv_color_make(0xE0, 0xE0, 0xE0)
 #define COLOR_DARK_GRAY    lv_color_make(0x30, 0x30, 0x30)
 #define COLOR_BLUE         lv_color_make(0x21, 0x96, 0xF3)
 #define COLOR_DARK_BLUE    lv_color_make(0x18, 0x76, 0xD2)
 #define COLOR_PURPLE       lv_color_make(0x9C, 0x27, 0xB0)
 #define COLOR_GREEN        lv_color_make(0x4C, 0xAF, 0x50)
 #define COLOR_RED          lv_color_make(0xF4, 0x43, 0x36)
 #define COLOR_ORANGE       lv_color_make(0xFF, 0x98, 0x00)
 #define COLOR_CYAN         lv_color_make(0x00, 0xBC, 0xD4)
 
 Theme::Theme() : currentTheme(THEME_LIGHT) {
     // Constructor initializes with light theme by default
 }
 
 bool Theme::init() {
     TDECK_LOG_I("Initializing theme system");
     
     // Initialize styles
     initStyles();
     
     // Set default theme colors
     updateColors();
     
     return true;
 }
 
 void Theme::setTheme(ThemeType type) {
     if (currentTheme != type) {
         TDECK_LOG_I("Changing theme to %s", type == THEME_DARK ? "dark" : "light");
         currentTheme = type;
         updateColors();
         initStyles(); // Reinitialize styles with new colors
     }
 }
 
 ThemeType Theme::getCurrentTheme() const {
     return currentTheme;
 }
 
 const ThemeColors& Theme::getColors() const {
     return colors;
 }
 
 void Theme::updateColors() {
     if (currentTheme == THEME_DARK) {
         // Dark theme palette
         colors.bg = COLOR_BLACK;
         colors.bg_alt = COLOR_DARK_GRAY;
         colors.surface = lv_color_make(0x12, 0x12, 0x12);
         colors.primary = COLOR_BLUE;
         colors.secondary = COLOR_PURPLE;
         colors.accent = COLOR_CYAN;
         colors.text = COLOR_WHITE;
         colors.text_secondary = COLOR_LIGHT_GRAY;
         colors.border = lv_color_make(0x40, 0x40, 0x40);
         colors.disabled = lv_color_make(0x60, 0x60, 0x60);
         colors.success = COLOR_GREEN;
         colors.warning = COLOR_ORANGE;
         colors.error = COLOR_RED;
     } else {
         // Light theme palette
         colors.bg = COLOR_WHITE;
         colors.bg_alt = COLOR_LIGHT_GRAY;
         colors.surface = lv_color_make(0xF5, 0xF5, 0xF5);
         colors.primary = COLOR_BLUE;
         colors.secondary = COLOR_PURPLE;
         colors.accent = COLOR_CYAN;
         colors.text = COLOR_BLACK;
         colors.text_secondary = COLOR_GRAY;
         colors.border = lv_color_make(0xD0, 0xD0, 0xD0);
         colors.disabled = lv_color_make(0xA0, 0xA0, 0xA0);
         colors.success = COLOR_GREEN;
         colors.warning = COLOR_ORANGE;
         colors.error = COLOR_RED;
     }
 }
 
 void Theme::initStyles() {
     // Initialize all styles with the current theme colors
     
     // Background style
     lv_style_init(&style_bg);
     lv_style_set_bg_color(&style_bg, LV_STATE_DEFAULT, colors.bg);
     lv_style_set_text_color(&style_bg, LV_STATE_DEFAULT, colors.text);
     
     // Card style
     lv_style_init(&style_card);
     lv_style_set_bg_color(&style_card, LV_STATE_DEFAULT, colors.surface);
     lv_style_set_bg_opa(&style_card, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_radius(&style_card, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     lv_style_set_border_color(&style_card, LV_STATE_DEFAULT, colors.border);
     lv_style_set_border_width(&style_card, LV_STATE_DEFAULT, 1);
     lv_style_set_shadow_width(&style_card, LV_STATE_DEFAULT, 10);
     lv_style_set_shadow_color(&style_card, LV_STATE_DEFAULT, lv_color_mix(colors.bg, colors.text, 240));
     lv_style_set_shadow_opa(&style_card, LV_STATE_DEFAULT, LV_OPA_20);
     lv_style_set_pad_top(&style_card, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&style_card, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&style_card, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&style_card, LV_STATE_DEFAULT, 10);
     
     // Button style
     lv_style_init(&style_btn);
     lv_style_set_bg_color(&style_btn, LV_STATE_DEFAULT, colors.primary);
     lv_style_set_bg_opa(&style_btn, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_bg_color(&style_btn, LV_STATE_DISABLED, colors.disabled);
     lv_style_set_radius(&style_btn, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     lv_style_set_text_color(&style_btn, LV_STATE_DEFAULT, COLOR_WHITE);
     lv_style_set_text_color(&style_btn, LV_STATE_DISABLED, lv_color_mix(COLOR_WHITE, colors.disabled, 180));
     lv_style_set_pad_top(&style_btn, LV_STATE_DEFAULT, TDECK_UI_BUTTON_PADDING);
     lv_style_set_pad_bottom(&style_btn, LV_STATE_DEFAULT, TDECK_UI_BUTTON_PADDING);
     lv_style_set_pad_left(&style_btn, LV_STATE_DEFAULT, TDECK_UI_BUTTON_PADDING * 2);
     lv_style_set_pad_right(&style_btn, LV_STATE_DEFAULT, TDECK_UI_BUTTON_PADDING * 2);
     
     // Button pressed style
     lv_style_init(&style_btn_pressed);
     lv_style_set_bg_color(&style_btn_pressed, LV_STATE_PRESSED, COLOR_DARK_BLUE);
     
     // Toggle button style
     lv_style_init(&style_btn_toggle);
     lv_style_set_bg_color(&style_btn_toggle, LV_STATE_CHECKED, colors.secondary);
     
     // Title text style
     lv_style_init(&style_title);
     lv_style_set_text_color(&style_title, LV_STATE_DEFAULT, colors.text);
     lv_style_set_text_font(&style_title, LV_STATE_DEFAULT, &lv_font_montserrat_24);
     
     // Normal text style
     lv_style_init(&style_text);
     lv_style_set_text_color(&style_text, LV_STATE_DEFAULT, colors.text);
     lv_style_set_text_font(&style_text, LV_STATE_DEFAULT, &lv_font_montserrat_14);
     
     // Secondary text style
     lv_style_init(&style_text_secondary);
     lv_style_set_text_color(&style_text_secondary, LV_STATE_DEFAULT, colors.text_secondary);
     lv_style_set_text_font(&style_text_secondary, LV_STATE_DEFAULT, &lv_font_montserrat_14);
     
     // Text field style
     lv_style_init(&style_textfield);
     lv_style_set_bg_color(&style_textfield, LV_STATE_DEFAULT, lv_color_mix(colors.bg, colors.surface, 128));
     lv_style_set_bg_opa(&style_textfield, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_border_color(&style_textfield, LV_STATE_DEFAULT, colors.border);
     lv_style_set_border_width(&style_textfield, LV_STATE_DEFAULT, 1);
     lv_style_set_radius(&style_textfield, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     lv_style_set_pad_top(&style_textfield, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&style_textfield, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&style_textfield, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&style_textfield, LV_STATE_DEFAULT, 10);
     
     // Text field focused style
     lv_style_init(&style_textfield_focused);
     lv_style_set_border_color(&style_textfield_focused, LV_STATE_FOCUSED, colors.primary);
     lv_style_set_border_width(&style_textfield_focused, LV_STATE_FOCUSED, 2);
     
     // Switch style
     lv_style_init(&style_switch);
     lv_style_set_bg_color(&style_switch, LV_STATE_DEFAULT, colors.bg_alt);
     lv_style_set_bg_color(&style_switch, LV_STATE_CHECKED, colors.primary);
     
     // Slider style
     lv_style_init(&style_slider);
     lv_style_set_bg_color(&style_slider, LV_STATE_DEFAULT, colors.bg_alt);
     lv_style_set_bg_color(&style_slider, LV_STATE_EDITED, colors.primary);
     
     // List style
     lv_style_init(&style_list);
     lv_style_set_bg_color(&style_list, LV_STATE_DEFAULT, colors.surface);
     lv_style_set_border_color(&style_list, LV_STATE_DEFAULT, colors.border);
     lv_style_set_border_width(&style_list, LV_STATE_DEFAULT, 1);
     lv_style_set_radius(&style_list, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     lv_style_set_pad_inner(&style_list, LV_STATE_DEFAULT, 5);
     
     // List item style
     lv_style_init(&style_list_item);
     lv_style_set_bg_color(&style_list_item, LV_STATE_DEFAULT, colors.surface);
     lv_style_set_bg_color(&style_list_item, LV_STATE_PRESSED, lv_color_mix(colors.surface, colors.primary, 230));
     lv_style_set_bg_color(&style_list_item, LV_STATE_FOCUSED, lv_color_mix(colors.surface, colors.primary, 230));
     lv_style_set_text_color(&style_list_item, LV_STATE_DEFAULT, colors.text);
     lv_style_set_border_width(&style_list_item, LV_STATE_DEFAULT, 0);
     lv_style_set_border_width(&style_list_item, LV_STATE_FOCUSED, 0);
     lv_style_set_pad_top(&style_list_item, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&style_list_item, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&style_list_item, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&style_list_item, LV_STATE_DEFAULT, 10);
     
     // Dropdown style
     lv_style_init(&style_dropdown);
     lv_style_set_bg_color(&style_dropdown, LV_STATE_DEFAULT, colors.surface);
     lv_style_set_bg_opa(&style_dropdown, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_border_color(&style_dropdown, LV_STATE_DEFAULT, colors.border);
     lv_style_set_border_width(&style_dropdown, LV_STATE_DEFAULT, 1);
     lv_style_set_text_color(&style_dropdown, LV_STATE_DEFAULT, colors.text);
     lv_style_set_pad_top(&style_dropdown, LV_STATE_DEFAULT, 8);
     lv_style_set_pad_bottom(&style_dropdown, LV_STATE_DEFAULT, 8);
     lv_style_set_pad_left(&style_dropdown, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&style_dropdown, LV_STATE_DEFAULT, 10);
     
     // Status bar style
     lv_style_init(&style_status_bar);
     lv_style_set_bg_color(&style_status_bar, LV_STATE_DEFAULT, colors.primary);
     lv_style_set_bg_opa(&style_status_bar, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_text_color(&style_status_bar, LV_STATE_DEFAULT, COLOR_WHITE);
     lv_style_set_pad_top(&style_status_bar, LV_STATE_DEFAULT, 2);
     lv_style_set_pad_bottom(&style_status_bar, LV_STATE_DEFAULT, 2);
     lv_style_set_pad_left(&style_status_bar, LV_STATE_DEFAULT, 5);
     lv_style_set_pad_right(&style_status_bar, LV_STATE_DEFAULT, 5);
 }
 
 // Apply functions for different UI elements
 
 void Theme::applyStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style_bg);
 }
 
 void Theme::applyCardStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style_card);
 }
 
 void Theme::applyButtonStyle(lv_obj_t* obj, bool isPrimary) {
     lv_obj_add_style(obj, LV_BTN_PART_MAIN, &style_btn);
     lv_obj_add_style(obj, LV_BTN_PART_MAIN, &style_btn_pressed);
     
     if (!isPrimary) {
         // For secondary buttons, override primary color with secondary color
         lv_obj_set_style_local_bg_color(obj, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, colors.secondary);
     }
 }
 
 void Theme::applyTitleStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_LABEL_PART_MAIN, &style_title);
 }
 
 void Theme::applyTextFieldStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_TEXTAREA_PART_BG, &style_textfield);
     lv_obj_add_style(obj, LV_TEXTAREA_PART_BG, &style_textfield_focused);
 }
 
 void Theme::applySwitchStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_SWITCH_PART_BG, &style_switch);
 }
 
 void Theme::applySliderStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_SLIDER_PART_BG, &style_slider);
     lv_obj_add_style(obj, LV_SLIDER_PART_INDIC, &style_slider);
     
     // Apply primary color to indicator
     lv_obj_set_style_local_bg_color(obj, LV_SLIDER_PART_INDIC, LV_STATE_DEFAULT, colors.primary);
     
     // Apply darker color to knob
     lv_obj_set_style_local_bg_color(obj, LV_SLIDER_PART_KNOB, LV_STATE_DEFAULT, 
         lv_color_darken(colors.primary, LV_OPA_20));
 }
 
 void Theme::applyListStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_LIST_PART_BG, &style_list);
 }
 
 void Theme::applyListItemStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_BTN_PART_MAIN, &style_list_item);
 }
 
 void Theme::applyDropdownStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_DROPDOWN_PART_MAIN, &style_dropdown);
 }
 
 void Theme::applyStatusBarStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style_status_bar);
 }