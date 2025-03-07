/**
 * @file config_storage.h
 * @brief Configuration storage system for T-Deck UI Firmware
 * 
 * This class handles saving and loading configuration settings to/from
 * persistent storage on the file system. It includes functionality for
 * handling different configuration categories and defaults.
 */

 #ifndef TDECK_CONFIG_STORAGE_H
 #define TDECK_CONFIG_STORAGE_H
 
 #include <Arduino.h>
 #include <ArduinoJson.h>
 #include "../config.h"
 #include "fs_manager.h"
 
 // Configuration categories
 enum ConfigCategory {
     CONFIG_SYSTEM,
     CONFIG_WIFI,
     CONFIG_BLUETOOTH,
     CONFIG_LORA,
     CONFIG_UI,
     CONFIG_POWER,
     CONFIG_CUSTOM,
     CONFIG_CATEGORY_COUNT
 };
 
 class ConfigStorage {
 public:
     /**
      * @brief Construct a new Config Storage object
      */
     ConfigStorage();
 
     /**
      * @brief Initialize the configuration storage system
      * 
      * @return true if initialization successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Load configuration for a specific category
      * 
      * @param category Category of configuration to load
      * @param doc JsonDocument to store loaded configuration
      * @return true if configuration loaded successfully
      * @return false if load failed
      */
     bool loadConfig(ConfigCategory category, JsonDocument& doc);
 
     /**
      * @brief Save configuration for a specific category
      * 
      * @param category Category of configuration to save
      * @param doc JsonDocument containing configuration to save
      * @return true if configuration saved successfully
      * @return false if save failed
      */
     bool saveConfig(ConfigCategory category, const JsonDocument& doc);
 
     /**
      * @brief Load a named custom configuration
      * 
      * @param name Name of the custom configuration
      * @param doc JsonDocument to store loaded configuration
      * @return true if configuration loaded successfully
      * @return false if load failed
      */
     bool loadCustomConfig(const char* name, JsonDocument& doc);
 
     /**
      * @brief Save a named custom configuration
      * 
      * @param name Name of the custom configuration
      * @param doc JsonDocument containing configuration to save
      * @return true if configuration saved successfully
      * @return false if save failed
      */
     bool saveCustomConfig(const char* name, const JsonDocument& doc);
 
     /**
      * @brief Reset a configuration category to defaults
      * 
      * @param category Category to reset
      * @return true if reset successful
      * @return false if reset failed
      */
     bool resetToDefaults(ConfigCategory category);
 
     /**
      * @brief Check if a configuration file exists
      * 
      * @param category Category to check
      * @return true if configuration exists
      * @return false if configuration does not exist
      */
     bool configExists(ConfigCategory category);
 
     /**
      * @brief Delete a configuration file
      * 
      * @param category Category to delete
      * @return true if deletion successful
      * @return false if deletion failed
      */
     bool deleteConfig(ConfigCategory category);
 
 private:
     bool _initialized;
     
     // Path getters
     const char* getCategoryPath(ConfigCategory category);
     String getCustomConfigPath(const char* name);
     
     // Create default configurations
     void createDefaultSystemConfig(JsonDocument& doc);
     void createDefaultWiFiConfig(JsonDocument& doc);
     void createDefaultBluetoothConfig(JsonDocument& doc);
     void createDefaultLoRaConfig(JsonDocument& doc);
     void createDefaultUIConfig(JsonDocument& doc);
     void createDefaultPowerConfig(JsonDocument& doc);
     
     // Ensure config directory exists
     bool ensureConfigDir();
 };
 
 extern ConfigStorage configStorage;
 
 #endif // TDECK_CONFIG_STORAGE_H