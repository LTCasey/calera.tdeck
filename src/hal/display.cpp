/**
 * @file display.cpp
 * @brief Display driver implementation for T-Deck UI Firmware
 */

 #include "display.h"

 // Global display instance
 TDeckDisplay Display;
 
 TDeckDisplay::TDeckDisplay() : 
     _backlight_level(TDECK_DISPLAY_BL_MAX),
     _is_on(false),
     _buf(nullptr),
     _disp(nullptr)
 {
 }
 
 bool TDeckDisplay::init() {
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
     
     // Initialize LVGL integration
     if (!_init_lvgl()) {
         TDECK_LOG_E("Failed to initialize LVGL display");
         return false;
     }
     
     // Turn on display with default brightness
     turnOn();
     setBacklight(_backlight_level);
     
     TDECK_LOG_I("Display initialized successfully");
     return true;
 }
 
 bool TDeckDisplay::_init_backlight() {
     // Configure backlight control pin
     pinMode(TDECK_DISPLAY_BL_PIN, OUTPUT);
     
     // Configure PWM for backlight control
     ledcSetup(TDECK_DISPLAY_BL_CHANNEL, TDECK_DISPLAY_BL_FREQ, TDECK_DISPLAY_BL_RESOLUTION);
     ledcAttachPin(TDECK_DISPLAY_BL_PIN, TDECK_DISPLAY_BL_CHANNEL);
     
     return true;
 }
 
 bool TDeckDisplay::_init_lvgl() {
     // Initialize LVGL library
     lv_init();
     
     // Allocate display buffer for LVGL
     // Using double buffering with 1/10 screen size buffers
     uint32_t buf_size = TDECK_DISPLAY_WIDTH * (TDECK_DISPLAY_HEIGHT / 10);
     _buf = (lv_color_t*)heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_DMA);
     
     if (!_buf) {
         TDECK_LOG_E("Failed to allocate LVGL display buffer");
         return false;
     }
     
     // Initialize LVGL draw buffer
     lv_disp_draw_buf_init(&_draw_buf, _buf, NULL, buf_size);
     
     // Initialize LVGL display driver
     lv_disp_drv_init(&_disp_drv);
     _disp_drv.hor_res = TDECK_DISPLAY_WIDTH;
     _disp_drv.ver_res = TDECK_DISPLAY_HEIGHT;
     _disp_drv.flush_cb = _lvgl_flush_cb;
     _disp_drv.draw_buf = &_draw_buf;
     _disp_drv.user_data = this;
     
     // Register the display driver
     _disp = lv_disp_drv_register(&_disp_drv);
     
     if (!_disp) {
         TDECK_LOG_E("Failed to register LVGL display driver");
         free(_buf);
         _buf = nullptr;
         return false;
     }
     
     return true;
 }
 
 void TDeckDisplay::_lvgl_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
     TDeckDisplay* display = (TDeckDisplay*)disp->user_data;
     TFT_eSPI& tft = display->getTft();
     
     uint32_t w = (area->x2 - area->x1 + 1);
     uint32_t h = (area->y2 - area->y1 + 1);
     
     // Set the active window
     tft.setAddrWindow(area->x1, area->y1, w, h);
     
     // Push colors to display
     tft.pushColors((uint16_t*)color_p, w * h);
     
     // Indicate to LVGL that the flush is done
     lv_disp_flush_ready(disp);
 }
 
 void TDeckDisplay::setBacklight(uint8_t level) {
     _backlight_level = level;
     
     if (_is_on) {
         ledcWrite(TDECK_DISPLAY_BL_CHANNEL, _backlight_level);
     }
 }
 
 uint8_t TDeckDisplay::getBacklight() const {
     return _backlight_level;
 }
 
 void TDeckDisplay::turnOff() {
     if (_is_on) {
         ledcWrite(TDECK_DISPLAY_BL_CHANNEL, 0);
         _is_on = false;
     }
 }
 
 void TDeckDisplay::turnOn() {
     if (!_is_on) {
         ledcWrite(TDECK_DISPLAY_BL_CHANNEL, _backlight_level);
         _is_on = true;
     }
 }
 
 bool TDeckDisplay::isOn() const {
     return _is_on;
 }
 
 TFT_eSPI& TDeckDisplay::getTft() {
     return _tft;
 }
 
 lv_disp_t* TDeckDisplay::getLvglDisplay() {
     return _disp;
 }
 
 void TDeckDisplay::update() {
     // Call LVGL tick and task handler
     lv_task_handler();
 }
 
 void TDeckDisplay::sleep() {
     // Save current state
     bool was_on = _is_on;
     
     // Turn off display
     turnOff();
     
     // Send display to sleep mode
     _tft.writecommand(0x10); // Enter sleep mode
     
     // Remember the display state
     _is_on = was_on;
 }
 
 void TDeckDisplay::wakeup() {
     // Wake up the display
     _tft.writecommand(0x11); // Exit sleep mode
     delay(120); // Mandatory delay after sleep out command
     
     // Restore previous state
     if (_is_on) {
         turnOn();
     }
 }