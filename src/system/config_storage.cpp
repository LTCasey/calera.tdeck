/**
 * @file config_storage.cpp
 * @brief Implementation of configuration storage system for T-Deck UI Firmware
 */

 #include "config_storage.h"

 // Create global instance
 ConfigStorage configStorage;
 
 // Config file paths (from config.h)
 const char* CONFIG_PATHS[CONFIG_CATEGORY_COUNT] = {
     TDECK_FS_CONFIG_FILE,           // CONFIG_SYSTEM
     TDECK_FS_WIFI_CONFIG_FILE,      // CONFIG_WIFI
     TDECK_FS_BT_CONFIG_FILE,        // CONFIG_BLUETOOTH
     TDECK_FS_LORA_CONFIG_FILE,      // CONFIG_LORA
     "/config/ui.json",              // CONFIG_UI
     "/config/power.json"            // CONFIG_POWER
 };
 
 // Size of JSON documents for each configuration type
 #define JSON_SIZE_SYSTEM 1024
 #define JSON_SIZE_WIFI 1024
 #define JSON_SIZE_BLUETOOTH 512
 #define JSON_SIZE_LORA 512
 #define JSON_SIZE_UI 2048
 #define JSON_SIZE_POWER 512
 #define JSON_SIZE_CUSTOM 1024
 
 ConfigStorage::ConfigStorage() : _initialized(false) {
 }
 
 bool ConfigStorage::init() {
     TDECK_LOG_I("Initializing configuration storage");
     
     // Make sure config directory exists
     if (!ensureConfigDir()) {
         TDECK_LOG_E("Failed to create config directory");
         return false;
     }
     
     // Check for existing configuration files and create defaults if needed
     for (int i = 0; i < CONFIG_CATEGORY_COUNT - 1; i++) { // Exclude CONFIG_CUSTOM
         ConfigCategory category = static_cast<ConfigCategory>(i);
         
         if (!configExists(category)) {
             TDECK_LOG_I("Creating default configuration for category %d", i);
             
             // Create appropriate document size based on category
             int jsonSize;
             switch (category) {
                 case CONFIG_SYSTEM: jsonSize = JSON_SIZE_SYSTEM; break;
                 case CONFIG_WIFI: jsonSize = JSON_SIZE_WIFI; break;
                 case CONFIG_BLUETOOTH: jsonSize = JSON_SIZE_BLUETOOTH; break;
                 case CONFIG_LORA: jsonSize = JSON_SIZE_LORA; break;
                 case CONFIG_UI: jsonSize = JSON_SIZE_UI; break;
                 case CONFIG_POWER: jsonSize = JSON_SIZE_POWER; break;
                 default: jsonSize = JSON_SIZE_CUSTOM;
             }
             
             DynamicJsonDocument doc(jsonSize);
             
             // Create default configuration
             switch (category) {
                 case CONFIG_SYSTEM: createDefaultSystemConfig(doc); break;
                 case CONFIG_WIFI: createDefaultWiFiConfig(doc); break;
                 case CONFIG_BLUETOOTH: createDefaultBluetoothConfig(doc); break;
                 case CONFIG_LORA: createDefaultLoRaConfig(doc); break;
                 case CONFIG_UI: createDefaultUIConfig(doc); break;
                 case CONFIG_POWER: createDefaultPowerConfig(doc); break;
                 default: break;
             }
             
             // Save default configuration
             if (!saveConfig(category, doc)) {
                 TDECK_LOG_E("Failed to save default configuration for category %d", i);
             }
         }
     }
     
     _initialized = true;
     TDECK_LOG_I("Configuration storage initialized");
     return true;
 }
 
 bool ConfigStorage::loadConfig(ConfigCategory category, JsonDocument& doc) {
     if (!_initialized) {
         TDECK_LOG_E("Config storage not initialized");
         return false;
     }
     
     if (category == CONFIG_CUSTOM) {
         TDECK_LOG_E("Use loadCustomConfig for custom configurations");
         return false;
     }
     
     const char* path = getCategoryPath(category);
     bool result = fsManager.loadJsonFromFile(path, doc);
     
     if (!result) {
         TDECK_LOG_E("Failed to load configuration from %s", path);
     }
     
     return result;
 }
 
 bool ConfigStorage::saveConfig(ConfigCategory category, const JsonDocument& doc) {
     if (!_initialized) {
         TDECK_LOG_E("Config storage not initialized");
         return false;
     }
     
     if (category == CONFIG_CUSTOM) {
         TDECK_LOG_E("Use saveCustomConfig for custom configurations");
         return false;
     }
     
     const char* path = getCategoryPath(category);
     bool result = fsManager.saveJsonToFile(path, doc);
     
     if (!result) {
         TDECK_LOG_E("Failed to save configuration to %s", path);
     } else {
         TDECK_LOG_I("Configuration saved to %s", path);
     }
     
     return result;
 }
 
 bool ConfigStorage::loadCustomConfig(const char* name, JsonDocument& doc) {
     if (!_initialized) {
         TDECK_LOG_E("Config storage not initialized");
         return false;
     }
     
     String path = getCustomConfigPath(name);
     bool result = fsManager.loadJsonFromFile(path.c_str(), doc);
     
     if (!result) {
         TDECK_LOG_E("Failed to load custom configuration from %s", path.c_str());
     }
     
     return result;
 }
 
 bool ConfigStorage::saveCustomConfig(const char* name, const JsonDocument& doc) {
     if (!_initialized) {
         TDECK_LOG_E("Config storage not initialized");
         return false;
     }
     
     String path = getCustomConfigPath(name);
     bool result = fsManager.saveJsonToFile(path.c_str(), doc);
     
     if (!result) {
         TDECK_LOG_E("Failed to save custom configuration to %s", path.c_str());
     } else {
         TDECK_LOG_I("Custom configuration saved to %s", path.c_str());
     }
     
     return result;
 }
 
 bool ConfigStorage::resetToDefaults(ConfigCategory category) {
     if (!_initialized) {
         TDECK_LOG_E("Config storage not initialized");
         return false;
     }
     
     if (category == CONFIG_CUSTOM) {
         TDECK_LOG_E("Cannot reset custom configurations to defaults");
         return false;
     }
     
     // First delete the existing configuration
     if (!deleteConfig(category)) {
         TDECK_LOG_E("Failed to delete existing configuration for reset");
         return false;
     }
     
     // Create appropriate document size based on category
     int jsonSize;
     switch (category) {
         case CONFIG_SYSTEM: jsonSize = JSON_SIZE_SYSTEM; break;
         case CONFIG_WIFI: jsonSize = JSON_SIZE_WIFI; break;
         case CONFIG_BLUETOOTH: jsonSize = JSON_SIZE_BLUETOOTH; break;
         case CONFIG_LORA: jsonSize = JSON_SIZE_LORA; break;
         case CONFIG_UI: jsonSize = JSON_SIZE_UI; break;
         case CONFIG_POWER: jsonSize = JSON_SIZE_POWER; break;
         default: jsonSize = JSON_SIZE_CUSTOM;
     }
     
     DynamicJsonDocument doc(jsonSize);
     
     // Create default configuration
     switch (category) {
         case CONFIG_SYSTEM: createDefaultSystemConfig(doc); break;
         case CONFIG_WIFI: createDefaultWiFiConfig(doc); break;
         case CONFIG_BLUETOOTH: createDefaultBluetoothConfig(doc); break;
         case CONFIG_LORA: createDefaultLoRaConfig(doc); break;
         case CONFIG_UI: createDefaultUIConfig(doc); break;
         case CONFIG_POWER: createDefaultPowerConfig(doc); break;
         default: break;
     }
     
     // Save default configuration
     return saveConfig(category, doc);
 }
 
 bool ConfigStorage::configExists(ConfigCategory category) {
     if (category == CONFIG_CUSTOM) {
         TDECK_LOG_E("Cannot check existence of unspecified custom configuration");
         return false;
     }
     
     const char* path = getCategoryPath(category);
     return fsManager.exists(path);
 }
 
 bool ConfigStorage::deleteConfig(ConfigCategory category) {
     if (category == CONFIG_CUSTOM) {
         TDECK_LOG_E("Cannot delete unspecified custom configuration");
         return false;
     }
     
     const char* path = getCategoryPath(category);
     return fsManager.deleteFile(path);
 }
 
 const char* ConfigStorage::getCategoryPath(ConfigCategory category) {
     if (category >= 0 && category < CONFIG_CATEGORY_COUNT - 1) { // Exclude CONFIG_CUSTOM
         return CONFIG_PATHS[category];
     }
     return "";
 }
 
 String ConfigStorage::getCustomConfigPath(const char* name) {
     return String(TDECK_FS_CONFIG_DIR) + "/" + name + ".json";
 }
 
 bool ConfigStorage::ensureConfigDir() {
     if (!fsManager.exists(TDECK_FS_CONFIG_DIR)) {
         return fsManager.createDir(TDECK_FS_CONFIG_DIR);
     }
     return true;
 }
 
 // Default configuration generators
 void ConfigStorage::createDefaultSystemConfig(JsonDocument& doc) {
     doc["firmware_version"] = TDECK_FIRMWARE_VERSION;
     doc["firmware_name"] = TDECK_FIRMWARE_NAME;
     doc["device_name"] = "T-Deck";
     doc["auto_update"] = true;
     doc["debug_mode"] = TDECK_DEBUG ? true : false;
     doc["language"] = "en";
     doc["timezone"] = "UTC";
     
     // Feature flags
     JsonObject features = doc.createNestedObject("features");
     features["wifi"] = TDECK_FEATURE_WIFI ? true : false;
     features["bluetooth"] = TDECK_FEATURE_BLUETOOTH ? true : false;
     features["lora"] = TDECK_FEATURE_LORA ? true : false;
     features["sd_card"] = TDECK_FEATURE_SD_CARD ? true : false;
     features["keyboard"] = TDECK_FEATURE_KEYBOARD ? true : false;
     features["touch"] = TDECK_FEATURE_TOUCH ? true : false;
 }
 
 void ConfigStorage::createDefaultWiFiConfig(JsonDocument& doc) {
     doc["wifi_enabled"] = true;
     doc["ap_mode"] = false;
     doc["ap_ssid"] = TDECK_WIFI_AP_SSID;
     doc["ap_password"] = TDECK_WIFI_AP_PASSWORD;
     doc["ap_channel"] = TDECK_WIFI_AP_CHANNEL;
     doc["ap_hidden"] = TDECK_WIFI_AP_HIDE_SSID;
     doc["max_ap_connections"] = TDECK_WIFI_AP_MAX_CONNECTIONS;
     
     // Save known networks as an array
     JsonArray networks = doc.createNestedArray("known_networks");
     
     // Example network entry - empty by default
     // JsonObject network = networks.createNestedObject();
     // network["ssid"] = "Home WiFi";
     // network["password"] = "password123";
     // network["priority"] = 1;
 }
 
 void ConfigStorage::createDefaultBluetoothConfig(JsonDocument& doc) {
     doc["bluetooth_enabled"] = true;
     doc["device_name"] = TDECK_BT_DEVICE_NAME;
     doc["discoverable"] = true;
     doc["pairing_mode"] = false;
     
     // Paired devices array
     JsonArray devices = doc.createNestedArray("paired_devices");
     
     // Example paired device - empty by default
     // JsonObject device = devices.createNestedObject();
     // device["name"] = "My Phone";
     // device["address"] = "00:11:22:33:44:55";
     // device["trusted"] = true;
 }
 
 void ConfigStorage::createDefaultLoRaConfig(JsonDocument& doc) {
     doc["lora_enabled"] = true;
     doc["frequency"] = (uint32_t)TDECK_LORA_FREQUENCY;
     doc["bandwidth"] = (uint32_t)TDECK_LORA_BANDWIDTH;
     doc["spreading_factor"] = TDECK_LORA_SPREADING_FACTOR;
     doc["coding_rate"] = TDECK_LORA_CODING_RATE;
     doc["sync_word"] = TDECK_LORA_SYNC_WORD;
     doc["tx_power"] = 17; // Default TX power in dBm
     doc["device_id"] = ""; // To be generated at first run
     doc["enable_encryption"] = false;
     doc["encryption_key"] = ""; // To be generated if encryption enabled
 }
 
 void ConfigStorage::createDefaultUIConfig(JsonDocument& doc) {
     doc["theme"] = "dark";
     doc["display_brightness"] = 80; // 0-100%
     doc["screen_timeout"] = 30; // seconds
     doc["font_size"] = "medium"; // small, medium, large
     doc["animations_enabled"] = true;
     doc["status_bar_enabled"] = true;
     doc["clock_format"] = 24; // 12 or 24 hour
     doc["date_format"] = "YYYY-MM-DD";
     
     // Home screen layout
     JsonObject home = doc.createNestedObject("home_screen");
     home["layout"] = "grid"; // grid, list
     home["columns"] = 3;
     home["show_labels"] = true;
     
     // App order array
     JsonArray appOrder = doc.createNestedArray("app_order");
     // Default app order - empty by default, will be populated on first run
 }
 
 void ConfigStorage::createDefaultPowerConfig(JsonDocument& doc) {
     doc["display_sleep_timeout"] = TDECK_POWER_SAVE_TIMEOUT / 1000; // Convert to seconds
     doc["system_sleep_timeout"] = TDECK_SLEEP_TIMEOUT / 1000; // Convert to seconds
     doc["enable_wake_on_movement"] = true;
     doc["battery_low_warning"] = TDECK_BATTERY_LOW_THRESHOLD;
     doc["battery_critical_warning"] = TDECK_BATTERY_CRITICAL_THRESHOLD;
     doc["auto_power_off"] = true;
     doc["auto_power_off_threshold"] = 3; // % battery remaining
     doc["enable_power_saving"] = true;
 }