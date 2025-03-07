/**
 * @file sdcard.cpp
 * @brief SD card implementation for LilyGO T-Deck
 * 
 * This file implements the SD card functionality for the T-Deck hardware.
 */

 #include "sdcard.h"

 // Create global SD card instance
 SDCard sdcard;
 
 SDCard::SDCard() :
     _available(false),
     _type(0)
 {
     _spi = new SPIClass(HSPI);
 }
 
 bool SDCard::begin(int8_t sckPin, int8_t misoPin, int8_t mosiPin, int8_t csPin)
 {
     // Initialize SPI for SD card
     if (sckPin >= 0 && misoPin >= 0 && mosiPin >= 0) {
         _spi->begin(sckPin, misoPin, mosiPin, csPin);
     } else {
         // Use default SPI pins if not specified
         _spi->begin();
     }
     
     // Initialize SD card
     if (!SD.begin(csPin, *_spi)) {
         TDECK_LOG_E("SD card initialization failed");
         _available = false;
         return false;
     }
     
     // Get SD card type
     _type = SD.cardType();
     
     if (_type == CARD_NONE) {
         TDECK_LOG_E("No SD card attached");
         _available = false;
         return false;
     }
     
     _available = true;
     
     // Log SD card info
     TDECK_LOG_I("SD Card initialized successfully");
     TDECK_LOG_I("Card Type: %s", getTypeString());
     TDECK_LOG_I("Card Size: %llu MB", getTotalSize() / (1024 * 1024));
     
     // Create default directories if they don't exist
     if (TDECK_FEATURE_SD_CARD) {
         mkdir(TDECK_FS_CONFIG_DIR);
         mkdir(TDECK_FS_APPS_DIR);
         mkdir(TDECK_FS_TEMP_DIR);
         mkdir(TDECK_FS_DOWNLOADS_DIR);
     }
     
     return true;
 }
 
 bool SDCard::isAvailable() const
 {
     return _available;
 }
 
 uint8_t SDCard::getType() const
 {
     return _type;
 }
 
 uint64_t SDCard::getTotalSize() const
 {
     if (!_available) {
         return 0;
     }
     
     return SD.cardSize();
 }
 
 uint64_t SDCard::getUsedSize() const
 {
     if (!_available) {
         return 0;
     }
     
     // Calculate used space (total - free)
     return getTotalSize() - getFreeSize();
 }
 
 uint64_t SDCard::getFreeSize() const
 {
     if (!_available) {
         return 0;
     }
     
     // Currently the ESP32 SD library doesn't provide a direct way to get free space
     // For a more accurate implementation, you may need to use the FATFS API directly
     
     // This is a placeholder approximation
     // In a real implementation, you would use f_getfree() from the FATFS API
     return getTotalSize() / 2; // Placeholder - assumes 50% free space
 }
 
 bool SDCard::exists(const char* path) const
 {
     if (!_available) {
         return false;
     }
     
     return SD.exists(path);
 }
 
 bool SDCard::mkdir(const char* path)
 {
     if (!_available) {
         return false;
     }
     
     // Check if directory already exists
     if (exists(path) && isDirectory(path)) {
         return true;
     }
     
     return SD.mkdir(path);
 }
 
 bool SDCard::remove(const char* path)
 {
     if (!_available) {
         return false;
     }
     
     return SD.remove(path);
 }
 
 bool SDCard::rename(const char* pathFrom, const char* pathTo)
 {
     if (!_available) {
         return false;
     }
     
     return SD.rename(pathFrom, pathTo);
 }
 
 File SDCard::open(const char* path, const char* mode)
 {
     if (!_available) {
         return File();
     }
     
     return SD.open(path, mode);
 }
 
 void SDCard::listDir(const char* dirname, uint8_t levels)
 {
     if (!_available) {
         TDECK_LOG_E("SD card not available");
         return;
     }
     
     TDECK_LOG_I("Listing directory: %s", dirname);
 
     File root = SD.open(dirname);
     if (!root) {
         TDECK_LOG_E("Failed to open directory");
         return;
     }
     
     if (!root.isDirectory()) {
         TDECK_LOG_E("Not a directory");
         return;
     }
 
     File file = root.openNextFile();
     while (file) {
         if (file.isDirectory()) {
             TDECK_LOG_I("  DIR : %s", file.name());
             if (levels) {
                 char subPath[TDECK_FS_MAX_PATH_LENGTH];
                 snprintf(subPath, TDECK_FS_MAX_PATH_LENGTH, "%s/%s", dirname, file.name());
                 listDir(subPath, levels - 1);
             }
         } else {
             TDECK_LOG_I("  FILE: %s  SIZE: %u", file.name(), file.size());
         }
         file = root.openNextFile();
     }
 }
 
 bool SDCard::isDirectory(const char* path) const
 {
     if (!_available) {
         return false;
     }
     
     File file = SD.open(path);
     if (!file) {
         return false;
     }
     
     bool isDir = file.isDirectory();
     file.close();
     
     return isDir;
 }
 
 size_t SDCard::fileSize(const char* path) const
 {
     if (!_available) {
         return 0;
     }
     
     if (!exists(path) || isDirectory(path)) {
         return 0;
     }
     
     File file = SD.open(path);
     if (!file) {
         return 0;
     }
     
     size_t size = file.size();
     file.close();
     
     return size;
 }
 
 bool SDCard::end()
 {
     if (!_available) {
         return true;
     }
     
     // The ESP32 SD library doesn't have a direct unmount function
     // We'll simulate it by ending the SPI connection
     
     SD.end();
     _spi->end();
     _available = false;
     
     TDECK_LOG_I("SD card unmounted");
     return true;
 }
 
 fs::FS& SDCard::getFS()
 {
     return SD;
 }
 
 const char* SDCard::getTypeString() const
 {
     switch (_type) {
         case CARD_NONE:
             return "NONE";
         case CARD_MMC:
             return "MMC";
         case CARD_SD:
             return "SD";
         case CARD_SDHC:
             return "SDHC";
         default:
             return "UNKNOWN";
     }
 }