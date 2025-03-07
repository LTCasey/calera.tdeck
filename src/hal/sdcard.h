/**
 * @file sdcard.h
 * @brief SD card interface for LilyGO T-Deck
 * 
 * This file contains the interface for the SD card functionality
 * used in the T-Deck hardware.
 */

 #ifndef TDECK_SDCARD_H
 #define TDECK_SDCARD_H
 
 #include <Arduino.h>
 #include <FS.h>
 #include <SD.h>
 #include <SPI.h>
 #include "../../config.h"
 
 /**
  * @class SDCard
  * @brief Hardware abstraction layer for the T-Deck SD card
  * 
  * This class handles the initialization and interaction with the
  * SD card slot on the T-Deck hardware.
  */
 class SDCard {
 public:
     /**
      * @brief Construct a new SDCard object
      */
     SDCard();
 
     /**
      * @brief Initialize the SD card
      * 
      * @param sckPin SCK pin number (optional, uses default if not specified)
      * @param misoPin MISO pin number (optional, uses default if not specified)
      * @param mosiPin MOSI pin number (optional, uses default if not specified)
      * @param csPin CS pin number (optional, uses default if not specified)
      * @return true if initialization was successful
      * @return false if initialization failed
      */
     bool begin(int8_t sckPin = -1, int8_t misoPin = -1, int8_t mosiPin = -1, int8_t csPin = TDECK_SD_CS);
 
     /**
      * @brief Check if SD card is available
      * 
      * @return true if SD card is available
      * @return false if SD card is not available
      */
     bool isAvailable() const;
 
     /**
      * @brief Get the SD card type
      * 
      * @return uint8_t SD card type (0=NONE, 1=MMC, 2=SD, 3=SDHC, 4=UNKNOWN)
      */
     uint8_t getType() const;
 
     /**
      * @brief Get the total size of the SD card
      * 
      * @return uint64_t Total size in bytes
      */
     uint64_t getTotalSize() const;
 
     /**
      * @brief Get the used size of the SD card
      * 
      * @return uint64_t Used size in bytes
      */
     uint64_t getUsedSize() const;
 
     /**
      * @brief Get the free size of the SD card
      * 
      * @return uint64_t Free size in bytes
      */
     uint64_t getFreeSize() const;
 
     /**
      * @brief Check if a file exists
      * 
      * @param path File path
      * @return true if file exists
      * @return false if file doesn't exist
      */
     bool exists(const char* path) const;
 
     /**
      * @brief Create a directory
      * 
      * @param path Directory path
      * @return true if directory was created successfully
      * @return false if directory creation failed
      */
     bool mkdir(const char* path);
 
     /**
      * @brief Remove a file or directory
      * 
      * @param path File or directory path
      * @return true if removal was successful
      * @return false if removal failed
      */
     bool remove(const char* path);
 
     /**
      * @brief Rename a file or directory
      * 
      * @param pathFrom Source path
      * @param pathTo Destination path
      * @return true if rename was successful
      * @return false if rename failed
      */
     bool rename(const char* pathFrom, const char* pathTo);
 
     /**
      * @brief Get a file in the SD card file system
      * 
      * @param path File path
      * @param mode File mode (FILE_READ or FILE_WRITE)
      * @return File File object
      */
     File open(const char* path, const char* mode = FILE_READ);
 
     /**
      * @brief List files in a directory
      * 
      * @param dirname Directory path
      * @param levels Maximum levels of subdirectories to traverse (default: 1)
      */
     void listDir(const char* dirname, uint8_t levels = 1);
 
     /**
      * @brief Check if a path is a directory
      * 
      * @param path Path to check
      * @return true if path is a directory
      * @return false if path is not a directory
      */
     bool isDirectory(const char* path) const;
 
     /**
      * @brief Get the file size
      * 
      * @param path File path
      * @return size_t File size in bytes, 0 if file doesn't exist
      */
     size_t fileSize(const char* path) const;
 
     /**
      * @brief Unmount the SD card
      * 
      * @return true if unmount was successful
      * @return false if unmount failed
      */
     bool end();
 
     /**
      * @brief Get the underlying FS object
      * 
      * @return fs::FS& Reference to the FS object
      */
     fs::FS& getFS();
 
 private:
     bool _available;         // SD card availability flag
     SPIClass* _spi;          // SPI interface
     uint8_t _type;           // SD card type
     
     /**
      * @brief Get SD card type as string
      * 
      * @return const char* Card type string
      */
     const char* getTypeString() const;
 };
 
 // Global SD card instance
 extern SDCard sdcard;
 
 #endif // TDECK_SDCARD_H