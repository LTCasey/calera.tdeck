/**
 * @file file_browser.cpp
 * @brief File browser application for T-Deck UI
 * 
 * This application provides a file browser interface for navigating
 * and managing files on the SD card and internal storage.
 */

 #include "file_browser.h"
 #include "../config.h"
 #include "../hal/sdcard.h"
 #include "../system/fs_manager.h"
 #include "../ui/ui_manager.h"
 
 #include <lvgl.h>
 #include <Arduino.h>
 #include <vector>
 #include <string>
 #include <algorithm>
 
 // Private variables
 static lv_obj_t *file_browser_screen = NULL;
 static lv_obj_t *file_list = NULL;
 static lv_obj_t *path_label = NULL;
 static lv_obj_t *no_files_label = NULL;
 static lv_obj_t *status_bar = NULL;
 static lv_obj_t *context_menu = NULL;
 static lv_obj_t *rename_dialog = NULL;
 static lv_obj_t *delete_dialog = NULL;
 static lv_obj_t *new_folder_dialog = NULL;
 
 static std::string current_path = "/";
 static std::string selected_file = "";
 static std::vector<FSFileInfo> file_entries;
 static bool is_selecting_mode = false;
 static FileSelectionCallback selection_callback = NULL;
 
 // Forward declarations of event handlers
 static void file_item_event_cb(lv_event_t *e);
 static void back_btn_event_cb(lv_event_t *e);
 static void nav_up_btn_event_cb(lv_event_t *e);
 static void menu_btn_event_cb(lv_event_t *e);
 static void refresh_btn_event_cb(lv_event_t *e);
 static void context_menu_event_cb(lv_event_t *e);
 static void rename_dialog_event_cb(lv_event_t *e);
 static void delete_dialog_event_cb(lv_event_t *e);
 static void new_folder_dialog_event_cb(lv_event_t *e);
 static void keyboard_event_cb(lv_event_t *e);
 
 // Helper functions
 static void load_directory(const std::string &path);
 static void refresh_file_list();
 static void update_path_label();
 static std::string get_file_extension(const std::string &filename);
 static std::string get_file_size_str(size_t size);
 static void create_context_menu();
 static void close_context_menu();
 static void show_rename_dialog(const std::string &filename);
 static void show_delete_dialog(const std::string &filename);
 static void show_new_folder_dialog();
 static bool is_image_file(const std::string &filename);
 static bool is_text_file(const std::string &filename);
 static lv_obj_t* create_file_list_item(const FSFileInfo &file_info);
 static void handle_file_open(const std::string &path, const std::string &filename);
 
 /**
  * @brief Initialize the file browser application
  * 
  * Creates the file browser screen with navigation buttons, file list, and handlers.
  */
 void FileBrowser::init() {
     TDECK_LOG_I("Initializing File Browser");
     
     // Create file browser screen
     file_browser_screen = lv_obj_create(NULL);
     lv_obj_clear_flag(file_browser_screen, LV_OBJ_FLAG_SCROLLABLE);
     
     // Create status bar at the top
     status_bar = UIManager::create_status_bar(file_browser_screen);
     
     // Create path display label
     path_label = lv_label_create(file_browser_screen);
     lv_obj_align(path_label, LV_ALIGN_TOP_LEFT, 5, TDECK_STATUS_BAR_HEIGHT + 5);
     lv_label_set_text(path_label, "File Browser: /");
     lv_obj_set_width(path_label, TDECK_DISPLAY_WIDTH - 10);
     lv_label_set_long_mode(path_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
     
     // Create button panel at bottom
     lv_obj_t *btn_panel = lv_obj_create(file_browser_screen);
     lv_obj_clear_flag(btn_panel, LV_OBJ_FLAG_SCROLLABLE);
     lv_obj_set_size(btn_panel, TDECK_DISPLAY_WIDTH, 40);
     lv_obj_align(btn_panel, LV_ALIGN_BOTTOM_MID, 0, 0);
     lv_obj_set_style_pad_all(btn_panel, 0, 0);
     lv_obj_set_style_bg_opa(btn_panel, LV_OPA_TRANSP, 0);
     lv_obj_set_style_border_width(btn_panel, 0, 0);
     
     // Add navigation buttons
     lv_obj_t *back_btn = lv_btn_create(btn_panel);
     lv_obj_set_size(back_btn, 60, 30);
     lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 5, 0);
     lv_obj_add_event_cb(back_btn, back_btn_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *back_label = lv_label_create(back_btn);
     lv_label_set_text(back_label, "Back");
     lv_obj_center(back_label);
     
     lv_obj_t *up_btn = lv_btn_create(btn_panel);
     lv_obj_set_size(up_btn, 60, 30);
     lv_obj_align(up_btn, LV_ALIGN_LEFT_MID, 70, 0);
     lv_obj_add_event_cb(up_btn, nav_up_btn_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *up_label = lv_label_create(up_btn);
     lv_label_set_text(up_label, "Up");
     lv_obj_center(up_label);
     
     lv_obj_t *refresh_btn = lv_btn_create(btn_panel);
     lv_obj_set_size(refresh_btn, 60, 30);
     lv_obj_align(refresh_btn, LV_ALIGN_LEFT_MID, 135, 0);
     lv_obj_add_event_cb(refresh_btn, refresh_btn_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *refresh_label = lv_label_create(refresh_btn);
     lv_label_set_text(refresh_label, "Refresh");
     lv_obj_center(refresh_label);
     
     lv_obj_t *menu_btn = lv_btn_create(btn_panel);
     lv_obj_set_size(menu_btn, 60, 30);
     lv_obj_align(menu_btn, LV_ALIGN_RIGHT_MID, -5, 0);
     lv_obj_add_event_cb(menu_btn, menu_btn_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *menu_label = lv_label_create(menu_btn);
     lv_label_set_text(menu_label, "Menu");
     lv_obj_center(menu_label);
     
     // Create file list container
     file_list = lv_list_create(file_browser_screen);
     lv_obj_set_size(file_list, TDECK_DISPLAY_WIDTH - 10, 
                    TDECK_DISPLAY_HEIGHT - TDECK_STATUS_BAR_HEIGHT - 40 - 30);
     lv_obj_align(file_list, LV_ALIGN_TOP_MID, 0, TDECK_STATUS_BAR_HEIGHT + 30);
     
     // Create "No files" label (hidden by default)
     no_files_label = lv_label_create(file_browser_screen);
     lv_label_set_text(no_files_label, "No files in this directory");
     lv_obj_align(no_files_label, LV_ALIGN_CENTER, 0, 0);
     lv_obj_add_flag(no_files_label, LV_OBJ_FLAG_HIDDEN);
     
     // Initialize keyboard event handling
     lv_group_t *input_group = lv_group_create();
     lv_group_add_obj(input_group, file_list);
     lv_group_add_obj(input_group, back_btn);
     lv_group_add_obj(input_group, up_btn);
     lv_group_add_obj(input_group, refresh_btn);
     lv_group_add_obj(input_group, menu_btn);
     lv_obj_add_event_cb(file_browser_screen, keyboard_event_cb, LV_EVENT_KEY, NULL);
     UIManager::register_input_group(input_group);
     
     // Set default path and load files
     current_path = "/";
     load_directory(current_path);
 }
 
 /**
  * @brief Show the file browser screen
  * 
  * @param select_mode If true, enters file selection mode for picking a file
  * @param callback Callback function to call when a file is selected (in select mode only)
  */
 void FileBrowser::show(bool select_mode, FileSelectionCallback callback) {
     is_selecting_mode = select_mode;
     selection_callback = callback;
     
     if (file_browser_screen) {
         // Refresh file list when shown
         refresh_file_list();
         
         // Update UI based on mode
         if (is_selecting_mode) {
             lv_label_set_text(path_label, "Select a file:");
         } else {
             update_path_label();
         }
         
         // Switch to file browser screen
         UIManager::switch_screen(file_browser_screen);
     }
 }
 
 /**
  * @brief Close the file browser
  */
 void FileBrowser::close() {
     // If we're in selection mode, call callback with empty string to indicate cancellation
     if (is_selecting_mode && selection_callback) {
         selection_callback("", "");
     }
     
     // Switch back to previous screen
     UIManager::back();
 }
 
 /**
  * @brief Set the current directory path
  * 
  * @param path New directory path
  */
 void FileBrowser::set_path(const std::string &path) {
     current_path = path;
     if (current_path.empty() || current_path[0] != '/') {
         current_path = "/" + current_path;
     }
     
     // Make sure path ends with slash
     if (current_path.back() != '/') {
         current_path += '/';
     }
     
     load_directory(current_path);
 }
 
 /**
  * @brief Get the current directory path
  * 
  * @return std::string Current path
  */
 std::string FileBrowser::get_current_path() {
     return current_path;
 }
 
 /**
  * @brief Load directory contents and update UI
  * 
  * @param path Directory path to load
  */
 static void load_directory(const std::string &path) {
     TDECK_LOG_I("Loading directory: %s", path.c_str());
     
     // Clear previous file entries
     file_entries.clear();
     
     // Get file list from file system manager
     if (!FSManager::list_directory(path, file_entries)) {
         TDECK_LOG_E("Failed to list directory: %s", path.c_str());
     }
     
     // Sort entries: directories first, then files alphabetically
     std::sort(file_entries.begin(), file_entries.end(), 
               [](const FSFileInfo &a, const FSFileInfo &b) {
                   if (a.is_directory && !b.is_directory) return true;
                   if (!a.is_directory && b.is_directory) return false;
                   return a.name < b.name;
               });
     
     // Update UI
     refresh_file_list();
     update_path_label();
 }
 
 /**
  * @brief Refresh the file list UI with current entries
  */
 static void refresh_file_list() {
     // Clear the list
     lv_obj_clean(file_list);
     
     // Show "No files" label if directory is empty
     if (file_entries.empty()) {
         lv_obj_clear_flag(no_files_label, LV_OBJ_FLAG_HIDDEN);
     } else {
         lv_obj_add_flag(no_files_label, LV_OBJ_FLAG_HIDDEN);
         
         // Add each file/directory to the list
         for (const auto &entry : file_entries) {
             lv_obj_t *item = create_file_list_item(entry);
             lv_obj_add_event_cb(item, file_item_event_cb, LV_EVENT_CLICKED, 
                               (void*)&entry);
         }
     }
 }
 
 /**
  * @brief Update the path label with current path
  */
 static void update_path_label() {
     std::string display_path = "Path: " + current_path;
     lv_label_set_text(path_label, display_path.c_str());
 }
 
 /**
  * @brief Create a list item for file browser
  * 
  * @param file_info File information structure
  * @return lv_obj_t* Created list item
  */
 static lv_obj_t* create_file_list_item(const FSFileInfo &file_info) {
     // Create list button
     lv_obj_t *btn = lv_list_add_btn(file_list, NULL, file_info.name.c_str());
     lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, 0);
     
     // Add icon based on file type
     lv_obj_t *icon = lv_img_create(btn);
     
     // Set icon based on file type
     if (file_info.is_directory) {
         // Use folder icon
         lv_img_set_src(icon, LV_SYMBOL_DIRECTORY);
     } else {
         // Determine icon based on file extension
         std::string ext = get_file_extension(file_info.name);
         
         if (is_image_file(file_info.name)) {
             lv_img_set_src(icon, LV_SYMBOL_IMAGE);
         } else if (is_text_file(file_info.name)) {
             lv_img_set_src(icon, LV_SYMBOL_FILE);
         } else {
             lv_img_set_src(icon, LV_SYMBOL_FILE);
         }
     }
     
     lv_obj_align(icon, LV_ALIGN_LEFT_MID, 5, 0);
     
     // Add file name label
     lv_obj_t *name_label = lv_label_create(btn);
     lv_label_set_text(name_label, file_info.name.c_str());
     lv_obj_align(name_label, LV_ALIGN_LEFT_MID, 30, 0);
     
     // Add file size or <DIR> indicator
     lv_obj_t *size_label = lv_label_create(btn);
     if (file_info.is_directory) {
         lv_label_set_text(size_label, "<DIR>");
     } else {
         lv_label_set_text(size_label, get_file_size_str(file_info.size).c_str());
     }
     lv_obj_align(size_label, LV_ALIGN_RIGHT_MID, -10, 0);
     
     // Store file info pointer in user data
     lv_obj_set_user_data(btn, (void*)&file_info);
     
     return btn;
 }
 
 /**
  * @brief Get human-readable file size string
  * 
  * @param size File size in bytes
  * @return std::string Formatted size string (e.g., "1.2 KB")
  */
 static std::string get_file_size_str(size_t size) {
     char size_str[32];
     
     if (size < 1024) {
         sprintf(size_str, "%d B", (int)size);
     } else if (size < 1024 * 1024) {
         sprintf(size_str, "%.1f KB", (float)size / 1024);
     } else if (size < 1024 * 1024 * 1024) {
         sprintf(size_str, "%.1f MB", (float)size / (1024 * 1024));
     } else {
         sprintf(size_str, "%.1f GB", (float)size / (1024 * 1024 * 1024));
     }
     
     return std::string(size_str);
 }
 
 /**
  * @brief Get file extension from filename
  * 
  * @param filename Filename
  * @return std::string File extension (lowercase, without dot)
  */
 static std::string get_file_extension(const std::string &filename) {
     size_t dot_pos = filename.find_last_of('.');
     if (dot_pos != std::string::npos && dot_pos < filename.length() - 1) {
         std::string ext = filename.substr(dot_pos + 1);
         // Convert to lowercase
         std::transform(ext.begin(), ext.end(), ext.begin(), 
                       [](unsigned char c) { return std::tolower(c); });
         return ext;
     }
     return "";
 }
 
 /**
  * @brief Check if file is an image based on extension
  * 
  * @param filename Filename to check
  * @return true if file is an image
  */
 static bool is_image_file(const std::string &filename) {
     std::string ext = get_file_extension(filename);
     return (ext == "jpg" || ext == "jpeg" || ext == "png" || 
             ext == "bmp" || ext == "gif");
 }
 
 /**
  * @brief Check if file is a text file based on extension
  * 
  * @param filename Filename to check
  * @return true if file is a text file
  */
 static bool is_text_file(const std::string &filename) {
     std::string ext = get_file_extension(filename);
     return (ext == "txt" || ext == "log" || ext == "ini" || 
             ext == "json" || ext == "xml" || ext == "csv" || 
             ext == "md" || ext == "h" || ext == "c" || 
             ext == "cpp" || ext == "py");
 }
 
 /**
  * @brief Handle file item click event
  * 
  * @param e LVGL event
  */
 static void file_item_event_cb(lv_event_t *e) {
     lv_obj_t *target = lv_event_get_target(e);
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         FSFileInfo *file_info = (FSFileInfo*)lv_obj_get_user_data(target);
         if (!file_info) return;
         
         if (file_info->is_directory) {
             // Navigate into directory
             std::string new_path = current_path + file_info->name + "/";
             current_path = new_path;
             load_directory(current_path);
         } else {
             // Handle file according to mode
             if (is_selecting_mode) {
                 // In selection mode, call callback with selected file
                 if (selection_callback) {
                     selection_callback(current_path, file_info->name);
                 }
                 
                 // Close file browser
                 UIManager::back();
             } else {
                 // In normal mode, open/handle the file
                 handle_file_open(current_path, file_info->name);
             }
         }
     }
 }
 
 /**
  * @brief Handle file opening based on file type
  * 
  * @param path File path
  * @param filename Filename
  */
 static void handle_file_open(const std::string &path, const std::string &filename) {
     std::string full_path = path + filename;
     selected_file = filename;
     
     // Different handling based on file type
     std::string ext = get_file_extension(filename);
     
     if (is_image_file(filename)) {
         // Open image viewer
         TDECK_LOG_I("Opening image: %s", full_path.c_str());
         // TODO: Implement image viewer or call appropriate handler
     } else if (is_text_file(filename)) {
         // Open text viewer/editor
         TDECK_LOG_I("Opening text file: %s", full_path.c_str());
         // TODO: Implement text viewer or call appropriate handler
     } else {
         // Default action: show file properties or context menu
         create_context_menu();
     }
 }
 
 /**
  * @brief Back button event handler
  * 
  * @param e LVGL event
  */
 static void back_btn_event_cb(lv_event_t *e) {
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         // Close file browser
         FileBrowser::close();
     }
 }
 
 /**
  * @brief Navigate up button event handler
  * 
  * @param e LVGL event
  */
 static void nav_up_btn_event_cb(lv_event_t *e) {
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         // Navigate to parent directory
         if (current_path != "/") {
             // Remove trailing slash
             if (current_path.back() == '/') {
                 current_path.pop_back();
             }
             
             // Find last slash
             size_t last_slash = current_path.find_last_of('/');
             if (last_slash != std::string::npos) {
                 current_path = current_path.substr(0, last_slash + 1);
                 load_directory(current_path);
             }
         }
     }
 }
 
 /**
  * @brief Menu button event handler
  * 
  * @param e LVGL event
  */
 static void menu_btn_event_cb(lv_event_t *e) {
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         // Show operations menu
         create_context_menu();
     }
 }
 
 /**
  * @brief Refresh button event handler
  * 
  * @param e LVGL event
  */
 static void refresh_btn_event_cb(lv_event_t *e) {
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         // Reload current directory
         load_directory(current_path);
     }
 }
 
 /**
  * @brief Create and show context menu
  */
 static void create_context_menu() {
     // Close existing menu if open
     if (context_menu) {
         close_context_menu();
     }
     
     // Create context menu
     context_menu = lv_menu_create(file_browser_screen);
     lv_obj_set_size(context_menu, 160, 200);
     lv_obj_align(context_menu, LV_ALIGN_CENTER, 0, 0);
     
     // Add menu items
     lv_obj_t *cont = lv_menu_page_create(context_menu, NULL);
     lv_menu_set_page(context_menu, cont);
     
     lv_obj_t *section = lv_menu_section_create(cont);
     
     // Add "New Folder" item
     lv_obj_t *new_folder_btn = lv_menu_cont_create(section);
     lv_obj_t *new_folder_label = lv_label_create(new_folder_btn);
     lv_label_set_text(new_folder_label, "New Folder");
     lv_obj_set_user_data(new_folder_btn, (void*)1); // Action: New Folder
     lv_obj_add_event_cb(new_folder_btn, context_menu_event_cb, LV_EVENT_CLICKED, NULL);
     
     // Add "Rename" item (only if file selected)
     if (!selected_file.empty()) {
         lv_obj_t *rename_btn = lv_menu_cont_create(section);
         lv_obj_t *rename_label = lv_label_create(rename_btn);
         lv_label_set_text(rename_label, "Rename");
         lv_obj_set_user_data(rename_btn, (void*)2); // Action: Rename
         lv_obj_add_event_cb(rename_btn, context_menu_event_cb, LV_EVENT_CLICKED, NULL);
     }
     
     // Add "Delete" item (only if file selected)
     if (!selected_file.empty()) {
         lv_obj_t *delete_btn = lv_menu_cont_create(section);
         lv_obj_t *delete_label = lv_label_create(delete_btn);
         lv_label_set_text(delete_label, "Delete");
         lv_obj_set_user_data(delete_btn, (void*)3); // Action: Delete
         lv_obj_add_event_cb(delete_btn, context_menu_event_cb, LV_EVENT_CLICKED, NULL);
     }
     
     // Add "Cancel" item
     lv_obj_t *cancel_btn = lv_menu_cont_create(section);
     lv_obj_t *cancel_label = lv_label_create(cancel_btn);
     lv_label_set_text(cancel_label, "Cancel");
     lv_obj_set_user_data(cancel_btn, (void*)0); // Action: Cancel
     lv_obj_add_event_cb(cancel_btn, context_menu_event_cb, LV_EVENT_CLICKED, NULL);
 }
 
 /**
  * @brief Close context menu
  */
 static void close_context_menu() {
     if (context_menu) {
         lv_obj_del(context_menu);
         context_menu = NULL;
     }
 }
 
 /**
  * @brief Context menu event handler
  * 
  * @param e LVGL event
  */
 static void context_menu_event_cb(lv_event_t *e) {
     lv_obj_t *btn = lv_event_get_target(e);
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         int action = (int)lv_obj_get_user_data(btn);
         
         // Close context menu
         close_context_menu();
         
         // Handle actions
         switch (action) {
             case 1: // New Folder
                 show_new_folder_dialog();
                 break;
             case 2: // Rename
                 if (!selected_file.empty()) {
                     show_rename_dialog(selected_file);
                 }
                 break;
             case 3: // Delete
                 if (!selected_file.empty()) {
                     show_delete_dialog(selected_file);
                 }
                 break;
             case 0: // Cancel
             default:
                 // Do nothing
                 break;
         }
     }
 }
 
 /**
  * @brief Show rename dialog
  * 
  * @param filename File to rename
  */
 static void show_rename_dialog(const std::string &filename) {
     // Create modal dialog
     rename_dialog = lv_obj_create(file_browser_screen);
     lv_obj_set_size(rename_dialog, 240, 160);
     lv_obj_align(rename_dialog, LV_ALIGN_CENTER, 0, 0);
     lv_obj_add_style(rename_dialog, UIManager::get_popup_style(), 0);
     
     // Add title
     lv_obj_t *title = lv_label_create(rename_dialog);
     lv_label_set_text(title, "Rename File");
     lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
     
     // Add instruction
     lv_obj_t *instr = lv_label_create(rename_dialog);
     lv_label_set_text(instr, "Enter new name:");
     lv_obj_align(instr, LV_ALIGN_TOP_LEFT, 10, 35);
     
     // Add text input
     lv_obj_t *text_input = lv_textarea_create(rename_dialog);
     lv_obj_set_size(text_input, 220, 40);
     lv_obj_align(text_input, LV_ALIGN_TOP_MID, 0, 55);
     lv_textarea_set_text(text_input, filename.c_str());
     lv_textarea_set_max_length(text_input, TDECK_FS_MAX_PATH_LENGTH - current_path.length());
     lv_obj_add_state(text_input, LV_STATE_FOCUSED);
     
     // Add buttons
     lv_obj_t *cancel_btn = lv_btn_create(rename_dialog);
     lv_obj_set_size(cancel_btn, 100, 40);
     lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
     lv_obj_add_event_cb(cancel_btn, rename_dialog_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *cancel_label = lv_label_create(cancel_btn);
     lv_label_set_text(cancel_label, "Cancel");
     lv_obj_center(cancel_label);
     lv_obj_set_user_data(cancel_btn, (void*)0);
     
     lv_obj_t *ok_btn = lv_btn_create(rename_dialog);
     lv_obj_set_size(ok_btn, 100, 40);
     lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
     lv_obj_add_event_cb(ok_btn, rename_dialog_event_cb, LV_EVENT_CLICKED, (void*)text_input);
     lv_obj_t *ok_label = lv_label_create(ok_btn);
     lv_label_set_text(ok_label, "OK");
     lv_obj_center(ok_label);
     lv_obj_set_user_data(ok_btn, (void*)1);
 }
 
 /**
  * @brief Rename dialog event handler
  * 
  * @param e LVGL event
  */
 static void rename_dialog_event_cb(lv_event_t *e) {
     lv_obj_t *btn = lv_event_get_target(e);
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         int action = (int)lv_obj_get_user_data(btn);
         
         if (action == 1) { // OK button
             // Get text input
             lv_obj_t *text_input = (lv_obj_t*)lv_event_get_user_data(e);
             if (text_input) {
                 const char *new_name = lv_textarea_get_text(text_input);
                 std::string old_path = current_path + selected_file;
                 std::string new_path = current_path + new_name;
                 
                 // Perform rename operation
                 if (FSManager::rename_file(old_path, new_path)) {
                     TDECK_LOG_I("Renamed: %s to %s", old_path.c_str(), new_path.c_str());
                     selected_file = new_name;
                 } else {
                     TDECK_LOG_E("Failed to rename: %s to %s", old_path.c_str(), new_path.c_str());
                     // TODO: Show error message
                 }
                 
                 // Refresh file list
                 load_directory(current_path);
             }
         }
         
         // Close dialog
         lv_obj_del(rename_dialog);
         rename_dialog = NULL;
     }
 }
 
 /**
  * @brief Show delete confirmation dialog
  * 
  * @param filename File to delete
  */
 static void show_delete_dialog(const std::string &filename) {
     // Create modal dialog
     delete_dialog = lv_obj_create(file_browser_screen);
     lv_obj_set_size(delete_dialog, 240, 160);
     lv_obj_align(delete_dialog, LV_ALIGN_CENTER, 0, 0);
     lv_obj_add_style(delete_dialog, UIManager::get_popup_style(), 0);
     
     // Add title
     lv_obj_t *title = lv_label_create(delete_dialog);
     lv_label_set_text(title, "Confirm Delete");
     lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
     
     // Add confirmation message
     lv_obj_t *msg = lv_label_create(delete_dialog);
     std::string msg_text = "Delete file:\n" + filename + "?";
     lv_label_set_text(msg, msg_text.c_str());
     lv_obj_align(msg, LV_ALIGN_TOP_MID, 0, 40);
     
     // Add buttons
     lv_obj_t *cancel_btn = lv_btn_create(delete_dialog);
     lv_obj_set_size(cancel_btn, 100, 40);
     lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
     lv_obj_add_event_cb(cancel_btn, delete_dialog_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *cancel_label = lv_label_create(cancel_btn);
     lv_label_set_text(cancel_label, "Cancel");
     lv_obj_center(cancel_label);
     lv_obj_set_user_data(cancel_btn, (void*)0);
     
     lv_obj_t *ok_btn = lv_btn_create(delete_dialog);
     lv_obj_set_size(ok_btn, 100, 40);
     lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
     lv_obj_add_event_cb(ok_btn, delete_dialog_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *ok_label = lv_label_create(ok_btn);
     lv_label_set_text(ok_label, "Delete");
     lv_obj_center(ok_label);
     lv_obj_set_user_data(ok_btn, (void*)1);
 }
 
 /**
  * @brief Delete dialog event handler
  * 
  * @param e LVGL event
  */
 static void delete_dialog_event_cb(lv_event_t *e) {
     lv_obj_t *btn = lv_event_get_target(e);
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         int action = (int)lv_obj_get_user_data(btn);
         
         if (action == 1) { // Delete button
             std::string path = current_path + selected_file;
             
             // Check if it's a directory
             bool is_dir = false;
             for (const auto &entry : file_entries) {
                 if (entry.name == selected_file && entry.is_directory) {
                     is_dir = true;
                     break;
                 }
             }
             
             // Perform delete operation
             bool success = false;
             if (is_dir) {
                 success = FSManager::remove_directory(path);
             } else {
                 success = FSManager::remove_file(path);
             }
             
             if (success) {
                 TDECK_LOG_I("Deleted: %s", path.c_str());
                 selected_file = "";
             } else {
                 TDECK_LOG_E("Failed to delete: %s", path.c_str());
                 // TODO: Show error message
             }
             
             // Refresh file list
             load_directory(current_path);
         }
         
         // Close dialog
         lv_obj_del(delete_dialog);
         delete_dialog = NULL;
     }
 }
 
 /**
  * @brief Show new folder dialog
  */
 static void show_new_folder_dialog() {
     // Create modal dialog
     new_folder_dialog = lv_obj_create(file_browser_screen);
     lv_obj_set_size(new_folder_dialog, 240, 160);
     lv_obj_align(new_folder_dialog, LV_ALIGN_CENTER, 0, 0);
     lv_obj_add_style(new_folder_dialog, UIManager::get_popup_style(), 0);
     
     // Add title
     lv_obj_t *title = lv_label_create(new_folder_dialog);
     lv_label_set_text(title, "Create New Folder");
     lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
     
     // Add instruction
     lv_obj_t *instr = lv_label_create(new_folder_dialog);
     lv_label_set_text(instr, "Enter folder name:");
     lv_obj_align(instr, LV_ALIGN_TOP_LEFT, 10, 35);
     
     // Add text input
     lv_obj_t *text_input = lv_textarea_create(new_folder_dialog);
     lv_obj_set_size(text_input, 220, 40);
     lv_obj_align(text_input, LV_ALIGN_TOP_MID, 0, 55);
     lv_textarea_set_text(text_input, "New Folder");
     lv_textarea_set_max_length(text_input, TDECK_FS_MAX_PATH_LENGTH - current_path.length());
     lv_obj_add_state(text_input, LV_STATE_FOCUSED);
     
     // Add buttons
     lv_obj_t *cancel_btn = lv_btn_create(new_folder_dialog);
     lv_obj_set_size(cancel_btn, 100, 40);
     lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_LEFT, 10, -10);
     lv_obj_add_event_cb(cancel_btn, new_folder_dialog_event_cb, LV_EVENT_CLICKED, NULL);
     lv_obj_t *cancel_label = lv_label_create(cancel_btn);
     lv_label_set_text(cancel_label, "Cancel");
     lv_obj_center(cancel_label);
     lv_obj_set_user_data(cancel_btn, (void*)0);
     
     lv_obj_t *ok_btn = lv_btn_create(new_folder_dialog);
     lv_obj_set_size(ok_btn, 100, 40);
     lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
     lv_obj_add_event_cb(ok_btn, new_folder_dialog_event_cb, LV_EVENT_CLICKED, (void*)text_input);
     lv_obj_t *ok_label = lv_label_create(ok_btn);
     lv_label_set_text(ok_label, "Create");
     lv_obj_center(ok_label);
     lv_obj_set_user_data(ok_btn, (void*)1);
 }
 
 /**
  * @brief New folder dialog event handler
  * 
  * @param e LVGL event
  */
 static void new_folder_dialog_event_cb(lv_event_t *e) {
     lv_obj_t *btn = lv_event_get_target(e);
     lv_event_code_t code = lv_event_get_code(e);
     
     if (code == LV_EVENT_CLICKED) {
         int action = (int)lv_obj_get_user_data(btn);
         
         if (action == 1) { // Create button
             // Get text input
             lv_obj_t *text_input = (lv_obj_t*)lv_event_get_user_data(e);
             if (text_input) {
                 const char *folder_name = lv_textarea_get_text(text_input);
                 std::string new_dir = current_path + folder_name;
                 
                 // Create directory
                 if (FSManager::create_directory(new_dir)) {
                     TDECK_LOG_I("Created directory: %s", new_dir.c_str());
                 } else {
                     TDECK_LOG_E("Failed to create directory: %s", new_dir.c_str());
                     // TODO: Show error message
                 }
                 
                 // Refresh file list
                 load_directory(current_path);
             }
         }
         
         // Close dialog
         lv_obj_del(new_folder_dialog);
         new_folder_dialog = NULL;
     }
 }
 
 /**
  * @brief Handle keyboard events
  * 
  * @param e LVGL event
  */
 static void keyboard_event_cb(lv_event_t *e) {
     lv_event_code_t code = lv_event_get_code(e);
     uint32_t key = lv_event_get_key(e);
     
     if (code == LV_EVENT_KEY) {
         switch (key) {
             case LV_KEY_ESC:
                 // Close dialogs if open
                 if (context_menu) {
                     close_context_menu();
                 } else if (rename_dialog) {
                     lv_obj_del(rename_dialog);
                     rename_dialog = NULL;
                 } else if (delete_dialog) {
                     lv_obj_del(delete_dialog);
                     delete_dialog = NULL;
                 } else if (new_folder_dialog) {
                     lv_obj_del(new_folder_dialog);
                     new_folder_dialog = NULL;
                 } else {
                     // Exit file browser
                     FileBrowser::close();
                 }
                 break;
                 
             case LV_KEY_BACKSPACE:
                 // Navigate up
                 if (current_path != "/" && 
                     !context_menu && !rename_dialog && 
                     !delete_dialog && !new_folder_dialog) {
                     
                     // Remove trailing slash
                     if (current_path.back() == '/') {
                         current_path.pop_back();
                     }
                     
                     // Find last slash
                     size_t last_slash = current_path.find_last_of('/');
                     if (last_slash != std::string::npos) {
                         current_path = current_path.substr(0, last_slash + 1);
                         load_directory(current_path);
                     }
                 }
                 break;
                 
             case LV_KEY_HOME:
                 // Go to root directory
                 if (!context_menu && !rename_dialog && 
                     !delete_dialog && !new_folder_dialog) {
                     
                     current_path = "/";
                     load_directory(current_path);
                 }
                 break;
                 
             case LV_KEY_REFRESH:
                 // Refresh current directory
                 if (!context_menu && !rename_dialog && 
                     !delete_dialog && !new_folder_dialog) {
                     
                     load_directory(current_path);
                 }
                 break;
                 
             case LV_KEY_DELETE:
                 // Delete selected file
                 if (!context_menu && !rename_dialog && 
                     !delete_dialog && !new_folder_dialog && 
                     !selected_file.empty()) {
                     
                     show_delete_dialog(selected_file);
                 }
                 break;
                 
             case LV_KEY_F2:
                 // Rename selected file
                 if (!context_menu && !rename_dialog && 
                     !delete_dialog && !new_folder_dialog && 
                     !selected_file.empty()) {
                     
                     show_rename_dialog(selected_file);
                 }
                 break;
         }
     }
 }