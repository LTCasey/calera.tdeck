/**
 * @file display.h
 * @brief Display driver interface for T-Deck UI Firmware
 * 
 * This file contains the declarations for the display driver functionality
 * including initialization, control, and management of the 2.8" IPS display.
 */

 #ifndef TDECK_DISPLAY_H
 #define TDECK_DISPLAY_H
 
 #include <Arduino.h>
 #include <TFT_eSPI.h>
 #include <lvgl.h>
 #include "../../config.h"
 
 /**
  * @brief Display manager class for T-Deck
  * 
  * Handles initialization and management of the T-Deck's 2.8" IPS display,
  * including backlight control, LVGL integration, and power management.
  */
 class TDeckDisplay {
 public:
     /**
      * @brief Construct a new TDeckDisplay object
      */
     TDeckDisplay();
 
     /**
      * @brief Initialize the display and related components
      * 
      * @return true if initialization was successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Set the display backlight level
      * 
      * @param level Backlight level (0-255)
      */
     void setBacklight(uint8_t level);
 
     /**
      * @brief Get the current backlight level
      * 
      * @return uint8_t Current backlight level (0-255)
      */
     uint8_t getBacklight() const;
 
     /**
      * @brief Turn off the display completely
      */
     void turnOff();
 
     /**
      * @brief Turn on the display
      */
     void turnOn();
 
     /**
      * @brief Check if the display is currently on
      * 
      * @return true if the display is on
      * @return false if the display is off
      */
     bool isOn() const;
 
     /**
      * @brief Get the underlying TFT_eSPI object
      * 
      * @return TFT_eSPI& Reference to the TFT_eSPI object
      */
     TFT_eSPI& getTft();
 
     /**
      * @brief Get the LVGL display object
      * 
      * @return lv_disp_t* Pointer to the LVGL display object
      */
     lv_disp_t* getLvglDisplay();
 
     /**
      * @brief Update the display (should be called regularly from the main loop)
      */
     void update();
 
     /**
      * @brief Put the display in low power mode
      */
     void sleep();
 
     /**
      * @brief Wake the display from low power mode
      */
     void wakeup();
 
 private:
     // TFT driver instance
     TFT_eSPI _tft;
     
     // LVGL display buffer
     lv_disp_draw_buf_t _draw_buf;
     
     // LVGL display device
     lv_disp_drv_t _disp_drv;
     
     // Display buffer for LVGL
     lv_color_t* _buf;
     
     // Display driver data
     lv_disp_t* _disp;
     
     // Current backlight level
     uint8_t _backlight_level;
     
     // Display state
     bool _is_on;
     
     // LVGL display flush callback
     static void _lvgl_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
     
     // Initialize LVGL integration
     bool _init_lvgl();
     
     // Initialize backlight control
     bool _init_backlight();
 };
 
 // Global display instance
 extern TDeckDisplay Display;
 
 #endif // TDECK_DISPLAY_H