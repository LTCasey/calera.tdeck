/**
 * @file power.cpp
 * @brief Power management implementation for LilyGO T-Deck
 * 
 * This file implements power management functionality, battery monitoring,
 * and sleep modes for the T-Deck hardware.
 */

 #include "power.h"

 // Create global power instance
 Power power;
 
 Power::Power() :
     _batteryVoltage(0.0f),
     _batteryPercentage(0),
     _batteryState(BatteryState::UNKNOWN),
     _powerState(PowerState::NORMAL),
     _isCharging(false),
     _isUsbConnected(false),
     _lastActivity(0),
     _backlightLevel(TDECK_DISPLAY_BL_MAX)
 {
 }
 
 bool Power::begin()
 {
     // Configure ADC for battery voltage reading
     analogReadResolution(12);  // 12-bit resolution for better accuracy
     
     // Initialize battery ADC pin
     pinMode(TDECK_BATTERY_ADC, INPUT);
     
     // Initialize backlight control
     ledcSetup(TDECK_DISPLAY_BL_CHANNEL, TDECK_DISPLAY_BL_FREQ, TDECK_DISPLAY_BL_RESOLUTION);
     ledcAttachPin(TDECK_DISPLAY_BL_PIN, TDECK_DISPLAY_BL_CHANNEL);
     setBacklight(_backlightLevel);
     
     // Initialize power management
     initPowerManagement();
     
     // Reset idle timer
     resetIdleTimer();
     
     // Read initial battery status
     update();
     
     TDECK_LOG_I("Power management initialized");
     return true;
 }
 
 bool Power::update()
 {
     // Store previous state for change detection
     float prevVoltage = _batteryVoltage;
     BatteryState prevState = _batteryState;
     bool prevCharging = _isCharging;
     bool prevUsbConnected = _isUsbConnected;
     
     // Update battery voltage
     _batteryVoltage = readBatteryVoltage();
     
     // Convert voltage to percentage
     _batteryPercentage = voltageToPercentage(_batteryVoltage);
     
     // Check charging and USB status
     _isCharging = checkCharging();
     _isUsbConnected = checkUsbConnection();
     
     // Update battery state
     updateBatteryState();
     
     // Check for auto power state transitions based on battery level
     if (_batteryState == BatteryState::CRITICAL && _powerState == PowerState::NORMAL) {
         // Auto transition to low power mode on critical battery
         setPowerState(PowerState::LOW_POWER);
     }
     
     // Check idle timeout for power saving
     if (_powerState == PowerState::NORMAL && 
         getIdleTime() > TDECK_POWER_SAVE_TIMEOUT) {
         // Enter power save mode after timeout
         setPowerState(PowerState::LOW_POWER);
     }
     
     // Check sleep timeout
     if (_powerState != PowerState::SLEEP && 
         _powerState != PowerState::DEEP_SLEEP && 
         getIdleTime() > TDECK_SLEEP_TIMEOUT) {
         // Enter sleep mode after timeout
         sleep();
     }
     
     // Return true if any value has changed
     return (prevVoltage != _batteryVoltage || 
             prevState != _batteryState || 
             prevCharging != _isCharging || 
             prevUsbConnected != _isUsbConnected);
 }
 
 float Power::getBatteryVoltage()
 {
     return _batteryVoltage;
 }
 
 uint8_t Power::getBatteryPercentage()
 {
     return _batteryPercentage;
 }
 
 BatteryState Power::getBatteryState()
 {
     return _batteryState;
 }
 
 bool Power::isCharging()
 {
     return _isCharging;
 }
 
 bool Power::isUsbConnected()
 {
     return _isUsbConnected;
 }
 
 bool Power::setPowerState(PowerState state)
 {
     // Handle state transition
     switch (state) {
         case PowerState::NORMAL:
             // Restore full performance
             setCpuFrequencyMhz(240);
             setBacklight(_backlightLevel);
             break;
             
         case PowerState::LOW_POWER:
             // Reduce CPU frequency and backlight to save power
             setCpuFrequencyMhz(80);
             setBacklight(_backlightLevel / 2);
             break;
             
         case PowerState::SLEEP:
             // Prepare for sleep
             setBacklight(0);
             // Actual sleep transition happens in sleep() method
             break;
             
         case PowerState::DEEP_SLEEP:
             // Prepare for deep sleep
             setBacklight(0);
             // Actual deep sleep transition happens in deepSleep() method
             break;
     }
     
     _powerState = state;
     TDECK_LOG_I("Power state changed to %d", (int)state);
     return true;
 }
 
 PowerState Power::getPowerState()
 {
     return _powerState;
 }
 
 bool Power::sleep(uint64_t sleepTime)
 {
     // Set power state to sleep
     setPowerState(PowerState::SLEEP);
     
     // Prepare for sleep
     TDECK_LOG_I("Entering light sleep for %llu ms", sleepTime);
     
     // Configure wakeup timer if specified
     if (sleepTime > 0) {
         esp_sleep_enable_timer_wakeup(sleepTime * 1000); // Convert to microseconds
     }
     
     // Enter light sleep mode
     esp_light_sleep_start();
     
     // Code resumes here after waking up
     TDECK_LOG_I("Woke up from light sleep");
     
     // Reset idle timer
     resetIdleTimer();
     
     // Restore normal power state
     setPowerState(PowerState::NORMAL);
     
     return true;
 }
 
 bool Power::deepSleep(uint64_t sleepTime)
 {
     // Set power state to deep sleep
     setPowerState(PowerState::DEEP_SLEEP);
     
     // Prepare for deep sleep
     TDECK_LOG_I("Entering deep sleep for %llu ms", sleepTime);
     
     // Configure wakeup timer if specified
     if (sleepTime > 0) {
         esp_sleep_enable_timer_wakeup(sleepTime * 1000); // Convert to microseconds
     }
     
     // Enter deep sleep mode
     esp_deep_sleep_start();
     
     // Code does not continue past this point
     // System will reboot on wakeup
     
     return true; // Never reached
 }
 
 void Power::resetIdleTimer()
 {
     _lastActivity = millis();
 }
 
 uint32_t Power::getIdleTime()
 {
     return millis() - _lastActivity;
 }
 
 void Power::setBacklight(uint8_t brightness)
 {
     _backlightLevel = brightness;
     ledcWrite(TDECK_DISPLAY_BL_CHANNEL, brightness);
 }
 
 uint8_t Power::getBacklight()
 {
     return _backlightLevel;
 }
 
 void Power::setWakeupPin(gpio_num_t pin, int level, bool pullup, bool pulldown)
 {
     // Configure GPIO for wakeup
     if (pullup) {
         gpio_pullup_en(pin);
     }
     
     if (pulldown) {
         gpio_pulldown_en(pin);
     }
     
     // Set the pin as wakeup source
     esp_sleep_enable_ext0_wakeup(pin, level);
     TDECK_LOG_I("Set pin %d as wakeup source", pin);
 }
 
 bool Power::isWakeFromDeepSleep()
 {
     return esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED;
 }
 
 esp_sleep_wakeup_cause_t Power::getWakeupCause()
 {
     return esp_sleep_get_wakeup_cause();
 }
 
 float Power::readBatteryVoltage()
 {
     // Read raw ADC value
     uint32_t adcValue = 0;
     const int samples = 10;
     
     // Take multiple samples for better accuracy
     for (int i = 0; i < samples; i++) {
         adcValue += analogRead(TDECK_BATTERY_ADC);
         delay(5);
     }
     adcValue /= samples;
     
     // Convert ADC reading to voltage
     // ESP32-S3 ADC has 12-bit resolution (0-4095)
     // ADC is typically configured with an input divider circuit
     // The actual conversion factor depends on the specific hardware
     
     // Typically, T-Deck has a voltage divider to measure battery voltage
     // These values may need adjustment based on specific hardware
     const float conversionFactor = 2.0f * 3.3f / 4095.0f;
     float voltage = adcValue * conversionFactor;
     
     // Optional: Apply any calibration factors
     // voltage = voltage * calibrationFactor;
     
     return voltage;
 }
 
 uint8_t Power::voltageToPercentage(float voltage)
 {
     // Convert battery voltage to percentage
     // LiPo battery discharge curve is non-linear, but we can approximate
     
     // Clamp voltage to valid range
     voltage = constrain(voltage, TDECK_BATTERY_MIN_VOLTAGE, TDECK_BATTERY_MAX_VOLTAGE);
     
     // Linear mapping from voltage to percentage
     uint8_t percentage = map(voltage * 100, 
                              TDECK_BATTERY_MIN_VOLTAGE * 100, 
                              TDECK_BATTERY_MAX_VOLTAGE * 100, 
                              0, 100);
     
     return percentage;
 }
 
 bool Power::checkCharging()
 {
     // On most T-Deck hardware, there is no dedicated charging detection pin
     // Charging status is often determined by checking if USB is connected
     // and if the battery voltage is increasing over time
     
     // This is a simplified implementation and may need enhancement
     // based on specific hardware capabilities
     static float lastVoltage = 0;
     static unsigned long lastCheckTime = 0;
     
     // Check if voltage is rising while USB connected
     bool potentiallyCharging = _isUsbConnected && (_batteryVoltage > lastVoltage);
     
     // Only update reference values periodically
     if (millis() - lastCheckTime > 30000) { // 30 second interval
         lastVoltage = _batteryVoltage;
         lastCheckTime = millis();
     }
     
     return potentiallyCharging;
 }
 
 bool Power::checkUsbConnection()
 {
     // USB detection on ESP32-S3
     // This can be determined based on VBUS detection if hardware supports it
     // or by checking if serial communication is active
     
     // Simple approximation: if voltage is stable/increasing, likely USB connected
     static bool lastConnected = false;
     
     // For a more accurate implementation, you would need hardware-specific
     // detection logic or a dedicated USB detection pin
     
     // This is a placeholder implementation
     bool connected = Serial.availableForWrite() > 0;
     
     // Add hysteresis to prevent rapid switching
     if (connected != lastConnected) {
         lastConnected = connected;
     }
     
     return lastConnected;
 }
 
 void Power::updateBatteryState()
 {
     // Determine battery state based on percentage and charging status
     if (_isCharging) {
         if (_batteryPercentage >= 95) {
             _batteryState = BatteryState::FULL;
         } else {
             _batteryState = BatteryState::CHARGING;
         }
     } else {
         if (_batteryPercentage <= TDECK_BATTERY_CRITICAL_THRESHOLD) {
             _batteryState = BatteryState::CRITICAL;
         } else if (_batteryPercentage <= TDECK_BATTERY_LOW_THRESHOLD) {
             _batteryState = BatteryState::LOW;
         } else {
             _batteryState = BatteryState::DISCHARGING;
         }
     }
 }
 
 void Power::initPowerManagement()
 {
     // Configure ESP32 power management
     esp_pm_config_esp32s3_t pmConfig = {
         .max_freq_mhz = 240,
         .min_freq_mhz = 80,
         .light_sleep_enable = true
     };
     
     // Apply power management configuration
     esp_err_t err = esp_pm_configure(&pmConfig);
     if (err != ESP_OK) {
         TDECK_LOG_E("Failed to configure power management");
     }
 }