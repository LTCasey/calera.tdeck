/**
 * @file lora_messenger.cpp
 * @brief Implementation of LoRa Messaging Application for T-Deck UI
 */

 #include "lora_messenger.h"
 #include <ArduinoJson.h>
 #include <time.h>
 
 // Define the global instance
 LoRaMessenger loraMessenger;
 
 // Message packet header to identify LoRa messages from this app
 const uint8_t MSG_HEADER[] = {'T', 'D', 'M', 'S', 'G'};
 const size_t MSG_HEADER_LEN = 5;
 
 // Max message length
 const size_t MAX_MESSAGE_LENGTH = 230; // Keep some space for headers
 
 // Message format: [MSG_HEADER][sender_len(1)][sender(var)][content_len(2)][content(var)][timestamp(4)]
 
 LoRaMessenger::LoRaMessenger() : 
     _parent(nullptr),
     _container(nullptr),
     _messageList(nullptr),
     _inputField(nullptr),
     _sendBtn(nullptr),
     _settingsBtn(nullptr),
     _statusLabel(nullptr),
     _settingsDialog(nullptr),
     _nameInput(nullptr),
     _frequencyInput(nullptr),
     _spreadingInput(nullptr),
     _bandwidthInput(nullptr),
     _active(false),
     _userIdentifier("User"),
     _messageReceived(false)
 {
     // Get LoRa manager instance
     _loraManager = &loraManager;
     _fsManager = &fsManager;
 }
 
 LoRaMessenger::~LoRaMessenger() {
     // Save messages and settings on destruction
     if (_active) {
         saveMessages();
         saveSettings();
     }
 }
 
 bool LoRaMessenger::init(lv_obj_t* parent) {
     TDECK_LOG_I("Initializing LoRa Messenger");
     
     // Store parent container
     _parent = parent;
     
     // Load settings
     loadSettings();
     
     // Create UI components
     createUI();
     
     // Load previous messages
     loadMessages();
     
     // Register receive callback with LoRa Manager
     // Note: This would be properly handled in a real implementation
     // Here we would typically set up a way for the LoRaManager to call
     // our onMessageReceived method when data arrives.
     
     TDECK_LOG_I("LoRa Messenger initialized");
     return true;
 }
 
 void LoRaMessenger::start() {
     TDECK_LOG_I("Starting LoRa Messenger");
     
     // Show the UI
     if (_container) {
         lv_obj_set_hidden(_container, false);
         
         // Update status
         updateStatus();
         
         // Set as active
         _active = true;
     }
 }
 
 void LoRaMessenger::stop() {
     TDECK_LOG_I("Stopping LoRa Messenger");
     
     // Hide the UI
     if (_container) {
         lv_obj_set_hidden(_container, true);
     }
     
     // Save messages and settings
     saveMessages();
     saveSettings();
     
     // Set as inactive
     _active = false;
 }
 
 void LoRaMessenger::update() {
     // Only process if active
     if (!_active) return;
     
     // Check if a new message was received
     if (_messageReceived) {
         // Reset flag
         _messageReceived = false;
         
         // Scroll to the bottom of the message list
         lv_page_focus(_messageList, NULL, LV_ANIM_ON);
     }
     
     // Update status display
     static uint32_t lastStatusUpdate = 0;
     uint32_t now = millis();
     if (now - lastStatusUpdate > 2000) { // Update every 2 seconds
         updateStatus();
         lastStatusUpdate = now;
     }
 }
 
 void LoRaMessenger::setUserIdentifier(const String& id) {
     _userIdentifier = id;
     saveSettings();
 }
 
 bool LoRaMessenger::sendMessage(const String& message) {
     if (message.length() == 0 || message.length() > MAX_MESSAGE_LENGTH) {
         TDECK_LOG_W("Invalid message length: %d", message.length());
         return false;
     }
     
     // Create message structure for UI
     LoRaMessage msg;
     msg.sender = _userIdentifier;
     msg.content = message;
     msg.timestamp = millis(); // In a real app, we'd use a proper timestamp
     msg.incoming = false;
     
     // Add to UI
     addMessageToUI(msg);
     
     // Add to message history
     _messages.push_back(msg);
     
     // Save messages
     saveMessages();
     
     // Create LoRa packet
     uint8_t packet[256]; // Max packet size
     size_t packetLen = 0;
     
     // Add header
     memcpy(packet, MSG_HEADER, MSG_HEADER_LEN);
     packetLen += MSG_HEADER_LEN;
     
     // Add sender length (1 byte)
     packet[packetLen++] = _userIdentifier.length();
     
     // Add sender
     memcpy(packet + packetLen, _userIdentifier.c_str(), _userIdentifier.length());
     packetLen += _userIdentifier.length();
     
     // Add message length (2 bytes)
     packet[packetLen++] = message.length() & 0xFF;
     packet[packetLen++] = (message.length() >> 8) & 0xFF;
     
     // Add message content
     memcpy(packet + packetLen, message.c_str(), message.length());
     packetLen += message.length();
     
     // Add timestamp (4 bytes)
     uint32_t timestamp = msg.timestamp;
     packet[packetLen++] = timestamp & 0xFF;
     packet[packetLen++] = (timestamp >> 8) & 0xFF;
     packet[packetLen++] = (timestamp >> 16) & 0xFF;
     packet[packetLen++] = (timestamp >> 24) & 0xFF;
     
     // Send through LoRa
     TDECK_LOG_I("Sending LoRa message, len: %d", packetLen);
     return _loraManager->sendPacket(packet, packetLen);
 }
 
 void LoRaMessenger::onMessageReceived(uint8_t* data, size_t len) {
     // Check if this is a message for our app (check header)
     if (len < MSG_HEADER_LEN || memcmp(data, MSG_HEADER, MSG_HEADER_LEN) != 0) {
         // Not for us
         return;
     }
     
     // Parse message
     size_t pos = MSG_HEADER_LEN;
     
     // Get sender length
     if (pos >= len) return;
     uint8_t senderLen = data[pos++];
     
     // Get sender
     if (pos + senderLen > len) return;
     String sender((char*)(data + pos), senderLen);
     pos += senderLen;
     
     // Get message length
     if (pos + 2 > len) return;
     uint16_t msgLen = data[pos] | (data[pos + 1] << 8);
     pos += 2;
     
     // Get message content
     if (pos + msgLen > len) return;
     String content((char*)(data + pos), msgLen);
     pos += msgLen;
     
     // Get timestamp
     if (pos + 4 > len) return;
     uint32_t timestamp = data[pos] | 
                          (data[pos + 1] << 8) | 
                          (data[pos + 2] << 16) | 
                          (data[pos + 3] << 24);
     
     // Create message structure
     LoRaMessage msg;
     msg.sender = sender;
     msg.content = content;
     msg.timestamp = timestamp;
     msg.incoming = true;
     
     // Add to UI
     addMessageToUI(msg);
     
     // Add to message history
     _messages.push_back(msg);
     
     // Save messages
     saveMessages();
     
     // Set flag for new message
     _messageReceived = true;
     
     TDECK_LOG_I("Received message from %s: %s", sender.c_str(), content.c_str());
 }
 
 void LoRaMessenger::setActive(bool active) {
     _active = active;
     
     if (active) {
         updateStatus();
     }
 }
 
 bool LoRaMessenger::isActive() const {
     return _active;
 }
 
 // Private methods
 
 void LoRaMessenger::createUI() {
     // Create main container
     _container = lv_cont_create(_parent, NULL);
     lv_obj_set_size(_container, TDECK_DISPLAY_WIDTH, TDECK_DISPLAY_HEIGHT - TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_pos(_container, 0, TDECK_STATUS_BAR_HEIGHT);
     lv_obj_set_hidden(_container, true); // Hidden by default
     
     // Create status label
     _statusLabel = lv_label_create(_container, NULL);
     lv_label_set_long_mode(_statusLabel, LV_LABEL_LONG_SROLL_CIRC);
     lv_obj_set_width(_statusLabel, TDECK_DISPLAY_WIDTH - 80);
     lv_obj_align(_statusLabel, _container, LV_ALIGN_IN_TOP_LEFT, 5, 5);
     lv_label_set_text(_statusLabel, "LoRa: Not Connected");
     
     // Create settings button
     _settingsBtn = lv_btn_create(_container, NULL);
     lv_obj_set_size(_settingsBtn, 70, 30);
     lv_obj_align(_settingsBtn, _container, LV_ALIGN_IN_TOP_RIGHT, -5, 5);
     lv_obj_set_event_cb(_settingsBtn, onSettingsBtnClick);
     
     lv_obj_t* settingsBtnLabel = lv_label_create(_settingsBtn, NULL);
     lv_label_set_text(settingsBtnLabel, "Settings");
     
     // Create message list (scrollable page)
     _messageList = lv_page_create(_container, NULL);
     lv_obj_set_size(_messageList, TDECK_DISPLAY_WIDTH - 10, TDECK_DISPLAY_HEIGHT - TDECK_STATUS_BAR_HEIGHT - 80);
     lv_obj_align(_messageList, _container, LV_ALIGN_IN_TOP_MID, 0, 40);
     lv_page_set_scrl_width(_messageList, TDECK_DISPLAY_WIDTH - 30);
     lv_page_set_scrollable_fit(_messageList, LV_FIT_TIGHT);
     
     // Create input field
     _inputField = lv_ta_create(_container, NULL);
     lv_obj_set_size(_inputField, TDECK_DISPLAY_WIDTH - 90, 40);
     lv_obj_align(_inputField, _container, LV_ALIGN_IN_BOTTOM_LEFT, 5, -5);
     lv_ta_set_placeholder_text(_inputField, "Type a message...");
     lv_ta_set_one_line(_inputField, true);
     lv_obj_set_event_cb(_inputField, onInputSubmit);
     
     // Create send button
     _sendBtn = lv_btn_create(_container, NULL);
     lv_obj_set_size(_sendBtn, 70, 40);
     lv_obj_align(_sendBtn, _container, LV_ALIGN_IN_BOTTOM_RIGHT, -5, -5);
     lv_obj_set_event_cb(_sendBtn, onSendBtnClick);
     
     lv_obj_t* sendBtnLabel = lv_label_create(_sendBtn, NULL);
     lv_label_set_text(sendBtnLabel, "Send");
     
     // Set user data reference for callbacks
     lv_obj_set_user_data(_container, this);
     lv_obj_set_user_data(_sendBtn, this);
     lv_obj_set_user_data(_settingsBtn, this);
     lv_obj_set_user_data(_inputField, this);
 }
 
 void LoRaMessenger::addMessageToUI(const LoRaMessage& message) {
     // Message bubble (container)
     lv_obj_t* bubble = lv_cont_create(_messageList, NULL);
     
     // Set style based on incoming or outgoing message
     static lv_style_t style_bubble_in;
     static lv_style_t style_bubble_out;
     static bool styles_initialized = false;
     
     if (!styles_initialized) {
         lv_style_copy(&style_bubble_in, &lv_style_pretty);
         style_bubble_in.body.radius = TDECK_UI_DEFAULT_CORNER_RADIUS;
         style_bubble_in.body.main_color = lv_color_hex(0xDDDDDD);
         style_bubble_in.body.grad_color = lv_color_hex(0xDDDDDD);
         
         lv_style_copy(&style_bubble_out, &lv_style_pretty);
         style_bubble_out.body.radius = TDECK_UI_DEFAULT_CORNER_RADIUS;
         style_bubble_out.body.main_color = lv_color_hex(0x90CAF9);
         style_bubble_out.body.grad_color = lv_color_hex(0x90CAF9);
         
         styles_initialized = true;
     }
     
     // Apply style based on incoming or outgoing message
     lv_cont_set_style(bubble, LV_CONT_STYLE_MAIN, message.incoming ? &style_bubble_in : &style_bubble_out);
     
     // Set width to fit content
     lv_obj_set_width(bubble, TDECK_DISPLAY_WIDTH - 60);
     
     // Create sender label
     lv_obj_t* senderLabel = lv_label_create(bubble, NULL);
     lv_label_set_text(senderLabel, message.sender.c_str());
     
     // Create content label
     lv_obj_t* contentLabel = lv_label_create(bubble, NULL);
     lv_label_set_long_mode(contentLabel, LV_LABEL_LONG_BREAK);
     lv_obj_set_width(contentLabel, TDECK_DISPLAY_WIDTH - 80);
     lv_label_set_text(contentLabel, message.content.c_str());
     
     // Create time label
     lv_obj_t* timeLabel = lv_label_create(bubble, NULL);
     
     // Convert timestamp to time string
     char timeStr[20];
     // In a real app, we'd format properly using time APIs
     sprintf(timeStr, "%02d:%02d", (message.timestamp / 60000) % 60, (message.timestamp / 1000) % 60);
     lv_label_set_text(timeLabel, timeStr);
     
     // Position elements
     lv_obj_align(senderLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 5, 5);
     lv_obj_align(contentLabel, senderLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
     lv_obj_align(timeLabel, contentLabel, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);
     
     // Adjust bubble size to fit content
     lv_obj_set_height(bubble, lv_obj_get_height(senderLabel) + 
                              lv_obj_get_height(contentLabel) + 
                              lv_obj_get_height(timeLabel) + 20);
     
     // Position bubble on left or right side based on incoming/outgoing
     if (message.incoming) {
         lv_obj_align(bubble, NULL, LV_ALIGN_IN_TOP_LEFT, 5, -1);
     } else {
         lv_obj_align(bubble, NULL, LV_ALIGN_IN_TOP_RIGHT, -5, -1);
     }
     
     // Move next message position down
     lv_page_glue_obj(bubble, true);
     
     // Scroll to show the new message
     lv_page_focus(_messageList, bubble, LV_ANIM_ON);
 }
 
 void LoRaMessenger::clearMessages() {
     // Remove all messages from UI
     lv_obj_t* page_scrl = lv_page_get_scrl(_messageList);
     lv_obj_clean(page_scrl);
     
     // Clear message history
     _messages.clear();
 }
 
 void LoRaMessenger::loadMessages() {
     // Load messages from storage
     String messagesPath = "/messages.json";
     
     // Check if file exists
     if (!_fsManager->fileExists(messagesPath)) {
         TDECK_LOG_I("No saved messages found");
         return;
     }
     
     // Load JSON file
     DynamicJsonDocument doc(16384); // Adjust size based on expected message volume
     
     if (!_fsManager->loadJsonFromFile(messagesPath, doc)) {
         TDECK_LOG_E("Failed to load messages from storage");
         return;
     }
     
     // Clear existing messages
     _messages.clear();
     
     // Parse JSON and add messages
     JsonArray messagesArray = doc["messages"].as<JsonArray>();
     
     for (JsonObject msgObj : messagesArray) {
         LoRaMessage msg;
         msg.sender = msgObj["sender"].as<String>();
         msg.content = msgObj["content"].as<String>();
         msg.timestamp = msgObj["timestamp"].as<uint32_t>();
         msg.incoming = msgObj["incoming"].as<bool>();
         
         // Add to message history
         _messages.push_back(msg);
         
         // Add to UI
         addMessageToUI(msg);
     }
     
     TDECK_LOG_I("Loaded %d messages from storage", _messages.size());
 }
 
 void LoRaMessenger::saveMessages() {
     // Save messages to storage
     String messagesPath = "/messages.json";
     
     // Create JSON document
     DynamicJsonDocument doc(16384); // Adjust size based on expected message volume
     
     // Create messages array
     JsonArray messagesArray = doc.createNestedArray("messages");
     
     // Add messages to JSON
     for (const LoRaMessage& msg : _messages) {
         JsonObject msgObj = messagesArray.createNestedObject();
         msgObj["sender"] = msg.sender;
         msgObj["content"] = msg.content;
         msgObj["timestamp"] = msg.timestamp;
         msgObj["incoming"] = msg.incoming;
     }
     
     // Save JSON to file
     if (!_fsManager->saveJsonToFile(messagesPath, doc)) {
         TDECK_LOG_E("Failed to save messages to storage");
         return;
     }
     
     TDECK_LOG_I("Saved %d messages to storage", _messages.size());
 }
 
 void LoRaMessenger::loadSettings() {
     // Load settings from storage
     if (!_fsManager->fileExists(SETTINGS_PATH)) {
         // Use default settings
         TDECK_LOG_I("No saved settings found, using defaults");
         
         // Set default values
         _userIdentifier = "User";
         
         // Save default settings
         saveSettings();
         return;
     }
     
     // Load JSON file
     DynamicJsonDocument doc(512);
     
     if (!_fsManager->loadJsonFromFile(SETTINGS_PATH, doc)) {
         TDECK_LOG_E("Failed to load settings from storage");
         return;
     }
     
     // Parse settings
     _userIdentifier = doc["userName"].as<String>();
     
     // Apply LoRa configuration
     if (doc.containsKey("frequency")) {
         long frequency = doc["frequency"].as<long>();
         _loraManager->setFrequency(frequency);
     }
     
     if (doc.containsKey("spreadingFactor")) {
         int sf = doc["spreadingFactor"].as<int>();
         _loraManager->setSpreadingFactor(sf);
     }
     
     if (doc.containsKey("bandwidth")) {
         long bw = doc["bandwidth"].as<long>();
         _loraManager->setBandwidth(bw);
     }
     
     TDECK_LOG_I("Loaded settings: userName=%s", _userIdentifier.c_str());
 }
 
 void LoRaMessenger::saveSettings() {
     // Save settings to storage
     DynamicJsonDocument doc(512);
     
     // Add settings to JSON
     doc["userName"] = _userIdentifier;
     doc["frequency"] = _loraManager->getFrequency();
     doc["spreadingFactor"] = _loraManager->getSpreadingFactor();
     doc["bandwidth"] = _loraManager->getBandwidth();
     
     // Save JSON to file
     if (!_fsManager->saveJsonToFile(SETTINGS_PATH, doc)) {
         TDECK_LOG_E("Failed to save settings");
         return;
     }
     
     TDECK_LOG_I("Settings saved");
 }
 
 void LoRaMessenger::showSettingsDialog() {
     // Create settings dialog if it doesn't exist
     if (!_settingsDialog) {
         _settingsDialog = lv_mbox_create(_parent, NULL);
         lv_obj_set_size(_settingsDialog, TDECK_DISPLAY_WIDTH - 40, TDECK_DISPLAY_HEIGHT - 80);
         lv_obj_align(_settingsDialog, NULL, LV_ALIGN_CENTER, 0, 0);
         lv_mbox_set_text(_settingsDialog, "LoRa Messenger Settings");
         
         // Create a container for settings
         lv_obj_t* settings_cont = lv_cont_create(_settingsDialog, NULL);
         lv_obj_set_size(settings_cont, TDECK_DISPLAY_WIDTH - 80, TDECK_DISPLAY_HEIGHT - 160);
         lv_cont_set_layout(settings_cont, LV_LAYOUT_COLUMN_MID);
         
         // Name setting
         lv_obj_t* nameLabel = lv_label_create(settings_cont, NULL);
         lv_label_set_text(nameLabel, "Your Name:");
         
         _nameInput = lv_ta_create(settings_cont, NULL);
         lv_obj_set_width(_nameInput, TDECK_DISPLAY_WIDTH - 100);
         lv_ta_set_text(_nameInput, _userIdentifier.c_str());
         
         // Frequency setting
         lv_obj_t* freqLabel = lv_label_create(settings_cont, NULL);
         lv_label_set_text(freqLabel, "Frequency (MHz):");
         
         _frequencyInput = lv_ta_create(settings_cont, NULL);
         lv_obj_set_width(_frequencyInput, TDECK_DISPLAY_WIDTH - 100);
         char freqStr[20];
         sprintf(freqStr, "%.2f", _loraManager->getFrequency() / 1E6);
         lv_ta_set_text(_frequencyInput, freqStr);
         
         // Spreading factor setting
         lv_obj_t* sfLabel = lv_label_create(settings_cont, NULL);
         lv_label_set_text(sfLabel, "Spreading Factor (7-12):");
         
         _spreadingInput = lv_ta_create(settings_cont, NULL);
         lv_obj_set_width(_spreadingInput, TDECK_DISPLAY_WIDTH - 100);
         char sfStr[5];
         sprintf(sfStr, "%d", _loraManager->getSpreadingFactor());
         lv_ta_set_text(_spreadingInput, sfStr);
         
         // Bandwidth setting
         lv_obj_t* bwLabel = lv_label_create(settings_cont, NULL);
         lv_label_set_text(bwLabel, "Bandwidth (kHz):");
         
         _bandwidthInput = lv_ta_create(settings_cont, NULL);
         lv_obj_set_width(_bandwidthInput, TDECK_DISPLAY_WIDTH - 100);
         char bwStr[10];
         sprintf(bwStr, "%.1f", _loraManager->getBandwidth() / 1E3);
         lv_ta_set_text(_bandwidthInput, bwStr);
         
         // Add action buttons
         static const char* btn_map[] = {"Apply", "Cancel", ""};
         lv_mbox_add_btns(_settingsDialog, btn_map);
         
         // Set user data
         lv_obj_set_user_data(_settingsDialog, this);
         
         // Set button callbacks
         lv_obj_set_event_cb(_settingsDialog, [](lv_obj_t* obj, lv_event_t event) {
             if (event == LV_EVENT_VALUE_CHANGED) {
                 LoRaMessenger* self = (LoRaMessenger*)lv_obj_get_user_data(obj);
                 const char* txt = lv_mbox_get_active_btn_text(obj);
                 
                 if (strcmp(txt, "Apply") == 0) {
                     self->applySettings();
                 }
                 
                 // Close dialog regardless of which button was pressed
                 lv_obj_del(obj);
                 self->_settingsDialog = nullptr;
             }
         });
     } else {
         // Dialog already exists, just show it
         lv_obj_set_hidden(_settingsDialog, false);
     }
 }