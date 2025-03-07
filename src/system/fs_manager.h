/**
 * @file fs_manager.h
 * @brief File system management for T-Deck UI
 * 
 * This header defines the interface for file system operations
 * including SD card and internal storage access.
 */

 #ifndef TDECK_FS_MANAGER_H
 #define TDECK_FS_MANAGER_H
 
 #include <string>
 #include <vector>
 
 /**
  * @brief File information structure
  */
 struct FSFileInfo {
     std::string name;        ///< File or directory name
     size_t size;             ///< File size in bytes (0 for directories)
     bool is_directory;       ///< true if entry is a directory
     uint32_t modified_time;  ///< Last modified time (Unix timestamp)
 };
 
 /**
  * @brief File system manager
  * 
  * Provides file operations for SD card and internal storage.
  */
 class FSManager {
 public:
     /**
      * @brief Initialize the file system
      * 
      * @return true if initialization successful
      */
     static bool init();
     
     /**
      * @brief Check if SD card is available
      * 
      * @return true if SD card is mounted
      */
     static bool is_sd_available();
     
     /**
      * @brief List contents of a directory
      * 
      * @param path Directory path
      * @param entries Vector to populate with file entries
      * @return true if successful
      */
     static bool list_directory(const std::string &path, std::vector<FSFileInfo> &entries);
     
     /**
      * @brief Create a new directory
      * 
      * @param path Directory path to create
      * @return true if successful
      */
     static bool create_directory(const std::string &path);
     
     /**
      * @brief Remove a directory
      * 
      * @param path Directory path to remove
      * @return true if successful
      */
     static bool remove_directory(const std::string &path);
     
     /**
      * @brief Rename a file or directory
      * 
      * @param old_path Original path
      * @param new_path New path
      * @return true if successful
      */
     static bool rename_file(const std::string &old_path, const std::string &new_path);
     
     /**
      * @brief Remove a file
      * 
      * @param path File path to remove
      * @return true if successful
      */
     static bool remove_file(const std::string &path);
     
     /**
      * @brief Get file size
      * 
      * @param path File path
      * @return File size in bytes, or -1 if error
      */
     static int64_t get_file_size(const std::string &path);
     
     /**
      * @brief Read file content
      * 
      * @param path File path
      * @param buffer Buffer to store file content
      * @param max_size Maximum size to read
      * @return Number of bytes read, or -1 if error
      */
     static int64_t read_file(const std::string &path, uint8_t *buffer, size_t max_size);
     
     /**
      * @brief Write file content
      * 
      * @param path File path
      * @param buffer Buffer containing data to write
      * @param size Number of bytes to write
      * @return Number of bytes written, or -1 if error
      */
     static int64_t write_file(const std::string &path, const uint8_t *buffer, size_t size);
     
     /**
      * @brief Check if a file exists
      * 
      * @param path File path
      * @return true if file exists
      */
     static bool file_exists(const std::string &path);
     
     /**
      * @brief Get total and free space information
      * 
      * @param path Mount point path
      * @param total_bytes Variable to receive total bytes
      * @param free_bytes Variable to receive free bytes
      * @return true if successful
      */
     static bool get_space_info(const std::string &path, uint64_t &total_bytes, uint64_t &free_bytes);
     
     /**
      * @brief Get last file system error
      * 
      * @return Error message string
      */
     static std::string get_last_error();
 };
 
 #endif // TDECK_FS_MANAGER_H