/**
 * @file terminal.h
 * @brief Terminal application for T-Deck UI Firmware
 * 
 * This class implements a simple terminal application that allows users
 * to enter commands and see their output. It supports basic command history,
 * scrolling, and common terminal features.
 */

 #ifndef TDECK_TERMINAL_H
 #define TDECK_TERMINAL_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include <vector>
 #include <deque>
 #include <functional>
 #include "../../config.h"
 #include "../../system/fs_manager.h"
 
 extern FSManager fsManager;
 
 // Terminal colors
 #define TERM_COLOR_NORMAL    0xFFFFFF // White
 #define TERM_COLOR_COMMAND   0x00FF00 // Green
 #define TERM_COLOR_ERROR     0xFF0000 // Red
 #define TERM_COLOR_PROMPT    0x00FFFF // Cyan
 #define TERM_COLOR_SYSTEM    0xFFFF00 // Yellow
 
 // Terminal configuration
 #define TERM_MAX_HISTORY        100   // Maximum lines in terminal history
 #define TERM_MAX_CMD_HISTORY    20    // Maximum commands in history
 #define TERM_MAX_COMMAND_LENGTH 128   // Maximum command length
 #define TERM_PROMPT             "> "  // Command prompt
 
 /**
  * @brief Terminal application class
  */
 class Terminal {
 public:
     /**
      * @brief Constructor
      */
     Terminal();
     
     /**
      * @brief Destructor
      */
     ~Terminal();
     
     /**
      * @brief Initialize the terminal application
      * 
      * @param parent LVGL parent container
      * @return true if initialization succeeds, false otherwise
      */
     bool init(lv_obj_t* parent);
     
     /**
      * @brief Start the terminal application
      */
     void start();
     
     /**
      * @brief Stop the terminal application
      */
     void stop();
     
     /**
      * @brief Check if terminal is running
      * 
      * @return true if terminal is active, false otherwise
      */
     bool isRunning() const;
     
     /**
      * @brief Add text to the terminal
      * 
      * @param text Text to add
      * @param color Text color (use TERM_COLOR_* defines)
      */
     void print(const String& text, uint32_t color = TERM_COLOR_NORMAL);
     
     /**
      * @brief Add text line to the terminal (with newline)
      * 
      * @param text Text to add
      * @param color Text color (use TERM_COLOR_* defines)
      */
     void println(const String& text, uint32_t color = TERM_COLOR_NORMAL);
     
     /**
      * @brief Clear the terminal
      */
     void clear();
     
     /**
      * @brief Handle keyboard events
      * 
      * @param key Key code
      * @return true if event was handled, false otherwise
      */
     bool handleKeyEvent(uint32_t key);
     
     /**
      * @brief Register a command handler
      * 
      * @param command Command name
      * @param description Command description
      * @param handler Function to handle the command
      */
     void registerCommand(const String& command, const String& description, 
                         std::function<void(const std::vector<String>&)> handler);
     
 private:
     // UI Elements
     lv_obj_t* _parent;            // Parent container
     lv_obj_t* _container;         // Terminal container
     lv_obj_t* _textArea;          // Text display area
     lv_obj_t* _cmdLine;           // Command input line
     lv_obj_t* _scrollBar;         // Scroll bar
     
     // State
     bool _running;                // Is terminal running
     std::deque<String> _history;  // Terminal output history
     std::deque<String> _cmdHistory; // Command history
     int _cmdHistoryPos;           // Position in command history
     
     // Command processing
     struct Command {
         String name;
         String description;
         std::function<void(const std::vector<String>&)> handler;
     };
     
     std::vector<Command> _commands; // Registered commands
     
     /**
      * @brief Process a command
      * 
      * @param command Command to process
      */
     void processCommand(const String& command);
     
     /**
      * @brief Parse command into arguments
      * 
      * @param command Command string
      * @return Vector of arguments
      */
     std::vector<String> parseCommand(const String& command);
     
     /**
      * @brief Update terminal display
      */
     void updateDisplay();
     
     /**
      * @brief Scroll to bottom of terminal
      */
     void scrollToBottom();
     
     /**
      * @brief Initialize built-in commands
      */
     void initCommands();
 };
 
 #endif // TDECK_TERMINAL_H