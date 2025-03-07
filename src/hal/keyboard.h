/**
 * @file keyboard.h
 * @brief Keyboard driver interface for T-Deck UI Firmware
 * 
 * This file contains the declarations for the T-Deck QWERTY keyboard
 * driver functionality, including key detection and event handling.
 */

 #ifndef TDECK_KEYBOARD_H
 #define TDECK_KEYBOARD_H
 
 #include <Arduino.h>
 #include <Wire.h>
 #include <lvgl.h>
 #include "../config.h"
 
 // Key event callback type
 typedef void (*KeyEventCallback)(uint8_t key, bool pressed);
 
 /**
  * @brief Key codes for special keys
  */
 enum TDeckKeyCode {
     TDECK_KEY_UNKNOWN = 0,
     TDECK_KEY_ESC = 27,      // Escape key
     TDECK_KEY_BACKSPACE = 8, // Backspace key
     TDECK_KEY_TAB = 9,       // Tab key
     TDECK_KEY_ENTER = 13,    // Enter key
     TDECK_KEY_SHIFT = 128,   // Shift key
     TDECK_KEY_CTRL = 129,    // Control key
     TDECK_KEY_ALT = 130,     // Alt key
     TDECK_KEY_CAPS = 131,    // Caps Lock key
     TDECK_KEY_FN = 132,      // Function key
     TDECK_KEY_UP = 133,      // Arrow up key
     TDECK_KEY_DOWN = 134,    // Arrow down key
     TDECK_KEY_LEFT = 135,    // Arrow left key
     TDECK_KEY_RIGHT = 136,   // Arrow right key
     TDECK_KEY_HOME = 137,    // Home key
     TDECK_KEY_END = 138      // End key
 };
 
 /**
  * @brief Keyboard manager class for T-Deck
  * 
  * Handles initialization and management of the T-Deck's physical QWERTY keyboard,
  * including key detection, event generation, and LVGL integration.
  */
 class TDeckKeyboard {
 public:
     /**
      * @brief Construct a new TDeckKeyboard object
      */
     TDeckKeyboard();
 
     /**
      * @brief Initialize the keyboard controller
      * 
      * @return true if initialization was successful
      * @return false if initialization failed
      */
     bool init();
 
     /**
      * @brief Update the keyboard state (should be called regularly from the main loop)
      */
     void update();
 
     /**
      * @brief Check if a key is currently pressed
      * 
      * @param key The key code to check
      * @return true if the key is pressed
      * @return false if the key is not pressed
      */
     bool isKeyPressed(uint8_t key) const;
 
     /**
      * @brief Register a callback for key events
      * 
      * @param callback The callback function to register
      * @return true if the callback was registered successfully
      * @return false if the callback could not be registered
      */
     bool registerCallback(KeyEventCallback callback);
 
     /**
      * @brief Unregister a callback for key events
      * 
      * @param callback The callback function to unregister
      * @return true if the callback was unregistered successfully
      * @return false if the callback could not be unregistered
      */
     bool unregisterCallback(KeyEventCallback callback);
 
     /**
      * @brief Static LVGL keyboard read callback
      * 
      * @param indev_drv Input device driver
      * @param data Input data to be filled
      */
     static void read_cb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data);
 
 private:
     // I2C address of the keyboard controller
     const uint8_t _address = TDECK_KEYBOARD_ADDR;
     
     // I2C pins
     const uint8_t _sda_pin = TDECK_KEYBOARD_SDA;
     const uint8_t _scl_pin = TDECK_KEYBOARD_SCL;
     
     // Interrupt pin
     const uint8_t _int_pin = TDECK_KEYBOARD_INT;
     
     // I2C instance for keyboard
     TwoWire _wire = TwoWire(0);
     
     // Current key states
     uint8_t _key_states[8]; // Bitfield for 64 keys
     
     // Key event callbacks
     static const uint8_t MAX_CALLBACKS = 5;
     KeyEventCallback _callbacks[MAX_CALLBACKS];
     uint8_t _callback_count;
     
     // Last interrupt time for debouncing
     unsigned long _last_interrupt_time;
     
     // Flag indicating if an interrupt was triggered
     volatile bool _interrupt_triggered;
     
     // Interrupt service routine
     static void IRAM_ATTR _keyboard_isr(void* arg);
     
     // Read key states from the keyboard controller
     bool _read_key_states();
     
     // Process key changes
     void _process_key_changes(const uint8_t* old_states);
     
     // Get key status at specific position
     bool _get_key_state(uint8_t key) const;
     
     // Convert raw keyboard code to character or special key code
     uint8_t _map_key_code(uint8_t row, uint8_t col) const;
 };
 
 // Global keyboard instance
 extern TDeckKeyboard keyboard;
 
 #endif // TDECK_KEYBOARD_H