/**
 * @file styles.cpp
 * @brief Implementation of specialized UI styles for T-Deck UI
 */

 #include "styles.h"

 // Global styles instance
 Styles styles;
 
 Styles::Styles() {
     // Nothing to initialize here, initialization happens in init()
 }
 
 bool Styles::init() {
     TDECK_LOG_I("Initializing UI styles");
     
     // Initialize and apply styles based on current theme
     initStyles();
     
     return true;
 }
 
 void Styles::updateForTheme(ThemeType themeType) {
     TDECK_LOG_I("Updating styles for theme change");
     
     // Reinitialize styles with the new theme
     initStyles();
 }
 
 void Styles::initStyles() {
     // Get current theme colors
     const ThemeColors& colors = theme.getColors();
     
     // App icon style
     lv_style_init(&style_app_icon);
     lv_style_set_image_recolor(&style_app_icon, LV_STATE_DEFAULT, colors.text);
     lv_style_set_image_recolor_opa(&style_app_icon, LV_STATE_DEFAULT, LV_OPA_0);
     lv_style_set_image_recolor_opa(&style_app_icon, LV_STATE_PRESSED, LV_OPA_30);
     lv_style_set_transition_time(&style_app_icon, LV_STATE_DEFAULT, 100);
     lv_style_set_transition_prop_1(&style_app_icon, LV_STATE_DEFAULT, LV_STYLE_IMAGE_RECOLOR_OPA);
     
     // Launcher grid style
     lv_style_init(&style_launcher_grid);
     lv_style_set_pad_inner(&style_launcher_grid, LV_STATE_DEFAULT, 20);
     lv_style_set_pad_top(&style_launcher_grid, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&style_launcher_grid, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&style_launcher_grid, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&style_launcher_grid, LV_STATE_DEFAULT, 10);
     
     // Status bar icon style
     lv_style_init(&style_status_bar_icon);
     lv_style_set_image_recolor(&style_status_bar_icon, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
     lv_style_set_image_recolor_opa(&style_status_bar_icon, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_pad_all(&style_status_bar_icon, LV_STATE_DEFAULT, 2);
     
     // Status bar text style
     lv_style_init(&style_status_bar_text);
     lv_style_set_text_color(&style_status_bar_text, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
     lv_style_set_text_font(&style_status_bar_text, LV_STATE_DEFAULT, &lv_font_montserrat_12);
     
     // Table header style
     lv_style_init(&style_table_header);
     lv_style_set_bg_color(&style_table_header, LV_STATE_DEFAULT, colors.primary);
     lv_style_set_bg_opa(&style_table_header, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_text_color(&style_table_header, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
     lv_style_set_border_side(&style_table_header, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
     lv_style_set_border_width(&style_table_header, LV_STATE_DEFAULT, 2);
     lv_style_set_border_color(&style_table_header, LV_STATE_DEFAULT, colors.border);
     lv_style_set_pad_top(&style_table_header, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&style_table_header, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&style_table_header, LV_STATE_DEFAULT, 5);
     lv_style_set_pad_right(&style_table_header, LV_STATE_DEFAULT, 5);
     
     // Settings item style
     lv_style_init(&style_settings_item);
     lv_style_set_bg_color(&style_settings_item, LV_STATE_DEFAULT, colors.surface);
     lv_style_set_bg_opa(&style_settings_item, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_border_width(&style_settings_item, LV_STATE_DEFAULT, 0);
     lv_style_set_border_side(&style_settings_item, LV_STATE_DEFAULT, LV_BORDER_SIDE_BOTTOM);
     lv_style_set_border_color(&style_settings_item, LV_STATE_DEFAULT, colors.border);
     lv_style_set_border_width(&style_settings_item, LV_STATE_DEFAULT, 1);
     lv_style_set_pad_top(&style_settings_item, LV_STATE_DEFAULT, 15);
     lv_style_set_pad_bottom(&style_settings_item, LV_STATE_DEFAULT, 15);
     lv_style_set_pad_left(&style_settings_item, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_right(&style_settings_item, LV_STATE_DEFAULT, 10);
     
     // Battery normal style
     lv_style_init(&style_battery_normal);
     lv_style_set_bg_color(&style_battery_normal, LV_STATE_DEFAULT, colors.success);
     lv_style_set_bg_opa(&style_battery_normal, LV_STATE_DEFAULT, LV_OPA_COVER);
     
     // Battery low style
     lv_style_init(&style_battery_low);
     lv_style_set_bg_color(&style_battery_low, LV_STATE_DEFAULT, colors.warning);
     lv_style_set_bg_opa(&style_battery_low, LV_STATE_DEFAULT, LV_OPA_COVER);
     
     // Battery critical style
     lv_style_init(&style_battery_critical);
     lv_style_set_bg_color(&style_battery_critical, LV_STATE_DEFAULT, colors.error);
     lv_style_set_bg_opa(&style_battery_critical, LV_STATE_DEFAULT, LV_OPA_COVER);
     
     // Tab style
     lv_style_init(&style_tab);
     lv_style_set_bg_color(&style_tab, LV_STATE_DEFAULT, colors.bg_alt);
     lv_style_set_bg_opa(&style_tab, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_text_color(&style_tab, LV_STATE_DEFAULT, colors.text_secondary);
     lv_style_set_pad_top(&style_tab, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_bottom(&style_tab, LV_STATE_DEFAULT, 10);
     lv_style_set_pad_left(&style_tab, LV_STATE_DEFAULT, 15);
     lv_style_set_pad_right(&style_tab, LV_STATE_DEFAULT, 15);
     
     // Active tab style
     lv_style_init(&style_tab_active);
     lv_style_set_bg_color(&style_tab_active, LV_STATE_DEFAULT, colors.primary);
     lv_style_set_text_color(&style_tab_active, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
     lv_style_set_bg_opa(&style_tab_active, LV_STATE_DEFAULT, LV_OPA_COVER);
     
     // File icon style
     lv_style_init(&style_file_icon);
     lv_style_set_image_recolor(&style_file_icon, LV_STATE_DEFAULT, colors.secondary);
     lv_style_set_image_recolor_opa(&style_file_icon, LV_STATE_DEFAULT, LV_OPA_50);
     
     // Folder icon style
     lv_style_init(&style_folder_icon);
     lv_style_set_image_recolor(&style_folder_icon, LV_STATE_DEFAULT, colors.primary);
     lv_style_set_image_recolor_opa(&style_folder_icon, LV_STATE_DEFAULT, LV_OPA_50);
     
     // Dialog style
     lv_style_init(&style_dialog);
     lv_style_set_bg_color(&style_dialog, LV_STATE_DEFAULT, colors.surface);
     lv_style_set_bg_opa(&style_dialog, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_border_color(&style_dialog, LV_STATE_DEFAULT, colors.border);
     lv_style_set_border_width(&style_dialog, LV_STATE_DEFAULT, 1);
     lv_style_set_radius(&style_dialog, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     lv_style_set_shadow_width(&style_dialog, LV_STATE_DEFAULT, 20);
     lv_style_set_shadow_color(&style_dialog, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
     lv_style_set_shadow_opa(&style_dialog, LV_STATE_DEFAULT, LV_OPA_30);
     lv_style_set_pad_all(&style_dialog, LV_STATE_DEFAULT, 20);
     
     // Keyboard style
     lv_style_init(&style_keyboard);
     lv_style_set_bg_color(&style_keyboard, LV_STATE_DEFAULT, colors.bg_alt);
     lv_style_set_bg_opa(&style_keyboard, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_pad_top(&style_keyboard, LV_STATE_DEFAULT, 5);
     lv_style_set_pad_bottom(&style_keyboard, LV_STATE_DEFAULT, 5);
     lv_style_set_pad_left(&style_keyboard, LV_STATE_DEFAULT, 5);
     lv_style_set_pad_right(&style_keyboard, LV_STATE_DEFAULT, 5);
     lv_style_set_pad_inner(&style_keyboard, LV_STATE_DEFAULT, 4);
     
     // Toast notification style
     lv_style_init(&style_toast);
     lv_style_set_bg_color(&style_toast, LV_STATE_DEFAULT, lv_color_make(0x30, 0x30, 0x30));
     lv_style_set_bg_opa(&style_toast, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_text_color(&style_toast, LV_STATE_DEFAULT, lv_color_make(0xFF, 0xFF, 0xFF));
     lv_style_set_radius(&style_toast, LV_STATE_DEFAULT, TDECK_UI_DEFAULT_CORNER_RADIUS);
     lv_style_set_shadow_width(&style_toast, LV_STATE_DEFAULT, 15);
     lv_style_set_shadow_color(&style_toast, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
     lv_style_set_shadow_opa(&style_toast, LV_STATE_DEFAULT, LV_OPA_30);
     lv_style_set_pad_all(&style_toast, LV_STATE_DEFAULT, 15);
     
     // Chart style
     lv_style_init(&style_chart);
     lv_style_set_line_width(&style_chart, LV_STATE_DEFAULT, 2);
     lv_style_set_line_color(&style_chart, LV_STATE_DEFAULT, colors.primary);
     lv_style_set_scale_grad_color(&style_chart, LV_STATE_DEFAULT, colors.primary);
     lv_style_set_scale_end_color(&style_chart, LV_STATE_DEFAULT, colors.text_secondary);
     lv_style_set_pad_all(&style_chart, LV_STATE_DEFAULT, 10);
     
     // Progress bar style
     lv_style_init(&style_progress);
     lv_style_set_bg_color(&style_progress, LV_STATE_DEFAULT, colors.bg_alt);
     lv_style_set_bg_opa(&style_progress, LV_STATE_DEFAULT, LV_OPA_COVER);
     lv_style_set_border_width(&style_progress, LV_STATE_DEFAULT, 0);
     lv_style_set_radius(&style_progress, LV_STATE_DEFAULT, 10); // More rounded for progress bars
     lv_style_set_pad_all(&style_progress, LV_STATE_DEFAULT, 0);
 }
 
 // Style application methods
 
 void Styles::applyAppIconStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_IMG_PART_MAIN, &style_app_icon);
 }
 
 void Styles::applyLauncherGridStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_CONT_PART_MAIN, &style_launcher_grid);
 }
 
 void Styles::applyStatusBarIconStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_IMG_PART_MAIN, &style_status_bar_icon);
 }
 
 void Styles::applyStatusBarTextStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_LABEL_PART_MAIN, &style_status_bar_text);
 }
 
 void Styles::applyTableHeaderStyle(lv_obj_t* obj) {
     // Apply this to the table header part
     lv_obj_add_style(obj, LV_TABLE_PART_CELL1, &style_table_header);
 }
 
 void Styles::applySettingsItemStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_CONT_PART_MAIN, &style_settings_item);
 }
 
 void Styles::applyBatteryStyle(lv_obj_t* obj, uint8_t level) {
     if (level <= TDECK_BATTERY_CRITICAL_THRESHOLD) {
         lv_obj_add_style(obj, LV_BAR_PART_INDIC, &style_battery_critical);
     } else if (level <= TDECK_BATTERY_LOW_THRESHOLD) {
         lv_obj_add_style(obj, LV_BAR_PART_INDIC, &style_battery_low);
     } else {
         lv_obj_add_style(obj, LV_BAR_PART_INDIC, &style_battery_normal);
     }
 }
 
 void Styles::applyTabStyle(lv_obj_t* obj, bool isActive) {
     if (isActive) {
         lv_obj_add_style(obj, LV_BTN_PART_MAIN, &style_tab_active);
     } else {
         lv_obj_add_style(obj, LV_BTN_PART_MAIN, &style_tab);
     }
 }
 
 void Styles::applyFileIconStyle(lv_obj_t* obj, bool isFolder) {
     if (isFolder) {
         lv_obj_add_style(obj, LV_IMG_PART_MAIN, &style_folder_icon);
     } else {
         lv_obj_add_style(obj, LV_IMG_PART_MAIN, &style_file_icon);
     }
 }
 
 void Styles::applyDialogStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_CONT_PART_MAIN, &style_dialog);
 }
 
 void Styles::applyKeyboardStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_KEYBOARD_PART_BG, &style_keyboard);
 }
 
 void Styles::applyToastStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_CONT_PART_MAIN, &style_toast);
 }
 
 void Styles::applyChartStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_CHART_PART_BG, &style_chart);
     
     // Get current theme colors for custom series colors
     const ThemeColors& colors = theme.getColors();
     
     // Set series colors - modify these if using multiple series
     lv_obj_set_style_local_line_color(obj, LV_CHART_PART_SERIES, 0, colors.primary);
     lv_obj_set_style_local_line_color(obj, LV_CHART_PART_SERIES, 1, colors.secondary);
     lv_obj_set_style_local_line_color(obj, LV_CHART_PART_SERIES, 2, colors.accent);
     lv_obj_set_style_local_line_color(obj, LV_CHART_PART_SERIES, 3, colors.success);
 }
 
 void Styles::applyProgressStyle(lv_obj_t* obj) {
     lv_obj_add_style(obj, LV_BAR_PART_BG, &style_progress);
     
     // Get current theme colors
     const ThemeColors& colors = theme.getColors();
     
     // Set indicator style for progress bar
     lv_obj_set_style_local_bg_color(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, colors.primary);
     lv_obj_set_style_local_radius(obj, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, 10);
 }