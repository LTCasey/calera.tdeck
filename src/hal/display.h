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
 #include "../config.h"
 
 /**
  * @brief Display manager class for T-Deck
  * 
  * Handles initialization and management of the T-Deck's 2.8" IPS display,
  * including backlight control, LVGL integration, and power management.
  */
 class Display {
 public:
     /**
      * @brief Construct a new Display object
      */
     Display();
 
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
 
     /**
      * @brief Static callback for LVGL display flush
      * Required for LVGL to work with the display
      * 
      * @param disp_drv Display driver
      * @param area Area to update
      * @param color_p Color buffer
      */
     static void flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
 
 private:
     // TFT driver instance
     TFT_eSPI _tft;
     
     // Current backlight level
     uint8_t _backlight_level;
     
     // Display state
     bool _is_on;
     
     // Initialize backlight control
     bool _init_backlight();
 };
 
 // Global display instance
 extern Display display;
 
 #endif // TDECK_DISPLAY_H