/**
 * @file touchscreen.h
 * @brief Touchscreen driver for LilyGO T-Deck
 * 
 * This file contains the interface for the touchscreen controller
 * used in the T-Deck hardware.
 */

 #ifndef TDECK_TOUCHSCREEN_H
 #define TDECK_TOUCHSCREEN_H
 
 #include <Arduino.h>
 #include <Wire.h>
 #include <Adafruit_FT6206.h>
 #include "../config.h"
 
 /**
  * @class TouchScreen
  * @brief Hardware abstraction layer for the T-Deck touchscreen
  * 
  * This class handles the initialization and interaction with the
  * capacitive touchscreen controller on the T-Deck hardware.
  */
 class TouchScreen {
 public:
     /**
      * @brief Construct a new TouchScreen object
      */
     TouchScreen();
 
     /**
      * @brief Initialize the touchscreen
      * 
      * @return true if initialization was successful
      * @return false if initialization failed
      */
     bool begin();
 
     /**
      * @brief Check if touchscreen is available
      * 
      * @return true if touchscreen is available
      * @return false if touchscreen is not available
      */
     bool isAvailable() const;
 
     /**
      * @brief Update touch data
      * 
      * @return true if a touch event was detected
      * @return false if no touch event was detected
      */
     bool update();
 
     /**
      * @brief Get the current touch X coordinate
      * 
      * @return uint16_t X coordinate
      */
     uint16_t getX() const;
 
     /**
      * @brief Get the current touch Y coordinate
      * 
      * @return uint16_t Y coordinate
      */
     uint16_t getY() const;
 
     /**
      * @brief Get touch pressure
      * 
      * @return uint8_t touch pressure
      */
     uint8_t getPressure() const;
 
     /**
      * @brief Check if screen is being touched
      * 
      * @return true if touched
      * @return false if not touched
      */
     bool isTouched() const;
 
     /**
      * @brief Reset the touchscreen controller
      * 
      * @return true if reset was successful
      * @return false if reset failed
      */
     bool reset();
 
     /**
      * @brief Set touchscreen calibration
      * 
      * @param minX Minimum X value
      * @param maxX Maximum X value
      * @param minY Minimum Y value
      * @param maxY Maximum Y value
      */
     void setCalibration(uint16_t minX, uint16_t maxX, uint16_t minY, uint16_t maxY);
 
     /**
      * @brief Set touchscreen rotation
      * 
      * @param rotation Rotation (0-3)
      */
     void setRotation(uint8_t rotation);
 
     /**
      * @brief Static callback function for LVGL touchscreen input
      * 
      * @param indev_drv Pointer to the input device driver
      * @param data Pointer to the input data
      * @return true if more data is available (unused)
      * @return false if no more data is available (unused)
      */
     static void read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);
 
 private:
     Adafruit_FT6206 _ft6206;         // FT6206 driver instance
     TwoWire* _wire;                  // I2C interface
     bool _available;                 // Touchscreen availability flag
     bool _touched;                   // Touch state
     uint16_t _touchX;                // Current touch X coordinate
     uint16_t _touchY;                // Current touch Y coordinate
     uint8_t _touchPressure;          // Current touch pressure
     uint8_t _rotation;               // Display rotation (0-3)
     
     // Calibration values
     uint16_t _minX;
     uint16_t _maxX;
     uint16_t _minY;
     uint16_t _maxY;
 
     /**
      * @brief Map raw touchscreen coordinates to display coordinates
      * 
      * @param x Raw X coordinate
      * @param y Raw Y coordinate
      */
     void mapCoordinates(uint16_t &x, uint16_t &y);
 };
 
 // Global touchscreen instance
 extern TouchScreen touchScreen;
 
 #endif // TDECK_TOUCHSCREEN_H