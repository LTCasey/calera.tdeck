/**
 * @file power.h
 * @brief Power management for LilyGO T-Deck
 * 
 * This file contains the interface for power management, 
 * battery monitoring, and sleep modes for the T-Deck hardware.
 */

 #ifndef TDECK_POWER_H
 #define TDECK_POWER_H
 
 #include <Arduino.h>
 #include <esp_sleep.h>
 #include <esp_pm.h>
 #include "../../config.h"
 
 // Power states
 enum class PowerState {
     NORMAL,         // Normal operation
     LOW_POWER,      // Low power mode with reduced performance
     SLEEP,          // Sleep mode (light sleep)
     DEEP_SLEEP      // Deep sleep mode
 };
 
 // Battery states
 enum class BatteryState {
     UNKNOWN,        // Battery state unknown
     CHARGING,       // Battery is charging
     DISCHARGING,    // Battery is discharging
     FULL,           // Battery is fully charged
     LOW,            // Battery is low
     CRITICAL        // Battery is critically low
 };
 
 /**
  * @class Power
  * @brief Hardware abstraction layer for T-Deck power management
  * 
  * This class handles power management, battery monitoring, 
  * and sleep modes for the T-Deck hardware.
  */
 class Power {
 public:
     /**
      * @brief Construct a new Power object
      */
     Power();
 
     /**
      * @brief Initialize power management
      * 
      * @return true if initialization was successful
      * @return false if initialization failed
      */
     bool begin();
 
     /**
      * @brief Update power status
      * 
      * @return true if status has changed
      * @return false if status is unchanged
      */
     bool update();
 
     /**
      * @brief Get battery voltage
      * 
      * @return float Battery voltage in volts
      */
     float getBatteryVoltage();
 
     /**
      * @brief Get battery percentage
      * 
      * @return uint8_t Battery percentage (0-100)
      */
     uint8_t getBatteryPercentage();
 
     /**
      * @brief Get battery state
      * 
      * @return BatteryState Current battery state
      */
     BatteryState getBatteryState();
 
     /**
      * @brief Check if device is charging
      * 
      * @return true if charging
      * @return false if not charging
      */
     bool isCharging();
 
     /**
      * @brief Check if USB is connected
      * 
      * @return true if USB is connected
      * @return false if USB is not connected
      */
     bool isUsbConnected();
 
     /**
      * @brief Set power state
      * 
      * @param state Desired power state
      * @return true if state change was successful
      * @return false if state change failed
      */
     bool setPowerState(PowerState state);
 
     /**
      * @brief Get current power state
      * 
      * @return PowerState Current power state
      */
     PowerState getPowerState();
 
     /**
      * @brief Enter sleep mode
      * 
      * @param sleepTime Sleep time in milliseconds (0 for indefinite)
      * @return true if entering sleep was successful
      * @return false if entering sleep failed
      */
     bool sleep(uint64_t sleepTime = 0);
 
     /**
      * @brief Enter deep sleep mode
      * 
      * @param sleepTime Sleep time in milliseconds (0 for indefinite)
      * @return true if entering deep sleep was successful
      * @return false if entering deep sleep failed
      */
     bool deepSleep(uint64_t sleepTime = 0);
 
     /**
      * @brief Reset idle timer
      */
     void resetIdleTimer();
 
     /**
      * @brief Get idle time
      * 
      * @return uint32_t Idle time in milliseconds
      */
     uint32_t getIdleTime();
 
     /**
      * @brief Set backlight brightness
      * 
      * @param brightness Brightness level (0-255)
      */
     void setBacklight(uint8_t brightness);
 
     /**
      * @brief Get backlight brightness
      * 
      * @return uint8_t Current brightness level (0-255)
      */
     uint8_t getBacklight();
 
     /**
      * @brief Define a wakeup pin for sleep/deep sleep
      * 
      * @param pin GPIO pin number
      * @param level Level to trigger wakeup (HIGH or LOW)
      * @param pullup Enable internal pullup
      * @param pulldown Enable internal pulldown
      */
     void setWakeupPin(gpio_num_t pin, int level, bool pullup = false, bool pulldown = false);
 
     /**
      * @brief Check if system was woken up from deep sleep
      * 
      * @return true if woken from deep sleep
      * @return false if normal boot
      */
     bool isWakeFromDeepSleep();
 
     /**
      * @brief Get wakeup cause
      * 
      * @return esp_sleep_wakeup_cause_t Wakeup cause
      */
     esp_sleep_wakeup_cause_t getWakeupCause();
 
 private:
     float _batteryVoltage;          // Current battery voltage
     uint8_t _batteryPercentage;     // Current battery percentage
     BatteryState _batteryState;     // Current battery state
     PowerState _powerState;         // Current power state
     bool _isCharging;               // Charging status
     bool _isUsbConnected;           // USB connection status
     uint32_t _lastActivity;         // Last activity timestamp
     uint8_t _backlightLevel;        // Current backlight level
     
     /**
      * @brief Read battery voltage from ADC
      * 
      * @return float Raw battery voltage
      */
     float readBatteryVoltage();
     
     /**
      * @brief Convert battery voltage to percentage
      * 
      * @param voltage Battery voltage
      * @return uint8_t Battery percentage
      */
     uint8_t voltageToPercentage(float voltage);
     
     /**
      * @brief Check charging status
      * 
      * @return true if charging
      * @return false if not charging
      */
     bool checkCharging();
     
     /**
      * @brief Check USB connection status
      * 
      * @return true if USB is connected
      * @return false if USB is not connected
      */
     bool checkUsbConnection();
     
     /**
      * @brief Update battery state based on voltage and charging status
      */
     void updateBatteryState();
     
     /**
      * @brief Initialize ESP32 power management
      */
     void initPowerManagement();
 };
 
 // Global power instance
 extern Power power;
 
 #endif // TDECK_POWER_H