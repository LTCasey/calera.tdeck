/**
 * @file file_browser.h
 * @brief File browser application for T-Deck UI
 * 
 * This header defines the interface for the file browser application
 * which provides navigation and management of files on the device.
 */

 #ifndef TDECK_FILE_BROWSER_H
 #define TDECK_FILE_BROWSER_H
 
 #include <string>
 #include <functional>
 
 /**
  * Callback for file selection
  * @param path The path to the selected file
  * @param filename The name of the selected file
  */
 using FileSelectionCallback = std::function<void(const std::string &path, const std::string &filename)>;
 
 /**
  * @brief File browser application
  * 
  * Provides a UI for browsing and managing files on the SD card and internal storage.
  */
 class FileBrowser {
 public:
     /**
      * @brief Initialize the file browser
      */
     static void init();
     
     /**
      * @brief Show the file browser
      * 
      * @param select_mode If true, enters file selection mode for picking a file
      * @param callback Callback function to call when a file is selected (in select mode only)
      */
     static void show(bool select_mode = false, FileSelectionCallback callback = nullptr);
     
     /**
      * @brief Close the file browser
      */
     static void close();
     
     /**
      * @brief Set the current directory path
      * 
      * @param path New directory path
      */
     static void set_path(const std::string &path);
     
     /**
      * @brief Get the current directory path
      * 
      * @return std::string Current path
      */
     static std::string get_current_path();
 };
 
 #endif // TDECK_FILE_BROWSER_H