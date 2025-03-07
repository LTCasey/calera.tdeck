/**
 * @file battery.h
 * @brief Battery monitoring system for T-Deck UI Firmware
 * 
 * This class handles battery status monitoring, voltage measurements,
 * percentage calculations, and low battery notifications.
 */

 #ifndef TDECK_BATTERY_H
 #define TDECK_BATTERY_H
 
 #include <Arduino.h>
 #include "../config.h"
 
 class BatteryManager {
 public:
     /**
      * @brief Construct a new Battery Manager object
      */
     BatteryManager();
 
     /**
      * @brief Initialize the battery monitoring system
      * 
      * @return true if initialization successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Update battery status by reading voltage and calculating percentage
      */
     void update();
 
     /**
      * @brief Get battery voltage
      * 
      * @return float Current battery voltage in volts
      */
     float getVoltage() const;
 
     /**
      * @brief Get battery percentage
      * 
      * @return int Battery percentage (0-100)
      */
     int getPercentage() const;
 
     /**
      * @brief Check if battery is charging
      * 
      * @return true if battery is charging
      * @return false if battery is not charging
      */
     bool isCharging() const;
 
     /**
      * @brief Check if battery level is low
      * 
      * @return true if battery level is below low threshold
      * @return false if battery level is above low threshold
      */
     bool isLow() const;
 
     /**
      * @brief Check if battery level is critical
      * 
      * @return true if battery level is below critical threshold
      * @return false if battery level is above critical threshold
      */
     bool isCritical() const;
 
     /**
      * @brief Register callback for low battery event
      * 
      * @param callback Function to call when battery is low
      */
     void setLowBatteryCallback(void (*callback)());
 
     /**
      * @brief Register callback for critical battery event
      * 
      * @param callback Function to call when battery is critical
      */
     void setCriticalBatteryCallback(void (*callback)());
 
 private:
     float readVoltage();                   // Read raw voltage from ADC
     int calculatePercentage(float voltage); // Calculate percentage from voltage
 
     // Battery status
     float _voltage;                        // Current battery voltage
     int _percentage;                       // Current battery percentage
     bool _isCharging;                      // Charging status
     bool _wasLow;                          // Previous low battery state
     bool _wasCritical;                     // Previous critical battery state
     unsigned long _lastUpdateTime;         // Last update timestamp
 
     // Smoothing filter values
     static const int VOLTAGE_READINGS = 10; // Number of readings to average
     float _voltageReadings[VOLTAGE_READINGS]; // Array of recent voltage readings
     int _readIndex;                         // Current reading index
     float _voltageTotal;                    // Total for calculating average
 
     // Callback functions
     void (*_lowBatteryCallback)();         // Low battery callback
     void (*_criticalBatteryCallback)();    // Critical battery callback
 };
 
 extern BatteryManager batteryManager;
 
 #endif // TDECK_BATTERY_H