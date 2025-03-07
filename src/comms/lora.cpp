/**
 * @file lora.cpp
 * @brief Implementation of LoRa communication for T-Deck UI Firmware
 */

 #include "lora.h"
 #include "../system/fs_manager.h"
 #include <LoRa.h>
 
 // Global instance
 LoRaManager loraManager;
 
 // Default device ID (last 2 bytes of ESP32 MAC address)
 static uint16_t getDefaultDeviceId() {
     uint8_t mac[6];
     esp_read_mac(mac, ESP_MAC_WIFI_STA);
     return (mac[4] << 8) | mac[5];
 }
 
 // Constructor
 LoRaManager::LoRaManager() 
     : _initialized(false)
     , _enabled(false)
     , _deviceId(getDefaultDeviceId())
     , _packetCounter(0)
     , _frequency(TDECK_LORA_FREQUENCY)
     , _bandwidth(TDECK_LORA_BANDWIDTH)
     , _spreadingFactor(TDECK_LORA_SPREADING_FACTOR)
     , _codingRate(TDECK_LORA_CODING_RATE)
     , _syncWord(TDECK_LORA_SYNC_WORD)
     , _lastRssi(0)
     , _lastSnr(0)
     , _messageCallback(nullptr)
 {
 }
 
 // Destructor
 LoRaManager::~LoRaManager() {
     if (_initialized) {
         LoRa.end();
         _initialized = false;
     }
 }
 
 // Initialize the LoRa module
 bool LoRaManager::init() {
     TDECK_LOG_I("Initializing LoRa...");
     
     // Load configuration from file
     loadConfig();
     
     // Initialize the LoRa module with current settings
     LoRa.setPins(TDECK_LORA_CS, TDECK_LORA_RST, TDECK_LORA_DIO);
     
     if (!LoRa.begin(_frequency)) {
         TDECK_LOG_E("LoRa initialization failed");
         return false;
     }
     
     // Configure LoRa parameters
     LoRa.setSignalBandwidth(_bandwidth);
     LoRa.setSpreadingFactor(_spreadingFactor);
     LoRa.setCodingRate4(_codingRate);
     LoRa.setSyncWord(_syncWord);
     
     // Enable CRC checking
     LoRa.enableCrc();
     
     _initialized = true;
     _enabled = true;
     
     TDECK_LOG_I("LoRa initialized at %ld Hz, DeviceID: 0x%04X", _frequency, _deviceId);
     
     return true;
 }
 
 // Update routine to check for received messages
 void LoRaManager::update() {
     if (!_initialized || !_enabled) {
         return;
     }
     
     // Check if a packet is available
     int packetSize = LoRa.parsePacket();
     if (packetSize > 0) {
         handleReceivedPacket();
     }
 }
 
 // Send a text message via LoRa
 bool LoRaManager::sendTextMessage(const String& message, uint16_t destId) {
     if (!_initialized || !_enabled) {
         return false;
     }
     
     // Convert String to byte array
     uint8_t payload[LORA_MAX_PACKET_SIZE];
     size_t length = message.length();
     
     if (length > LORA_MAX_PACKET_SIZE) {
         TDECK_LOG_W("Message too long, truncating");
         length = LORA_MAX_PACKET_SIZE;
     }
     
     memcpy(payload, message.c_str(), length);
     
     // Create and send packet
     LoRaPacket packet = createPacket(LORA_MSG_TEXT, destId, payload, length);
     return sendPacket(packet);
 }
 
 // Send location data via LoRa
 bool LoRaManager::sendLocationData(float latitude, float longitude, float altitude, uint16_t destId) {
     if (!_initialized || !_enabled) {
         return false;
     }
     
     // Create a JSON document for the location data
     StaticJsonDocument<128> doc;
     doc["lat"] = latitude;
     doc["lon"] = longitude;
     doc["alt"] = altitude;
     
     // Serialize to byte array
     uint8_t payload[LORA_MAX_PACKET_SIZE];
     size_t length = serializeJson(doc, (char*)payload, LORA_MAX_PACKET_SIZE);
     
     // Create and send packet
     LoRaPacket packet = createPacket(LORA_MSG_LOCATION, destId, payload, length);
     return sendPacket(packet);
 }
 
 // Send status update via LoRa
 bool LoRaManager::sendStatusUpdate(const JsonDocument& status, uint16_t destId) {
     if (!_initialized || !_enabled) {
         return false;
     }
     
     // Serialize to byte array
     uint8_t payload[LORA_MAX_PACKET_SIZE];
     size_t length = serializeJson(status, (char*)payload, LORA_MAX_PACKET_SIZE);
     
     if (length > LORA_MAX_PACKET_SIZE) {
         TDECK_LOG_W("Status data too large, truncating");
         length = LORA_MAX_PACKET_SIZE;
     }
     
     // Create and send packet
     LoRaPacket packet = createPacket(LORA_MSG_STATUS, destId, payload, length);
     return sendPacket(packet);
 }
 
 // Send a command via LoRa
 bool LoRaManager::sendCommand(const String& command, const JsonDocument& params, uint16_t destId) {
     if (!_initialized || !_enabled) {
         return false;
     }
     
     // Create a combined JSON document
     StaticJsonDocument<256> doc;
     doc["cmd"] = command;
     doc["params"] = params;
     
     // Serialize to byte array
     uint8_t payload[LORA_MAX_PACKET_SIZE];
     size_t length = serializeJson(doc, (char*)payload, LORA_MAX_PACKET_SIZE);
     
     if (length > LORA_MAX_PACKET_SIZE) {
         TDECK_LOG_W("Command data too large, truncating");
         length = LORA_MAX_PACKET_SIZE;
     }
     
     // Create and send packet
     LoRaPacket packet = createPacket(LORA_MSG_COMMAND, destId, payload, length);
     return sendPacket(packet);
 }
 
 // Send a raw LoRa packet
 bool LoRaManager::sendPacket(const LoRaPacket& packet) {
     if (!_initialized || !_enabled) {
         return false;
     }
     
     // Begin packet transmission
     LoRa.beginPacket();
     
     // Write packet header
     LoRa.write(packet.messageType);
     LoRa.write((uint8_t)(packet.sourceId >> 8));
     LoRa.write((uint8_t)(packet.sourceId & 0xFF));
     LoRa.write((uint8_t)(packet.destId >> 8));
     LoRa.write((uint8_t)(packet.destId & 0xFF));
     LoRa.write((uint8_t)(packet.packetId >> 8));
     LoRa.write((uint8_t)(packet.packetId & 0xFF));
     LoRa.write(packet.length);
     
     // Write payload
     LoRa.write(packet.payload, packet.length);
     
     // End packet and transmit
     bool result = LoRa.endPacket();
     
     if (result) {
         TDECK_LOG_I("Sent LoRa packet: type=%d, dest=0x%04X, id=%d, len=%d", 
                     packet.messageType, packet.destId, packet.packetId, packet.length);
     } else {
         TDECK_LOG_E("Failed to send LoRa packet");
     }
     
     return result;
 }
 
 // Register a callback for received messages
 void LoRaManager::setMessageCallback(LoRaMessageCallback callback) {
     _messageCallback = callback;
 }
 
 // Configure LoRa parameters
 bool LoRaManager::configure(long frequency, long bandwidth, int spreadingFactor, int codingRate, int syncWord) {
     if (!_initialized) {
         _frequency = frequency;
         _bandwidth = bandwidth;
         _spreadingFactor = spreadingFactor;
         _codingRate = codingRate;
         _syncWord = syncWord;
         return true;
     }
     
     // Update settings on the fly
     bool success = true;
     
     // Update frequency if changed
     if (frequency != _frequency) {
         if (LoRa.setFrequency(frequency)) {
             _frequency = frequency;
         } else {
             TDECK_LOG_E("Failed to set frequency: %ld Hz", frequency);
             success = false;
         }
     }
     
     // Update bandwidth if changed
     if (bandwidth != _bandwidth) {
         LoRa.setSignalBandwidth(bandwidth);
         _bandwidth = bandwidth;
     }
     
     // Update spreading factor if changed
     if (spreadingFactor != _spreadingFactor && spreadingFactor >= 6 && spreadingFactor <= 12) {
         LoRa.setSpreadingFactor(spreadingFactor);
         _spreadingFactor = spreadingFactor;
     } else if (spreadingFactor < 6 || spreadingFactor > 12) {
         TDECK_LOG_E("Invalid spreading factor: %d", spreadingFactor);
         success = false;
     }
     
     // Update coding rate if changed
     if (codingRate != _codingRate && codingRate >= 5 && codingRate <= 8) {
         LoRa.setCodingRate4(codingRate);
         _codingRate = codingRate;
     } else if (codingRate < 5 || codingRate > 8) {
         TDECK_LOG_E("Invalid coding rate: %d", codingRate);
         success = false;
     }
     
     // Update sync word if changed
     if (syncWord != _syncWord) {
         LoRa.setSyncWord(syncWord);
         _syncWord = syncWord;
     }
     
     // Save configuration to file
     if (success) {
         saveConfig();
     }
     
     return success;
 }
 
 // Load configuration from file
 bool LoRaManager::loadConfig() {
     StaticJsonDocument<256> doc;
     if (!fsManager.loadJsonFromFile(TDECK_FS_LORA_CONFIG_FILE, doc)) {
         TDECK_LOG_W("LoRa config file not found, using defaults");
         return false;
     }
     
     // Load values from JSON, with defaults if not present
     _frequency = doc["frequency"] | TDECK_LORA_FREQUENCY;
     _bandwidth = doc["bandwidth"] | TDECK_LORA_BANDWIDTH;
     _spreadingFactor = doc["spreadingFactor"] | TDECK_LORA_SPREADING_FACTOR;
     _codingRate = doc["codingRate"] | TDECK_LORA_CODING_RATE;
     _syncWord = doc["syncWord"] | TDECK_LORA_SYNC_WORD;
     _deviceId = doc["deviceId"] | getDefaultDeviceId();
     _enabled = doc["enabled"] | true;
     
     TDECK_LOG_I("Loaded LoRa configuration: %ld Hz, SF%d, BW %ld, CR 4/%d", 
                 _frequency, _spreadingFactor, _bandwidth, _codingRate);
     
     return true;
 }
 
 // Save current configuration to file
 bool LoRaManager::saveConfig() {
     // Create directory if it doesn't exist
     fsManager.createDir(TDECK_FS_CONFIG_DIR);
     
     // Create JSON document with current settings
     StaticJsonDocument<256> doc;
     doc["frequency"] = _frequency;
     doc["bandwidth"] = _bandwidth;
     doc["spreadingFactor"] = _spreadingFactor;
     doc["codingRate"] = _codingRate;
     doc["syncWord"] = _syncWord;
     doc["deviceId"] = _deviceId;
     doc["enabled"] = _enabled;
     
     // Save to file
     bool result = fsManager.saveJsonToFile(TDECK_FS_LORA_CONFIG_FILE, doc);
     if (!result) {
         TDECK_LOG_E("Failed to save LoRa configuration");
     }
     
     return result;
 }
 
 // Get the current device ID
 uint16_t LoRaManager::getDeviceId() const {
     return _deviceId;
 }
 
 // Set the device ID
 void LoRaManager::setDeviceId(uint16_t id) {
     _deviceId = id;
     saveConfig();
 }
 
 // Check if LoRa is currently enabled
 bool LoRaManager::isEnabled() const {
     return _enabled;
 }
 
 // Enable or disable LoRa functionality
 void LoRaManager::setEnabled(bool enable) {
     _enabled = enable;
     saveConfig();
 }
 
 // Get the current RSSI
 int LoRaManager::getRSSI() const {
     return _lastRssi;
 }
 
 // Get the current SNR
 float LoRaManager::getSNR() const {
     return _lastSnr;
 }
 
 // Handle received LoRa packets
 void LoRaManager::handleReceivedPacket() {
     // Read packet size
     int packetSize = LoRa.available();
     
     if (packetSize < 8) {  // Minimum header size
         TDECK_LOG_W("Received malformed LoRa packet (too small)");
         return;
     }
     
     // Store signal quality metrics
     _lastRssi = LoRa.packetRssi();
     _lastSnr = LoRa.packetSnr();
     
     // Read header fields
     uint8_t messageType = LoRa.read();
     uint16_t sourceId = (LoRa.read() << 8) | LoRa.read();
     uint16_t destId = (LoRa.read() << 8) | LoRa.read();
     uint16_t packetId = (LoRa.read() << 8) | LoRa.read();
     uint8_t length = LoRa.read();
     
     // Check if packet is for us (or broadcast)
     if (destId != _deviceId && destId != 0xFFFF) {
         // Not for us, ignore
         return;
     }
     
     // Read payload
     LoRaPacket packet;
     packet.messageType = messageType;
     packet.sourceId = sourceId;
     packet.destId = destId;
     packet.packetId = packetId;
     packet.length = length;
     
     // Check payload length
     if (length > LORA_MAX_PACKET_SIZE) {
         TDECK_LOG_W("Received LoRa packet with invalid payload length: %d", length);
         return;
     }
     
     // Read payload data
     int bytesRead = 0;
     while (LoRa.available() && bytesRead < length) {
         packet.payload[bytesRead++] = LoRa.read();
     }
     
     // Log packet details
     TDECK_LOG_I("Received LoRa packet: type=%d, source=0x%04X, id=%d, len=%d, RSSI=%d, SNR=%.1f", 
                 messageType, sourceId, packetId, length, _lastRssi, _lastSnr);
     
     // Handle packet based on type
     switch (messageType) {
         case LORA_MSG_TEXT:
             // Null-terminate the payload for string operations
             if (length < LORA_MAX_PACKET_SIZE) {
                 packet.payload[length] = 0;
             } else {
                 packet.payload[LORA_MAX_PACKET_SIZE - 1] = 0;
             }
             TDECK_LOG_I("LoRa text message: %s", (char*)packet.payload);
             break;
             
         case LORA_MSG_LOCATION:
             // Parse JSON location data
             {
                 StaticJsonDocument<128> doc;
                 DeserializationError error = deserializeJson(doc, (char*)packet.payload, length);
                 if (!error) {
                     float lat = doc["lat"];
                     float lon = doc["lon"];
                     float alt = doc["alt"];
                     TDECK_LOG_I("LoRa location: lat=%.6f, lon=%.6f, alt=%.1f", lat, lon, alt);
                 } else {
                     TDECK_LOG_W("Failed to parse location data: %s", error.c_str());
                 }
             }
             break;
             
         case LORA_MSG_STATUS:
             // Parse JSON status data
             {
                 StaticJsonDocument<256> doc;
                 DeserializationError error = deserializeJson(doc, (char*)packet.payload, length);
                 if (!error) {
                     String status;
                     serializeJson(doc, status);
                     TDECK_LOG_I("LoRa status: %s", status.c_str());
                 } else {
                     TDECK_LOG_W("Failed to parse status data: %s", error.c_str());
                 }
             }
             break;
             
         case LORA_MSG_COMMAND:
             // Parse JSON command data
             {
                 StaticJsonDocument<256> doc;
                 DeserializationError error = deserializeJson(doc, (char*)packet.payload, length);
                 if (!error) {
                     String command = doc["cmd"];
                     TDECK_LOG_I("LoRa command: %s", command.c_str());
                 } else {
                     TDECK_LOG_W("Failed to parse command data: %s", error.c_str());
                 }
             }
             break;
             
         case LORA_MSG_ACK:
             // Acknowledgment packet
             {
                 uint16_t ackPacketId = (packet.payload[0] << 8) | packet.payload[1];
                 TDECK_LOG_I("LoRa acknowledgment for packet ID: %d", ackPacketId);
             }
             break;
             
         default:
             TDECK_LOG_W("Unknown LoRa message type: %d", messageType);
             break;
     }
     
     // Send acknowledgment if not a broadcast or ACK
     if (destId != 0xFFFF && messageType != LORA_MSG_ACK) {
         sendAcknowledgment(packet);
     }
     
     // Call the message callback if registered
     if (_messageCallback != nullptr) {
         _messageCallback(&packet);
     }
 }
 
 // Create a packet with the specified parameters
 LoRaPacket LoRaManager::createPacket(uint8_t messageType, uint16_t destId, const uint8_t* payload, uint8_t length) {
     LoRaPacket packet;
     packet.messageType = messageType;
     packet.sourceId = _deviceId;
     packet.destId = destId;
     packet.packetId = _packetCounter++;
     packet.length = length;
     
     if (length > 0 && payload != nullptr) {
         memcpy(packet.payload, payload, length);
     }
     
     return packet;
 }
 
 // Send an acknowledgment for a received packet
 bool LoRaManager::sendAcknowledgment(const LoRaPacket& receivedPacket) {
     uint8_t payload[2];
     payload[0] = (receivedPacket.packetId >> 8) & 0xFF;
     payload[1] = receivedPacket.packetId & 0xFF;
     
     LoRaPacket ackPacket = createPacket(LORA_MSG_ACK, receivedPacket.sourceId, payload, 2);
     return sendPacket(ackPacket);
 }