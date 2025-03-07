/**
 * @file terminal.cpp
 * @brief Implementation of the Terminal application
 */

 #include "terminal.h"
 #include <algorithm>
 #include <sstream>
 #include <iomanip>
 #include <time.h>
 #include <sys/time.h>
 #include "../../hal/keyboard.h"
 #include "../../ui/ui_manager.h"
 
 // Static event callback functions
 static void terminal_event_handler(lv_obj_t* obj, lv_event_t event) {
     if (event == LV_EVENT_KEY) {
         // Get the user data (Terminal instance)
         Terminal* terminal = static_cast<Terminal*>(lv_obj_get_user_data(obj));
         
         // Get the key
         uint32_t key = *((uint32_t*)lv_event_get_data());
         
         // Pass to Terminal instance
         if (terminal) {
             terminal->handleKeyEvent(key);
         }
     }
 }
 
 // Constructor
 Terminal::Terminal() : 
     _parent(nullptr),
     _container(nullptr),
     _textArea(nullptr),
     _cmdLine(nullptr),
     _scrollBar(nullptr),
     _running(false),
     _cmdHistoryPos(0) {
 }
 
 // Destructor
 Terminal::~Terminal() {
     stop();
 }
 
 // Initialize terminal application
 bool Terminal::init(lv_obj_t* parent) {
     _parent = parent;
     
     // Create terminal container
     _container = lv_cont_create(_parent, NULL);
     lv_obj_set_size(_container, lv_obj_get_width(_parent), lv_obj_get_height(_parent) - TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_pos(_container, 0, TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_hidden(_container, true);
     
     // Set a dark style for the terminal
     static lv_style_t style_terminal;
     lv_style_copy(&style_terminal, &lv_style_plain);
     style_terminal.body.main_color = lv_color_hex(0x000000);   // Black background
     style_terminal.body.grad_color = lv_color_hex(0x000000);
     style_terminal.text.color = lv_color_hex(TERM_COLOR_NORMAL);
     style_terminal.text.font = &lv_font_montserrat_14;
     
     lv_cont_set_style(_container, LV_CONT_STYLE_MAIN, &style_terminal);
     
     // Create text area for terminal output
     _textArea = lv_ta_create(_container, NULL);
     lv_obj_set_size(_textArea, lv_obj_get_width(_container), lv_obj_get_height(_container) - 30);
     lv_obj_align(_textArea, _container, LV_ALIGN_IN_TOP_MID, 0, 0);
     lv_ta_set_cursor_type(_textArea, LV_CURSOR_NONE);
     lv_ta_set_text(_textArea, ""); // Empty text
     lv_ta_set_style(_textArea, LV_TA_STYLE_BG, &style_terminal);
     lv_ta_set_scrollbar_mode(_textArea, LV_SCROLLBAR_MODE_AUTO);
     lv_ta_set_one_line(_textArea, false);
     lv_ta_set_cursor_click_pos(_textArea, false);
     lv_ta_set_text_align(_textArea, LV_LABEL_ALIGN_LEFT);
     lv_ta_set_pwd_mode(_textArea, false);
     lv_obj_set_event_cb(_textArea, terminal_event_handler);
     lv_obj_set_user_data(_textArea, this);
     
     // Create command line input
     _cmdLine = lv_ta_create(_container, NULL);
     lv_obj_set_size(_cmdLine, lv_obj_get_width(_container), 30);
     lv_obj_align(_cmdLine, _container, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
     lv_ta_set_text(_cmdLine, TERM_PROMPT);
     lv_ta_set_cursor_pos(_cmdLine, strlen(TERM_PROMPT));
     lv_ta_set_style(_cmdLine, LV_TA_STYLE_BG, &style_terminal);
     lv_ta_set_one_line(_cmdLine, true);
     lv_ta_set_cursor_type(_cmdLine, LV_CURSOR_LINE);
     lv_obj_set_event_cb(_cmdLine, terminal_event_handler);
     lv_obj_set_user_data(_cmdLine, this);
     
     // Set focus on command line
     lv_group_t* g = lv_group_create();
     lv_group_add_obj(g, _cmdLine);
     lv_group_set_focus_cb(g, NULL);
     lv_indev_t* cur_drv = lv_indev_get_next(NULL);
     while (cur_drv) {
         if (cur_drv->driver.type == LV_INDEV_TYPE_KEYPAD) {
             lv_indev_set_group(cur_drv, g);
         }
         cur_drv = lv_indev_get_next(cur_drv);
     }
     
     // Initialize built-in commands
     initCommands();
     
     // Print welcome message
     println("T-Deck Terminal v1.0", TERM_COLOR_SYSTEM);
     println("Type 'help' for a list of commands", TERM_COLOR_SYSTEM);
     println("", TERM_COLOR_NORMAL);
     
     TDECK_LOG_I("Terminal initialized");
     return true;
 }
 
 // Start terminal
 void Terminal::start() {
     if (!_running) {
         lv_obj_set_hidden(_container, false);
         lv_group_focus_obj(_cmdLine);
         _running = true;
         TDECK_LOG_I("Terminal started");
     }
 }
 
 // Stop terminal
 void Terminal::stop() {
     if (_running) {
         lv_obj_set_hidden(_container, true);
         _running = false;
         TDECK_LOG_I("Terminal stopped");
     }
 }
 
 // Check if terminal is running
 bool Terminal::isRunning() const {
     return _running;
 }
 
 // Add text to terminal
 void Terminal::print(const String& text, uint32_t color) {
     // Add to history with color markup
     String coloredText = String("#") + String(color, HEX) + " " + text + String("#");
     _history.push_back(coloredText);
     
     // Limit history size
     while (_history.size() > TERM_MAX_HISTORY) {
         _history.pop_front();
     }
     
     // Update display
     updateDisplay();
 }
 
 // Add line to terminal
 void Terminal::println(const String& text, uint32_t color) {
     print(text + "\n", color);
 }
 
 // Clear terminal
 void Terminal::clear() {
     _history.clear();
     lv_ta_set_text(_textArea, "");
 }
 
 // Handle keyboard events
 bool Terminal::handleKeyEvent(uint32_t key) {
     if (!_running) return false;
     
     // Get current command line text
     String cmdText = lv_ta_get_text(_cmdLine);
     int cursorPos = lv_ta_get_cursor_pos(_cmdLine);
     
     // Handle different keys
     switch (key) {
         case LV_KEY_ENTER: {
             // Process command if not empty
             if (cmdText.length() > strlen(TERM_PROMPT)) {
                 String command = cmdText.substring(strlen(TERM_PROMPT));
                 
                 // Print command to terminal
                 println(cmdText, TERM_COLOR_COMMAND);
                 
                 // Process command
                 processCommand(command);
                 
                 // Add to command history
                 if (command.length() > 0) {
                     _cmdHistory.push_back(command);
                     while (_cmdHistory.size() > TERM_MAX_CMD_HISTORY) {
                         _cmdHistory.pop_front();
                     }
                 }
                 
                 // Reset command line
                 lv_ta_set_text(_cmdLine, TERM_PROMPT);
                 lv_ta_set_cursor_pos(_cmdLine, strlen(TERM_PROMPT));
                 _cmdHistoryPos = 0;
             }
             return true;
         }
         
         case LV_KEY_BACKSPACE: {
             // Prevent deleting prompt
             if (cursorPos <= strlen(TERM_PROMPT)) {
                 return true;
             }
             break;
         }
         
         case LV_KEY_LEFT: {
             // Prevent cursor going before prompt
             if (cursorPos <= strlen(TERM_PROMPT)) {
                 return true;
             }
             break;
         }
         
         case LV_KEY_HOME: {
             // Go to beginning of input (after prompt)
             lv_ta_set_cursor_pos(_cmdLine, strlen(TERM_PROMPT));
             return true;
         }
         
         case LV_KEY_UP: {
             // Navigate command history (older commands)
             if (!_cmdHistory.empty() && _cmdHistoryPos < _cmdHistory.size()) {
                 _cmdHistoryPos++;
                 int index = _cmdHistory.size() - _cmdHistoryPos;
                 String historyCmd = _cmdHistory[index];
                 lv_ta_set_text(_cmdLine, (String(TERM_PROMPT) + historyCmd).c_str());
                 lv_ta_set_cursor_pos(_cmdLine, (strlen(TERM_PROMPT) + historyCmd.length()));
             }
             return true;
         }
         
         case LV_KEY_DOWN: {
             // Navigate command history (newer commands)
             if (_cmdHistoryPos > 0) {
                 _cmdHistoryPos--;
                 if (_cmdHistoryPos == 0) {
                     lv_ta_set_text(_cmdLine, TERM_PROMPT);
                     lv_ta_set_cursor_pos(_cmdLine, strlen(TERM_PROMPT));
                 } else {
                     int index = _cmdHistory.size() - _cmdHistoryPos;
                     String historyCmd = _cmdHistory[index];
                     lv_ta_set_text(_cmdLine, (String(TERM_PROMPT) + historyCmd).c_str());
                     lv_ta_set_cursor_pos(_cmdLine, (strlen(TERM_PROMPT) + historyCmd.length()));
                 }
             }
             return true;
         }
         
         case LV_KEY_ESC: {
             // Clear current command
             lv_ta_set_text(_cmdLine, TERM_PROMPT);
             lv_ta_set_cursor_pos(_cmdLine, strlen(TERM_PROMPT));
             _cmdHistoryPos = 0;
             return true;
         }
     }
     
     return false;
 }
 
 // Register a command handler
 void Terminal::registerCommand(const String& command, const String& description, 
                               std::function<void(const std::vector<String>&)> handler) {
     Command cmd;
     cmd.name = command;
     cmd.description = description;
     cmd.handler = handler;
     _commands.push_back(cmd);
 }
 
 // Process a command
 void Terminal::processCommand(const String& command) {
     // Parse command into arguments
     std::vector<String> args = parseCommand(command);
     
     if (args.empty()) {
         return; // Empty command, do nothing
     }
     
     // Find matching command
     String cmdName = args[0];
     bool found = false;
     
     for (const auto& cmd : _commands) {
         if (cmd.name == cmdName) {
             // Execute command handler
             cmd.handler(args);
             found = true;
             break;
         }
     }
     
     if (!found) {
         println("Command not found: " + cmdName, TERM_COLOR_ERROR);
     }
 }
 
 // Parse command into arguments
 std::vector<String> Terminal::parseCommand(const String& command) {
     std::vector<String> args;
     
     // Empty command
     if (command.length() == 0) {
         return args;
     }
     
     // Simple parsing by spaces (doesn't handle quotes yet)
     int start = 0;
     int end = 0;
     
     while (end < command.length()) {
         // Skip spaces
         while (start < command.length() && command[start] == ' ') {
             start++;
         }
         
         // Find end of argument
         end = start;
         while (end < command.length() && command[end] != ' ') {
             end++;
         }
         
         // Extract argument
         if (end > start) {
             args.push_back(command.substring(start, end));
         }
         
         start = end;
     }
     
     return args;
 }
 
 // Update terminal display
 void Terminal::updateDisplay() {
     String display;
     
     // Build display text from history
     for (const auto& line : _history) {
         display += line;
     }
     
     // Update text area
     lv_ta_set_text(_textArea, display.c_str());
     
     // Scroll to bottom
     scrollToBottom();
 }
 
 // Scroll to bottom of terminal
 void Terminal::scrollToBottom() {
     // Get total lines
     uint16_t totalLines = lv_ta_get_text_sel_en(_textArea);
     if (totalLines > 0) {
         // Scroll to last line
         lv_ta_set_cursor_pos(_textArea, strlen(lv_ta_get_text(_textArea)));
     }
 }
 
 // Initialize built-in commands
 void Terminal::initCommands() {
     // Help command
     registerCommand("help", "Display list of commands",
         [this](const std::vector<String>& args) {
             println("Available commands:", TERM_COLOR_SYSTEM);
             
             for (const auto& cmd : _commands) {
                 String helpText = "  " + cmd.name;
                 
                 // Add padding between name and description
                 int padding = 15 - cmd.name.length();
                 for (int i = 0; i < padding; i++) {
                     helpText += " ";
                 }
                 
                 helpText += cmd.description;
                 println(helpText, TERM_COLOR_NORMAL);
             }
         }
     );
     
     // Clear command
     registerCommand("clear", "Clear the terminal",
         [this](const std::vector<String>& args) {
             clear();
         }
     );
     
     // Echo command
     registerCommand("echo", "Display text",
         [this](const std::vector<String>& args) {
             String text;
             for (size_t i = 1; i < args.size(); i++) {
                 if (i > 1) text += " ";
                 text += args[i];
             }
             println(text, TERM_COLOR_NORMAL);
         }
     );
     
     // Date/time command
     registerCommand("date", "Display current date and time",
         [this](const std::vector<String>& args) {
             time_t now;
             struct tm timeinfo;
             char buf[64];
             
             time(&now);
             localtime_r(&now, &timeinfo);
             strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
             
             println(String("Current date and time: ") + buf, TERM_COLOR_NORMAL);
         }
     );
     
     // System info command
     registerCommand("sysinfo", "Display system information",
         [this](const std::vector<String>& args) {
             println("T-Deck System Information", TERM_COLOR_SYSTEM);
             println("---------------------------", TERM_COLOR_SYSTEM);
             println("Firmware: " + String(TDECK_FIRMWARE_NAME) + " v" + String(TDECK_FIRMWARE_VERSION), TERM_COLOR_NORMAL);
             println("ESP32-S3 Chip Info:", TERM_COLOR_NORMAL);
             println("  CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz", TERM_COLOR_NORMAL);
             println("  Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB", TERM_COLOR_NORMAL);
             println("  Heap Size: " + String(ESP.getHeapSize() / 1024) + " KB", TERM_COLOR_NORMAL);
             println("  PSRAM Size: " + String(ESP.getPsramSize() / 1024) + " KB", TERM_COLOR_NORMAL);
             println("  Flash Size: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB", TERM_COLOR_NORMAL);
             println("  SDK Version: " + String(ESP.getSdkVersion()), TERM_COLOR_NORMAL);
         }
     );
     
     // LS command (list directory)
     registerCommand("ls", "List directory contents",
         [this](const std::vector<String>& args) {
             String path = "/";
             if (args.size() > 1) {
                 path = args[1];
             }
             
             String dirContents;
             if (fsManager.listDir(path.c_str(), dirContents)) {
                 println("Directory: " + path, TERM_COLOR_SYSTEM);
                 println(dirContents, TERM_COLOR_NORMAL);
             } else {
                 println("Error listing directory: " + path, TERM_COLOR_ERROR);
             }
         }
     );
     
     // CAT command (show file contents)
     registerCommand("cat", "Display file contents",
         [this](const std::vector<String>& args) {
             if (args.size() < 2) {
                 println("Usage: cat <filename>", TERM_COLOR_ERROR);
                 return;
             }
             
             String path = args[1];
             const size_t bufSize = 4096;
             char* buffer = new char[bufSize];
             
             if (buffer) {
                 int bytesRead = fsManager.readFile(path.c_str(), buffer, bufSize - 1);
                 
                 if (bytesRead > 0) {
                     buffer[bytesRead] = '\0'; // Null terminate
                     println("File: " + path, TERM_COLOR_SYSTEM);
                     println("------------------", TERM_COLOR_SYSTEM);
                     println(buffer, TERM_COLOR_NORMAL);
                 } else {
                     println("Error reading file: " + path, TERM_COLOR_ERROR);
                 }
                 
                 delete[] buffer;
             } else {
                 println("Memory allocation error", TERM_COLOR_ERROR);
             }
         }
     );
     
     // MKDIR command (create directory)
     registerCommand("mkdir", "Create a directory",
         [this](const std::vector<String>& args) {
             if (args.size() < 2) {
                 println("Usage: mkdir <dirname>", TERM_COLOR_ERROR);
                 return;
             }
             
             String path = args[1];
             if (fsManager.createDir(path.c_str())) {
                 println("Directory created: " + path, TERM_COLOR_NORMAL);
             } else {
                 println("Error creating directory: " + path, TERM_COLOR_ERROR);
             }
         }
     );
     
     // RM command (remove file)
     registerCommand("rm", "Remove a file",
         [this](const std::vector<String>& args) {
             if (args.size() < 2) {
                 println("Usage: rm <filename>", TERM_COLOR_ERROR);
                 return;
             }
             
             String path = args[1];
             if (fsManager.deleteFile(path.c_str())) {
                 println("File deleted: " + path, TERM_COLOR_NORMAL);
             } else {
                 println("Error deleting file: " + path, TERM_COLOR_ERROR);
             }
         }
     );
     
     // RMDIR command (remove directory)
     registerCommand("rmdir", "Remove a directory",
         [this](const std::vector<String>& args) {
             if (args.size() < 2) {
                 println("Usage: rmdir <dirname>", TERM_COLOR_ERROR);
                 return;
             }
             
             String path = args[1];
             if (fsManager.removeDir(path.c_str())) {
                 println("Directory removed: " + path, TERM_COLOR_NORMAL);
             } else {
                 println("Error removing directory: " + path, TERM_COLOR_ERROR);
             }
         }
     );
     
     // TOUCH command (create empty file)
     registerCommand("touch", "Create an empty file",
         [this](const std::vector<String>& args) {
             if (args.size() < 2) {
                 println("Usage: touch <filename>", TERM_COLOR_ERROR);
                 return;
             }
             
             String path = args[1];
             if (fsManager.writeFile(path.c_str(), "")) {
                 println("File created: " + path, TERM_COLOR_NORMAL);
             } else {
                 println("Error creating file: " + path, TERM_COLOR_ERROR);
             }
         }
     );
     
     // WRITE command (write text to file)
     registerCommand("write", "Write text to a file",
         [this](const std::vector<String>& args) {
             if (args.size() < 3) {
                 println("Usage: write <filename> <text>", TERM_COLOR_ERROR);
                 return;
             }
             
             String path = args[1];
             String text;
             for (size_t i = 2; i < args.size(); i++) {
                 if (i > 2) text += " ";
                 text += args[i];
             }
             
             if (fsManager.writeFile(path.c_str(), text.c_str())) {
                 println("Text written to file: " + path, TERM_COLOR_NORMAL);
             } else {
                 println("Error writing to file: " + path, TERM_COLOR_ERROR);
             }
         }
     );
     
     // FREE command (show memory usage)
     registerCommand("free", "Display memory information",
         [this](const std::vector<String>& args) {
             println("Memory Information", TERM_COLOR_SYSTEM);
             println("------------------", TERM_COLOR_SYSTEM);
             println("Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB", TERM_COLOR_NORMAL);
             println("Heap Size: " + String(ESP.getHeapSize() / 1024) + " KB", TERM_COLOR_NORMAL);
             println("Minimum Free Heap: " + String(ESP.getMinFreeHeap() / 1024) + " KB", TERM_COLOR_NORMAL);
             println("Maximum Alloc Heap: " + String(ESP.getMaxAllocHeap() / 1024) + " KB", TERM_COLOR_NORMAL);
             
             // PSRAM information
             println("Free PSRAM: " + String(ESP.getFreePsram() / 1024) + " KB", TERM_COLOR_NORMAL);
             println("PSRAM Size: " + String(ESP.getPsramSize() / 1024) + " KB", TERM_COLOR_NORMAL);
             if (ESP.getPsramSize() > 0) {
                 println("Minimum Free PSRAM: " + String(ESP.getMinFreePsram() / 1024) + " KB", TERM_COLOR_NORMAL);
                 println("Maximum Alloc PSRAM: " + String(ESP.getMaxAllocPsram() / 1024) + " KB", TERM_COLOR_NORMAL);
             }
         }
     );
     
     // WIFI command (WiFi control)
     registerCommand("wifi", "WiFi control (scan, connect, status)",
         [this](const std::vector<String>& args) {
             if (args.size() < 2) {
                 println("Usage: wifi <scan|connect|status|disconnect>", TERM_COLOR_ERROR);
                 return;
             }
             
             String action = args[1];
             
             if (action == "scan") {
                 println("WiFi scanning not implemented yet", TERM_COLOR_ERROR);
             } else if (action == "connect") {
                 if (args.size() < 4) {
                     println("Usage: wifi connect <ssid> <password>", TERM_COLOR_ERROR);
                     return;
                 }
                 println("WiFi connect not implemented yet", TERM_COLOR_ERROR);
             } else if (action == "status") {
                 println("WiFi status not implemented yet", TERM_COLOR_ERROR);
             } else if (action == "disconnect") {
                 println("WiFi disconnect not implemented yet", TERM_COLOR_ERROR);
             } else {
                 println("Unknown WiFi command: " + action, TERM_COLOR_ERROR);
             }
         }
     );
     
     // REBOOT command
     registerCommand("reboot", "Reboot the device",
         [this](const std::vector<String>& args) {
             println("Rebooting...", TERM_COLOR_SYSTEM);
             delay(1000);
             ESP.restart();
         }
     );
 }