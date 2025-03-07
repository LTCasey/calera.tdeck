/**
 * @file battery.cpp
 * @brief Implementation of battery monitoring system for T-Deck UI Firmware
 */

 #include "battery.h"

 // Create global instance
 BatteryManager batteryManager;
 
 // ADC configuration values
 #define BATTERY_ADC_PIN TDECK_BATTERY_ADC
 #define ADC_RESOLUTION 4095
 #define ADC_REFERENCE_VOLTAGE 3.3f
 #define BATTERY_VOLTAGE_DIVIDER 2.0f  // Assuming a voltage divider is used
 
 // Update interval in milliseconds (check battery every 10 seconds)
 #define BATTERY_UPDATE_INTERVAL 10000
 
 BatteryManager::BatteryManager() :
     _voltage(0.0f),
     _percentage(0),
     _isCharging(false),
     _wasLow(false),
     _wasCritical(false),
     _lastUpdateTime(0),
     _readIndex(0),
     _voltageTotal(0),
     _lowBatteryCallback(nullptr),
     _criticalBatteryCallback(nullptr) {
     
     // Initialize voltage readings array
     for (int i = 0; i < VOLTAGE_READINGS; i++) {
         _voltageReadings[i] = 0;
     }
 }
 
 bool BatteryManager::init() {
     TDECK_LOG_I("Initializing battery management system");
     
     // Configure ADC
     analogReadResolution(12);  // 12-bit resolution for ESP32
     pinMode(BATTERY_ADC_PIN, INPUT);
     
     // Take initial readings for averaging
     for (int i = 0; i < VOLTAGE_READINGS; i++) {
         _voltageReadings[i] = readVoltage();
         _voltageTotal += _voltageReadings[i];
         delay(10);
     }
     
     // Calculate initial voltage and percentage
     _voltage = _voltageTotal / VOLTAGE_READINGS;
     _percentage = calculatePercentage(_voltage);
     
     TDECK_LOG_I("Battery initialized. Initial voltage: %.2fV, Percentage: %d%%", 
                 _voltage, _percentage);
     
     return true;
 }
 
 void BatteryManager::update() {
     unsigned long currentTime = millis();
     
     // Only update at defined intervals to reduce load
     if (currentTime - _lastUpdateTime < BATTERY_UPDATE_INTERVAL) {
         return;
     }
     
     _lastUpdateTime = currentTime;
     
     // Subtract the oldest reading
     _voltageTotal -= _voltageReadings[_readIndex];
     
     // Read new voltage
     _voltageReadings[_readIndex] = readVoltage();
     
     // Add the new reading to the total
     _voltageTotal += _voltageReadings[_readIndex];
     
     // Advance to the next position in the array
     _readIndex = (_readIndex + 1) % VOLTAGE_READINGS;
     
     // Calculate the average
     _voltage = _voltageTotal / VOLTAGE_READINGS;
     
     // Update percentage
     _percentage = calculatePercentage(_voltage);
     
     // Check charging status
     // This is a placeholder - actual charging detection will depend on hardware
     // For example, this could check a pin connected to a charging indicator
     // or monitor voltage trend over time
     
     // Detect low and critical battery conditions
     bool isLow = _percentage <= TDECK_BATTERY_LOW_THRESHOLD;
     bool isCritical = _percentage <= TDECK_BATTERY_CRITICAL_THRESHOLD;
     
     // Call callbacks if state changed
     if (isLow && !_wasLow && _lowBatteryCallback) {
         _lowBatteryCallback();
     }
     
     if (isCritical && !_wasCritical && _criticalBatteryCallback) {
         _criticalBatteryCallback();
     }
     
     _wasLow = isLow;
     _wasCritical = isCritical;
     
     // Log battery status periodically
     TDECK_LOG_I("Battery: %.2fV (%d%%)", _voltage, _percentage);
 }
 
 float BatteryManager::getVoltage() const {
     return _voltage;
 }
 
 int BatteryManager::getPercentage() const {
     return _percentage;
 }
 
 bool BatteryManager::isCharging() const {
     return _isCharging;
 }
 
 bool BatteryManager::isLow() const {
     return _percentage <= TDECK_BATTERY_LOW_THRESHOLD;
 }
 
 bool BatteryManager::isCritical() const {
     return _percentage <= TDECK_BATTERY_CRITICAL_THRESHOLD;
 }
 
 void BatteryManager::setLowBatteryCallback(void (*callback)()) {
     _lowBatteryCallback = callback;
 }
 
 void BatteryManager::setCriticalBatteryCallback(void (*callback)()) {
     _criticalBatteryCallback = callback;
 }
 
 float BatteryManager::readVoltage() {
     // Read raw ADC value
     int rawValue = analogRead(BATTERY_ADC_PIN);
     
     // Convert ADC value to voltage
     float pinVoltage = (rawValue / (float)ADC_RESOLUTION) * ADC_REFERENCE_VOLTAGE;
     
     // Calculate actual battery voltage using voltage divider
     float batteryVoltage = pinVoltage * BATTERY_VOLTAGE_DIVIDER;
     
     // Optional: Apply any calibration offset if needed
     
     return batteryVoltage;
 }
 
 int BatteryManager::calculatePercentage(float voltage) {
     // Simple linear mapping from voltage to percentage
     float percentage = (voltage - TDECK_BATTERY_MIN_VOLTAGE) / 
                       (TDECK_BATTERY_MAX_VOLTAGE - TDECK_BATTERY_MIN_VOLTAGE) * 100.0f;
     
     // Clamp percentage between 0 and 100
     if (percentage < 0) percentage = 0;
     if (percentage > 100) percentage = 100;
     
     return (int)percentage;
 }