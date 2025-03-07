/**
 * @file keyboard.cpp
 * @brief Keyboard driver implementation for T-Deck UI Firmware
 */

 #include "keyboard.h"

 // Global keyboard instance
 TDeckKeyboard keyboard;
 
 TDeckKeyboard::TDeckKeyboard() : 
     _callback_count(0),
     _last_interrupt_time(0),
     _interrupt_triggered(false)
 {
     // Initialize callbacks array
     for (uint8_t i = 0; i < MAX_CALLBACKS; i++) {
         _callbacks[i] = nullptr;
     }
     
     // Initialize key states
     memset(_key_states, 0, sizeof(_key_states));
 }
 
 bool TDeckKeyboard::init() {
     TDECK_LOG_I("Initializing keyboard");
     
     // Initialize I2C for keyboard communication
     _wire.begin(_sda_pin, _scl_pin);
     
     // Configure interrupt pin
     pinMode(_int_pin, INPUT_PULLUP);
     
     // Test communication with keyboard controller
     _wire.beginTransmission(_address);
     if (_wire.endTransmission() != 0) {
         TDECK_LOG_E("Failed to communicate with keyboard controller");
         return false;
     }
     
     // Attach interrupt handler
     attachInterruptArg(_int_pin, _keyboard_isr, this, FALLING);
     
     TDECK_LOG_I("Keyboard initialized successfully");
     return true;
 }
 
 void IRAM_ATTR TDeckKeyboard::_keyboard_isr(void* arg) {
     TDeckKeyboard* keyboard = static_cast<TDeckKeyboard*>(arg);
     
     // Set interrupt flag
     keyboard->_interrupt_triggered = true;
 }
 
 bool TDeckKeyboard::_read_key_states() {
     // Request 8 bytes of data from the keyboard controller
     _wire.requestFrom(_address, (uint8_t)8);
     
     if (_wire.available() != 8) {
         TDECK_LOG_E("Failed to read keyboard data, expected 8 bytes");
         return false;
     }
     
     // Read the key state bytes
     for (uint8_t i = 0; i < 8; i++) {
         _key_states[i] = _wire.read();
     }
     
     return true;
 }
 
 void TDeckKeyboard::_process_key_changes(const uint8_t* old_states) {
     // Check each bit to see which keys changed state
     for (uint8_t byte_idx = 0; byte_idx < 8; byte_idx++) {
         uint8_t changes = old_states[byte_idx] ^ _key_states[byte_idx];
         
         if (changes == 0) {
             continue; // No changes in this byte
         }
         
         // Check each bit in the changed byte
         for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++) {
             if (changes & (1 << bit_idx)) {
                 // Calculate key position
                 uint8_t key_pos = byte_idx * 8 + bit_idx;
                 
                 // Determine if key is now pressed or released
                 bool pressed = (_key_states[byte_idx] & (1 << bit_idx)) == 0; // Key is active low
                 
                 // Map key position to actual key code
                 uint8_t key_code = _map_key_code(byte_idx, bit_idx);
                 
                 // Notify all registered callbacks
                 for (uint8_t i = 0; i < _callback_count; i++) {
                     if (_callbacks[i]) {
                         _callbacks[i](key_code, pressed);
                     }
                 }
             }
         }
     }
 }
 
 void TDeckKeyboard::update() {
     if (!_interrupt_triggered) {
         return; // No key state change
     }
     
     // Reset interrupt flag
     _interrupt_triggered = false;
     
     // Debounce
     unsigned long current_time = millis();
     if (current_time - _last_interrupt_time < 20) { // 20ms debounce time
         return;
     }
     _last_interrupt_time = current_time;
     
     // Save old key states for change detection
     uint8_t old_states[8];
     memcpy(old_states, _key_states, sizeof(_key_states));
     
     // Read current key states
     if (_read_key_states()) {
         // Process key changes
         _process_key_changes(old_states);
     }
 }
 
 bool TDeckKeyboard::isKeyPressed(uint8_t key) const {
     for (uint8_t byte_idx = 0; byte_idx < 8; byte_idx++) {
         for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++) {
             uint8_t key_pos = byte_idx * 8 + bit_idx;
             uint8_t key_code = _map_key_code(byte_idx, bit_idx);
             
             if (key_code == key) {
                 return (_key_states[byte_idx] & (1 << bit_idx)) == 0; // Key is active low
             }
         }
     }
     
     return false;
 }
 
 bool TDeckKeyboard::registerCallback(KeyEventCallback callback) {
     if (_callback_count >= MAX_CALLBACKS) {
         TDECK_LOG_E("Maximum number of keyboard callbacks reached");
         return false;
     }
     
     _callbacks[_callback_count++] = callback;
     return true;
 }
 
 bool TDeckKeyboard::unregisterCallback(KeyEventCallback callback) {
     for (uint8_t i = 0; i < _callback_count; i++) {
         if (_callbacks[i] == callback) {
             // Move all callbacks after this one down by one
             for (uint8_t j = i; j < _callback_count - 1; j++) {
                 _callbacks[j] = _callbacks[j + 1];
             }
             
             // Clear the last callback slot
             _callbacks[--_callback_count] = nullptr;
             return true;
         }
     }
     
     return false;
 }
 
 // Static callback function for LVGL keyboard input reading
 void TDeckKeyboard::read_cb(lv_indev_drv_t* indev_drv, lv_indev_data_t* data) {
     // Default to no key pressed
     data->key = 0;
     data->state = LV_INDEV_STATE_RELEASED;
     
     // Check for arrow key presses for navigation
     if (keyboard.isKeyPressed(TDECK_KEY_UP)) {
         data->key = LV_KEY_UP;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_DOWN)) {
         data->key = LV_KEY_DOWN;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_LEFT)) {
         data->key = LV_KEY_LEFT;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_RIGHT)) {
         data->key = LV_KEY_RIGHT;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_ENTER)) {
         data->key = LV_KEY_ENTER;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_ESC)) {
         data->key = LV_KEY_ESC;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_BACKSPACE)) {
         data->key = LV_KEY_BACKSPACE;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_HOME)) {
         data->key = LV_KEY_HOME;
         data->state = LV_INDEV_STATE_PRESSED;
     } else if (keyboard.isKeyPressed(TDECK_KEY_END)) {
         data->key = LV_KEY_END;
         data->state = LV_INDEV_STATE_PRESSED;
     }
 }
 
 uint8_t TDeckKeyboard::_map_key_code(uint8_t row, uint8_t col) const {
     // This mapping is specific to the T-Deck keyboard layout
     // It maps the physical key position to the corresponding ASCII or special key code
     
     // T-Deck keyboard matrix layout (example - adjust based on actual hardware)
     static const uint8_t KEY_MAP[8][8] = {
         // Rows and columns based on the T-Deck keyboard matrix
         // This is a placeholder and should be updated with actual key mappings
         {'1', '2', '3', '4', '5', '6', '7', '8'},
         {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i'},
         {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k'},
         {'z', 'x', 'c', 'v', 'b', 'n', 'm', ','},
         {TDECK_KEY_SHIFT, TDECK_KEY_CTRL, TDECK_KEY_ALT, ' ', '.', '/', TDECK_KEY_ENTER, TDECK_KEY_BACKSPACE},
         {TDECK_KEY_ESC, TDECK_KEY_TAB, '9', '0', '-', '=', '[', ']'},
         {'o', 'p', 'l', ';', '\'', '\\', TDECK_KEY_UP, TDECK_KEY_HOME},
         {TDECK_KEY_CAPS, TDECK_KEY_FN, TDECK_KEY_LEFT, TDECK_KEY_DOWN, TDECK_KEY_RIGHT, TDECK_KEY_END, 0, 0}
     };
     
     if (row < 8 && col < 8) {
         return KEY_MAP[row][col];
     }
     
     return TDECK_KEY_UNKNOWN;
 }