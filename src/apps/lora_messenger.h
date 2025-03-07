/**
 * @file lora_messenger.h
 * @brief LoRa Messaging Application for T-Deck UI
 * 
 * This application provides a messaging interface using the LoRa module
 * for long range communication between T-Deck devices.
 */

 #ifndef LORA_MESSENGER_H
 #define LORA_MESSENGER_H
 
 #include <Arduino.h>
 #include <lvgl.h>
 #include <vector>
 #include <string>
 #include "../config.h"
 #include "../comms/lora.h"
 #include "../system/fs_manager.h"
 
 // Message structure for LoRa communication
 struct LoRaMessage {
     String sender;       // Sender identifier (name or ID)
     String content;      // Message content
     uint32_t timestamp;  // Message timestamp
     bool incoming;       // True if received message, false if sent by this device
 };
 
 class LoRaMessenger {
 public:
     LoRaMessenger();
     ~LoRaMessenger();
 
     /**
      * Initialize the LoRa Messenger application
      * @param parent Parent LVGL container
      * @return True if initialized successfully
      */
     bool init(lv_obj_t* parent);
 
     /**
      * Start the LoRa Messenger application
      */
     void start();
 
     /**
      * Stop the LoRa Messenger application
      */
     void stop();
 
     /**
      * Handle periodic updates
      * Should be called regularly from the main loop
      */
     void update();
 
     /**
      * Set the user identifier for this device
      * @param id User identifier (name)
      */
     void setUserIdentifier(const String& id);
 
     /**
      * Send a message via LoRa
      * @param message Message content
      * @return True if successfully queued for sending
      */
     bool sendMessage(const String& message);
 
     /**
      * Callback for when new LoRa message is received
      * @param data Received data
      * @param len Data length
      */
     void onMessageReceived(uint8_t* data, size_t len);
 
     /**
      * Set the active state of the app
      * @param active True if app is active/visible
      */
     void setActive(bool active);
 
     /**
      * Check if the app is currently active
      * @return True if active
      */
     bool isActive() const;
 
 private:
     // LVGL UI components
     lv_obj_t* _parent;           // Parent container
     lv_obj_t* _container;        // Main container
     lv_obj_t* _messageList;      // Container for message bubbles
     lv_obj_t* _inputField;       // Text input field
     lv_obj_t* _sendBtn;          // Send button
     lv_obj_t* _settingsBtn;      // Settings button
     lv_obj_t* _statusLabel;      // Status label
     
     // Settings dialog components
     lv_obj_t* _settingsDialog;   // Settings dialog
     lv_obj_t* _nameInput;        // User name input
     lv_obj_t* _frequencyInput;   // Frequency input
     lv_obj_t* _spreadingInput;   // Spreading factor input
     lv_obj_t* _bandwidthInput;   // Bandwidth input
 
     // Application state
     bool _active;                // Is the application currently active
     String _userIdentifier;      // User name/identifier
     std::vector<LoRaMessage> _messages;  // Message history
     LoRaManager* _loraManager;   // Reference to LoRa manager
     FSManager* _fsManager;       // Reference to filesystem manager
     bool _messageReceived;       // Flag for new message received
 
     // Settings path
     static constexpr const char* SETTINGS_PATH = "/config/lora_messenger.json";
 
     /**
      * Create the UI elements
      */
     void createUI();
 
     /**
      * Add a message to the UI
      * @param message Message to add
      */
     void addMessageToUI(const LoRaMessage& message);
 
     /**
      * Clear all messages from the UI
      */
     void clearMessages();
 
     /**
      * Load messages from storage
      */
     void loadMessages();
 
     /**
      * Save messages to storage
      */
     void saveMessages();
 
     /**
      * Load settings from storage
      */
     void loadSettings();
 
     /**
      * Save settings to storage
      */
     void saveSettings();
 
     /**
      * Show settings dialog
      */
     void showSettingsDialog();
 
     /**
      * Apply settings from dialog
      */
     void applySettings();
 
     /**
      * Update connection status display
      */
     void updateStatus();
 
     // LVGL callbacks
     static void onSendBtnClick(lv_obj_t* btn, lv_event_t event);
     static void onSettingsBtnClick(lv_obj_t* btn, lv_event_t event);
     static void onInputSubmit(lv_obj_t* ta, lv_event_t event);
     static void onSettingsApply(lv_obj_t* btn, lv_event_t event);
     static void onSettingsCancel(lv_obj_t* btn, lv_event_t event);
 };
 
 // Global instance (extern declaration)
 extern LoRaMessenger loraMessenger;
 
 #endif // LORA_MESSENGER_H