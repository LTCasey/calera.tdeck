/**
 * @file touchscreen.cpp
 * @brief Touchscreen driver implementation for LilyGO T-Deck
 * 
 * This file implements the touchscreen driver for the T-Deck hardware.
 */

 #include "touchscreen.h"

 // Create global touchscreen instance
 TouchScreen touchScreen;
 
 TouchScreen::TouchScreen() :
     _available(false),
     _touched(false),
     _touchX(0),
     _touchY(0),
     _touchPressure(0),
     _rotation(0),
     _minX(0),
     _maxX(TDECK_DISPLAY_WIDTH),
     _minY(0),
     _maxY(TDECK_DISPLAY_HEIGHT)
 {
     _wire = new TwoWire(0);
 }
 
 bool TouchScreen::begin() {
     // Initialize I2C bus for touchscreen
     _wire->begin(TDECK_TOUCH_SDA, TDECK_TOUCH_SCL);
     
     // Reset touchscreen controller
     if (!reset()) {
         TDECK_LOG_E("Touchscreen reset failed");
         return false;
     }
     
     // Initialize FT6206 controller
     if (!_ft6206.begin(0x38, _wire)) {
         TDECK_LOG_E("Failed to initialize touchscreen");
         _available = false;
         return false;
     }
     
     TDECK_LOG_I("Touchscreen initialized successfully");
     _available = true;
     return true;
 }
 
 bool TouchScreen::isAvailable() const {
     return _available;
 }
 
 bool TouchScreen::update() {
     if (!_available) {
         return false;
     }
     
     // Check if the screen is being touched
     bool previousTouched = _touched;
     _touched = _ft6206.touched();
     
     // If screen is being touched, update coordinates
     if (_touched) {
         TS_Point point = _ft6206.getPoint();
         
         // Store raw coordinates
         _touchX = point.x;
         _touchY = point.y;
         
         // Apply calibration and rotation
         mapCoordinates(_touchX, _touchY);
         
         // Set a default pressure value (FT6206 doesn't provide pressure)
         _touchPressure = 255;
         
         // Return true if this is a new touch event
         return !previousTouched;
     }
     
     // Return true if touch has ended
     return previousTouched && !_touched;
 }
 
 uint16_t TouchScreen::getX() const {
     return _touchX;
 }
 
 uint16_t TouchScreen::getY() const {
     return _touchY;
 }
 
 uint8_t TouchScreen::getPressure() const {
     return _touchPressure;
 }
 
 bool TouchScreen::isTouched() const {
     return _touched;
 }
 
 bool TouchScreen::reset() {
     // Check if reset pin is defined
     #ifdef TDECK_TOUCH_RST
         // Configure reset pin as output
         pinMode(TDECK_TOUCH_RST, OUTPUT);
         
         // Perform hardware reset sequence
         digitalWrite(TDECK_TOUCH_RST, LOW);
         delay(10);
         digitalWrite(TDECK_TOUCH_RST, HIGH);
         delay(50);
         
         return true;
     #else
         // No reset pin available, try soft reset via I2C
         // This is controller-specific and may not work with all touchscreens
         return false;
     #endif
 }
 
 void TouchScreen::setCalibration(uint16_t minX, uint16_t maxX, uint16_t minY, uint16_t maxY) {
     _minX = minX;
     _maxX = maxX;
     _minY = minY;
     _maxY = maxY;
 }
 
 void TouchScreen::setRotation(uint8_t rotation) {
     // Ensure rotation is in valid range (0-3)
     _rotation = rotation % 4;
 }
 
 void TouchScreen::mapCoordinates(uint16_t &x, uint16_t &y) {
     // T-Deck touchscreen typically needs coordinates to be mapped
     // depending on the display orientation
     
     // The FT6206 touchscreen controller used in T-Deck has a native
     // resolution that may differ from the display resolution, so we
     // need to map the coordinates
     
     // Apply calibration (map raw touch coordinates to display coordinates)
     // Note: The exact mapping depends on the specific hardware and may
     // need adjustment based on testing
     
     // Map X coordinate to display range
     x = map(x, 0, 320, _minX, _maxX);
     
     // Map Y coordinate to display range
     y = map(y, 0, 240, _minY, _maxY);
     
     // Apply rotation based on current display orientation
     uint16_t temp;
     switch (_rotation) {
         case 0: // 0 degrees (default)
             // No changes needed
             break;
             
         case 1: // 90 degrees
             temp = x;
             x = TDECK_DISPLAY_WIDTH - y;
             y = temp;
             break;
             
         case 2: // 180 degrees
             x = TDECK_DISPLAY_WIDTH - x;
             y = TDECK_DISPLAY_HEIGHT - y;
             break;
             
         case 3: // 270 degrees
             temp = x;
             x = y;
             y = TDECK_DISPLAY_HEIGHT - temp;
             break;
     }
     
     // Ensure coordinates are within display bounds
     x = constrain(x, 0, TDECK_DISPLAY_WIDTH - 1);
     y = constrain(y, 0, TDECK_DISPLAY_HEIGHT - 1);
 }
 
 // Static callback function for LVGL touch input reading
 void TouchScreen::read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
     static lv_coord_t last_x = 0;
     static lv_coord_t last_y = 0;
     
     // Check if touchscreen is being touched
     if (touchScreen.isTouched()) {
         // Get current coordinates
         last_x = touchScreen.getX();
         last_y = touchScreen.getY();
         
         // Set data for LVGL
         data->state = LV_INDEV_STATE_PR;
         data->point.x = last_x;
         data->point.y = last_y;
     } else {
         // Not touched, set released state but keep last coordinates
         data->state = LV_INDEV_STATE_REL;
         data->point.x = last_x;
         data->point.y = last_y;
     }
 }