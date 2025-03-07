/**
 * @file display.cpp
 * @brief Display driver implementation for T-Deck UI Firmware
 */

 #include "display.h"

 // Global display instance
 Display display;
 
 Display::Display() : 
     _backlight_level(TDECK_DISPLAY_BL_MAX),
     _is_on(false)
 {
 }
 
 bool Display::init() {
     TDECK_LOG_I("Initializing display");
     
     // Initialize TFT
     _tft.init();
     _tft.setRotation(1); // Landscape mode
     _tft.fillScreen(TFT_BLACK);
     
     // Initialize backlight control
     if (!_init_backlight()) {
         TDECK_LOG_E("Failed to initialize backlight");
         return false;
     }
     
     // Turn on display with default brightness
     turnOn();
     setBacklight(_backlight_level);
     
     TDECK_LOG_I("Display initialized successfully");
     return true;
 }
 
 bool Display::_init_backlight() {
     // Configure backlight control pin
     pinMode(TDECK_DISPLAY_BL_PIN, OUTPUT);
     
     // Configure PWM for backlight control
     ledcSetup(TDECK_DISPLAY_BL_CHANNEL, TDECK_DISPLAY_BL_FREQ, TDECK_DISPLAY_BL_RESOLUTION);
     ledcAttachPin(TDECK_DISPLAY_BL_PIN, TDECK_DISPLAY_BL_CHANNEL);
     
     return true;
 }
 
 void Display::setBacklight(uint8_t level) {
     _backlight_level = level;
     
     if (_is_on) {
         ledcWrite(TDECK_DISPLAY_BL_CHANNEL, _backlight_level);
     }
 }
 
 uint8_t Display::getBacklight() const {
     return _backlight_level;
 }
 
 void Display::turnOff() {
     if (_is_on) {
         ledcWrite(TDECK_DISPLAY_BL_CHANNEL, 0);
         _is_on = false;
     }
 }
 
 void Display::turnOn() {
     if (!_is_on) {
         ledcWrite(TDECK_DISPLAY_BL_CHANNEL, _backlight_level);
         _is_on = true;
     }
 }
 
 bool Display::isOn() const {
     return _is_on;
 }
 
 TFT_eSPI& Display::getTft() {
     return _tft;
 }
 
 void Display::update() {
     // Call LVGL tick and task handler
     lv_task_handler();
 }
 
 void Display::sleep() {
     // Save current state
     bool was_on = _is_on;
     
     // Turn off display
     turnOff();
     
     // Send display to sleep mode
     _tft.writecommand(0x10); // Enter sleep mode
     
     // Remember the display state
     _is_on = was_on;
 }
 
 void Display::wakeup() {
     // Wake up the display
     _tft.writecommand(0x11); // Exit sleep mode
     delay(120); // Mandatory delay after sleep out command
     
     // Restore previous state
     if (_is_on) {
         turnOn();
     }
 }
 
 // Static callback function for LVGL display flushing
 void Display::flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
     uint32_t w = (area->x2 - area->x1 + 1);
     uint32_t h = (area->y2 - area->y1 + 1);
     
     // Set the active window
     display.getTft().setAddrWindow(area->x1, area->y1, w, h);
     
     // Push colors to display
     display.getTft().pushColors((uint16_t*)color_p, w * h);
     
     // Indicate to LVGL that the flush is done
     lv_disp_flush_ready(disp_drv);
 }