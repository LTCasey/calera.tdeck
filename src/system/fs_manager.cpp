/**
 * @file fs_manager.cpp
 * @brief File system management implementation for T-Deck UI
 * 
 * This file implements the file system operations for SD card and internal storage.
 */

 #include "fs_manager.h"
 #include "../config.h"
 #include "../hal/sdcard.h"
 
 #include <Arduino.h>
 #include <FS.h>
 #include <SD.h>
 #include <SPIFFS.h>
 #include <SPI.h>
 #include <algorithm>
 #include <vector>
 #include <dirent.h>
 #include <sys/stat.h>
 
 // Static variables
 static std::string last_error;
 static bool sd_available = false;
 static bool spiffs_available = false;
 
 // Helper functions
 static fs::FS& get_fs_for_path(const std::string &path);
 static void set_error(const std::string &error);
 static std::string get_fs_type_for_path(const std::string &path);
 static bool is_root_directory(const std::string &path);
 static std::string normalize_path(const std::string &path);
 
 /**
  * @brief Initialize the file system
  * 
  * Initializes both SD card and internal storage
  * 
  * @return true if at least one file system initialized successfully
  */
 bool FSManager::init() {
     TDECK_LOG_I("Initializing file systems");
     
     // Initialize SPIFFS
     if(SPIFFS.begin(true)) {
         TDECK_LOG_I("SPIFFS initialized successfully");
         spiffs_available = true;
         
         // Create standard directories if they don't exist
         if(!file_exists(TDECK_FS_CONFIG_DIR)) {
             create_directory(TDECK_FS_CONFIG_DIR);
         }
         if(!file_exists(TDECK_FS_APPS_DIR)) {
             create_directory(TDECK_FS_APPS_DIR);
         }
         if(!file_exists(TDECK_FS_TEMP_DIR)) {
             create_directory(TDECK_FS_TEMP_DIR);
         }
         if(!file_exists(TDECK_FS_DOWNLOADS_DIR)) {
             create_directory(TDECK_FS_DOWNLOADS_DIR);
         }
     } else {
         TDECK_LOG_E("Failed to initialize SPIFFS");
         spiffs_available = false;
     }
     
     // Initialize SD card
     if(TDECK_FEATURE_SD_CARD) {
         SPIClass *spi = new SPIClass(HSPI);
         spi->begin();
         
         // Mount the SD card with the SD_CS pin and SPI
         if(SD.begin(TDECK_SD_CS, *spi)) {
             TDECK_LOG_I("SD card initialized successfully");
             sd_available = true;
         } else {
             TDECK_LOG_E("Failed to initialize SD card");
             sd_available = false;
         }
     }
     
     return spiffs_available || sd_available;
 }
 
 /**
  * @brief Check if SD card is available
  * 
  * @return true if SD card is mounted
  */
 bool FSManager::is_sd_available() {
     return sd_available;
 }
 
 /**
  * @brief List contents of a directory
  * 
  * @param path Directory path
  * @param entries Vector to populate with file entries
  * @return true if successful
  */
 bool FSManager::list_directory(const std::string &path, std::vector<FSFileInfo> &entries) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     if(norm_path == "/") {
         // Special case for root: show both filesystems if available
         if(spiffs_available) {
             FSFileInfo spiffs_entry;
             spiffs_entry.name = "flash";
             spiffs_entry.is_directory = true;
             spiffs_entry.size = 0;
             spiffs_entry.modified_time = 0;
             entries.push_back(spiffs_entry);
         }
         
         if(sd_available) {
             FSFileInfo sd_entry;
             sd_entry.name = "sd";
             sd_entry.is_directory = true;
             sd_entry.size = 0;
             sd_entry.modified_time = 0;
             entries.push_back(sd_entry);
         }
         
         return true;
     }
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     File dir = fs.open(fs_path.c_str());
     if(!dir || !dir.isDirectory()) {
         set_error("Failed to open directory: " + norm_path);
         TDECK_LOG_E("Failed to open directory: %s", norm_path.c_str());
         return false;
     }
     
     File file = dir.openNextFile();
     while(file) {
         FSFileInfo entry;
         entry.name = file.name();
         
         // Remove path prefix if present
         size_t last_slash = entry.name.find_last_of('/');
         if(last_slash != std::string::npos) {
             entry.name = entry.name.substr(last_slash + 1);
         }
         
         entry.size = file.size();
         entry.is_directory = file.isDirectory();
         entry.modified_time = file.getLastWrite();
         
         // Skip hidden files (starting with .)
         if(entry.name.length() > 0 && entry.name[0] != '.') {
             entries.push_back(entry);
         }
         
         file = dir.openNextFile();
     }
     
     return true;
 }
 
 /**
  * @brief Create a new directory
  * 
  * @param path Directory path to create
  * @return true if successful
  */
 bool FSManager::create_directory(const std::string &path) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     if(fs.mkdir(fs_path.c_str())) {
         TDECK_LOG_I("Created directory: %s", norm_path.c_str());
         return true;
     } else {
         set_error("Failed to create directory: " + norm_path);
         TDECK_LOG_E("Failed to create directory: %s", norm_path.c_str());
         return false;
     }
 }
 
 /**
  * @brief Remove a directory
  * 
  * @param path Directory path to remove
  * @return true if successful
  */
 bool FSManager::remove_directory(const std::string &path) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     // Prevent deleting root directories
     if(is_root_directory(norm_path)) {
         set_error("Cannot remove root directory: " + norm_path);
         TDECK_LOG_E("Cannot remove root directory: %s", norm_path.c_str());
         return false;
     }
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     if(fs.rmdir(fs_path.c_str())) {
         TDECK_LOG_I("Removed directory: %s", norm_path.c_str());
         return true;
     } else {
         set_error("Failed to remove directory: " + norm_path);
         TDECK_LOG_E("Failed to remove directory: %s", norm_path.c_str());
         return false;
     }
 }
 
 /**
  * @brief Rename a file or directory
  * 
  * @param old_path Original path
  * @param new_path New path
  * @return true if successful
  */
 bool FSManager::rename_file(const std::string &old_path, const std::string &new_path) {
     std::string norm_old_path = normalize_path(old_path);
     std::string norm_new_path = normalize_path(new_path);
     
     // Make sure both paths are on the same filesystem
     if(get_fs_type_for_path(norm_old_path) != get_fs_type_for_path(norm_new_path)) {
         set_error("Cannot rename across different filesystems");
         TDECK_LOG_E("Cannot rename across different filesystems: %s to %s", 
                   norm_old_path.c_str(), norm_new_path.c_str());
         return false;
     }
     
     fs::FS &fs = get_fs_for_path(norm_old_path);
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_old_path = norm_old_path;
     std::string fs_new_path = norm_new_path;
     std::string fs_type = get_fs_type_for_path(norm_old_path);
     
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos_old = norm_old_path.find('/', 1);
         if(pos_old != std::string::npos) {
             fs_old_path = norm_old_path.substr(pos_old);
         } else {
             fs_old_path = "/";
         }
         
         size_t pos_new = norm_new_path.find('/', 1);
         if(pos_new != std::string::npos) {
             fs_new_path = norm_new_path.substr(pos_new);
         } else {
             fs_new_path = "/";
         }
     }
     
     if(fs.rename(fs_old_path.c_str(), fs_new_path.c_str())) {
         TDECK_LOG_I("Renamed: %s to %s", norm_old_path.c_str(), norm_new_path.c_str());
         return true;
     } else {
         set_error("Failed to rename: " + norm_old_path + " to " + norm_new_path);
         TDECK_LOG_E("Failed to rename: %s to %s", norm_old_path.c_str(), norm_new_path.c_str());
         return false;
     }
 }
 
 /**
  * @brief Remove a file
  * 
  * @param path File path to remove
  * @return true if successful
  */
 bool FSManager::remove_file(const std::string &path) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     if(fs.remove(fs_path.c_str())) {
         TDECK_LOG_I("Removed file: %s", norm_path.c_str());
         return true;
     } else {
         set_error("Failed to remove file: " + norm_path);
         TDECK_LOG_E("Failed to remove file: %s", norm_path.c_str());
         return false;
     }
 }
 
 /**
  * @brief Get file size
  * 
  * @param path File path
  * @return File size in bytes, or -1 if error
  */
 int64_t FSManager::get_file_size(const std::string &path) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     File file = fs.open(fs_path.c_str(), FILE_READ);
     if(!file) {
         set_error("Failed to open file: " + norm_path);
         TDECK_LOG_E("Failed to open file: %s", norm_path.c_str());
         return -1;
     }
     
     int64_t size = file.size();
     file.close();
     return size;
 }
 
 /**
  * @brief Read file content
  * 
  * @param path File path
  * @param buffer Buffer to store file content
  * @param max_size Maximum size to read
  * @return Number of bytes read, or -1 if error
  */
 int64_t FSManager::read_file(const std::string &path, uint8_t *buffer, size_t max_size) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     File file = fs.open(fs_path.c_str(), FILE_READ);
     if(!file) {
         set_error("Failed to open file for reading: " + norm_path);
         TDECK_LOG_E("Failed to open file for reading: %s", norm_path.c_str());
         return -1;
     }
     
     int64_t bytes_read = file.read(buffer, max_size);
     file.close();
     
     if(bytes_read < 0) {
         set_error("Error reading from file: " + norm_path);
         TDECK_LOG_E("Error reading from file: %s", norm_path.c_str());
         return -1;
     }
     
     return bytes_read;
 }
 
 /**
  * @brief Write file content
  * 
  * @param path File path
  * @param buffer Buffer containing data to write
  * @param size Number of bytes to write
  * @return Number of bytes written, or -1 if error
  */
 int64_t FSManager::write_file(const std::string &path, const uint8_t *buffer, size_t size) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     File file = fs.open(fs_path.c_str(), FILE_WRITE);
     if(!file) {
         set_error("Failed to open file for writing: " + norm_path);
         TDECK_LOG_E("Failed to open file for writing: %s", norm_path.c_str());
         return -1;
     }
     
     int64_t bytes_written = file.write(buffer, size);
     file.close();
     
     if(bytes_written != size) {
         set_error("Error writing to file: " + norm_path);
         TDECK_LOG_E("Error writing to file: %s", norm_path.c_str());
         return -1;
     }
     
     return bytes_written;
 }
 
 /**
  * @brief Check if a file exists
  * 
  * @param path File path
  * @return true if file exists
  */
 bool FSManager::file_exists(const std::string &path) {
     std::string norm_path = normalize_path(path);
     fs::FS &fs = get_fs_for_path(norm_path);
     
     // Special case for virtual root and filesystem roots
     if(norm_path == "/" || norm_path == "/flash" || norm_path == "/sd") {
         return true;
     }
     
     // Remove leading slash for non-root FS-specific paths
     std::string fs_path = norm_path;
     std::string fs_type = get_fs_type_for_path(norm_path);
     if(fs_type == "flash" || fs_type == "sd") {
         size_t pos = norm_path.find('/', 1);
         if(pos != std::string::npos) {
             fs_path = norm_path.substr(pos);
         } else {
             fs_path = "/";
         }
     }
     
     return fs.exists(fs_path.c_str());
 }
 
 /**
  * @brief Get total and free space information
  * 
  * @param path Mount point path
  * @param total_bytes Variable to receive total bytes
  * @param free_bytes Variable to receive free bytes
  * @return true if successful
  */
 bool FSManager::get_space_info(const std::string &path, uint64_t &total_bytes, uint64_t &free_bytes) {
     std::string fs_type = get_fs_type_for_path(path);
     
     if(fs_type == "flash") {
         total_bytes = SPIFFS.totalBytes();
         free_bytes = SPIFFS.usedBytes();
         return true;
     } else if(fs_type == "sd" && sd_available) {
         // SD card total size and free space
         total_bytes = SD.totalBytes();
         free_bytes = SD.usedBytes();
         return true;
     } else {
         total_bytes = 0;
         free_bytes = 0;
         set_error("Invalid path for space info: " + path);
         TDECK_LOG_E("Invalid path for space info: %s", path.c_str());
         return false;
     }
 }
 
 /**
  * @brief Get last file system error
  * 
  * @return Error message string
  */
 std::string FSManager::get_last_error() {
     return last_error;
 }
 
 /**
  * @brief Get the appropriate filesystem for the given path
  * 
  * @param path The file path
  * @return Reference to the appropriate filesystem
  */
 static fs::FS& get_fs_for_path(const std::string &path) {
     std::string fs_type = get_fs_type_for_path(path);
     
     if(fs_type == "sd" && sd_available) {
         return SD;
     } else {
         return SPIFFS;
     }
 }
 
 /**
  * @brief Get the filesystem type for the given path
  * 
  * @param path The file path
  * @return String indicating filesystem type ("flash" or "sd")
  */
 static std::string get_fs_type_for_path(const std::string &path) {
     if(path.length() >= 3 && path.substr(0, 3) == "/sd") {
         return "sd";
     } else if(path.length() >= 6 && path.substr(0, 6) == "/flash") {
         return "flash";
     } else {
         // Default to flash for other paths
         return "flash";
     }
 }
 
 /**
  * @brief Set the last error message
  * 
  * @param error Error message
  */
 static void set_error(const std::string &error) {
     last_error = error;
 }
 
 /**
  * @brief Check if the path is a root directory
  * 
  * @param path Path to check
  * @return true if path is a root directory
  */
 static bool is_root_directory(const std::string &path) {
     return (path == "/" || path == "/flash" || path == "/sd" || 
             path == "/flash/" || path == "/sd/");
 }
 
 /**
  * @brief Normalize a file path (ensure leading slash, etc.)
  * 
  * @param path Path to normalize
  * @return Normalized path
  */
 static std::string normalize_path(const std::string &path) {
     std::string result = path;
     
     // Ensure path starts with a slash
     if(result.empty() || result[0] != '/') {
         result = "/" + result;
     }
     
     // Remove duplicate slashes
     size_t pos = 0;
     while((pos = result.find("//", pos)) != std::string::npos) {
         result.erase(pos, 1);
     }
     
     return result;
 }