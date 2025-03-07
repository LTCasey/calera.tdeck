/**
 * @file lora.h
 * @brief LoRa communication management for T-Deck UI Firmware
 * 
 * This file contains the LoRaManager class that provides
 * an interface for LoRa communication functionality on the T-Deck.
 */

 #ifndef TDECK_COMMS_LORA_H
 #define TDECK_COMMS_LORA_H
 
 #include <Arduino.h>
 #include <LoRa.h>
 #include <ArduinoJson.h>
 #include "../config.h"
 
 // Maximum packet size for LoRa transmission
 #define LORA_MAX_PACKET_SIZE 256
 
 // Message types for LoRa packets
 enum LoRaMessageType {
     LORA_MSG_TEXT = 0,      // Plain text message
     LORA_MSG_LOCATION = 1,  // Location data
     LORA_MSG_STATUS = 2,    // Status update
     LORA_MSG_COMMAND = 3,   // Command message
     LORA_MSG_ACK = 4        // Acknowledgment
 };
 
 // LoRa packet structure
 struct LoRaPacket {
     uint8_t messageType;    // Type of message
     uint16_t sourceId;      // Source device ID
     uint16_t destId;        // Destination device ID (0xFFFF for broadcast)
     uint16_t packetId;      // Unique packet ID
     uint8_t length;         // Payload length
     uint8_t payload[LORA_MAX_PACKET_SIZE]; // Message payload
 };
 
 // Callback function type for received messages
 typedef void (*LoRaMessageCallback)(LoRaPacket* packet);
 
 /**
  * @class LoRaManager
  * @brief Manages LoRa communication functionality
  * 
  * This class provides methods to initialize the LoRa module,
  * send and receive messages, and manage LoRa configuration.
  */
 class LoRaManager {
 public:
     /**
      * @brief Constructor
      */
     LoRaManager();
     
     /**
      * @brief Destructor
      */
     ~LoRaManager();
     
     /**
      * @brief Initialize the LoRa module
      * @return true if successful, false otherwise
      */
     bool init();
     
     /**
      * @brief Update routine to be called periodically
      * Checks for received messages and handles them
      */
     void update();
     
     /**
      * @brief Send a text message via LoRa
      * @param message The message to send
      * @param destId Destination device ID (0xFFFF for broadcast)
      * @return true if message was sent successfully, false otherwise
      */
     bool sendTextMessage(const String& message, uint16_t destId = 0xFFFF);
     
     /**
      * @brief Send location data via LoRa
      * @param latitude Latitude coordinate
      * @param longitude Longitude coordinate
      * @param altitude Altitude in meters
      * @param destId Destination device ID (0xFFFF for broadcast)
      * @return true if data was sent successfully, false otherwise
      */
     bool sendLocationData(float latitude, float longitude, float altitude, uint16_t destId = 0xFFFF);
     
     /**
      * @brief Send status update via LoRa
      * @param status JSON object containing status data
      * @param destId Destination device ID (0xFFFF for broadcast)
      * @return true if status was sent successfully, false otherwise
      */
     bool sendStatusUpdate(const JsonDocument& status, uint16_t destId = 0xFFFF);
     
     /**
      * @brief Send a command via LoRa
      * @param command Command string
      * @param params JSON object containing command parameters
      * @param destId Destination device ID (0xFFFF for broadcast)
      * @return true if command was sent successfully, false otherwise
      */
     bool sendCommand(const String& command, const JsonDocument& params, uint16_t destId = 0xFFFF);
     
     /**
      * @brief Send a raw LoRa packet
      * @param packet The packet to send
      * @return true if packet was sent successfully, false otherwise
      */
     bool sendPacket(const LoRaPacket& packet);
     
     /**
      * @brief Register a callback for received messages
      * @param callback Function to call when a message is received
      */
     void setMessageCallback(LoRaMessageCallback callback);
     
     /**
      * @brief Configure LoRa parameters
      * @param frequency Frequency in Hz
      * @param bandwidth Bandwidth in Hz
      * @param spreadingFactor Spreading factor (7-12)
      * @param codingRate Coding rate (5-8, representing 4/5-4/8)
      * @param syncWord Sync word
      * @return true if configuration was successful, false otherwise
      */
     bool configure(long frequency, long bandwidth, int spreadingFactor, int codingRate, int syncWord);
     
     /**
      * @brief Load configuration from file
      * @return true if configuration was loaded successfully, false otherwise
      */
     bool loadConfig();
     
     /**
      * @brief Save current configuration to file
      * @return true if configuration was saved successfully, false otherwise
      */
     bool saveConfig();
     
     /**
      * @brief Get the current device ID
      * @return Device ID
      */
     uint16_t getDeviceId() const;
     
     /**
      * @brief Set the device ID
      * @param id New device ID
      */
     void setDeviceId(uint16_t id);
     
     /**
      * @brief Check if LoRa is currently enabled
      * @return true if enabled, false otherwise
      */
     bool isEnabled() const;
     
     /**
      * @brief Enable or disable LoRa functionality
      * @param enable true to enable, false to disable
      */
     void setEnabled(bool enable);
     
     /**
      * @brief Get the current RSSI (Received Signal Strength Indicator)
      * @return RSSI value in dBm
      */
     int getRSSI() const;
     
     /**
      * @brief Get the current SNR (Signal to Noise Ratio)
      * @return SNR value in dB
      */
     float getSNR() const;
     
 private:
     bool _initialized;           // Initialization flag
     bool _enabled;               // Whether LoRa is currently enabled
     uint16_t _deviceId;          // This device's ID
     uint16_t _packetCounter;     // Counter for packet IDs
     long _frequency;             // Current frequency
     long _bandwidth;             // Current bandwidth
     int _spreadingFactor;        // Current spreading factor
     int _codingRate;             // Current coding rate
     int _syncWord;               // Current sync word
     int _lastRssi;               // Last RSSI value
     float _lastSnr;              // Last SNR value
     LoRaMessageCallback _messageCallback; // Callback for received messages
     
     /**
      * @brief Handle received LoRa packets
      */
     void handleReceivedPacket();
     
     /**
      * @brief Create a packet with the specified parameters
      * @param messageType Type of message
      * @param destId Destination device ID
      * @param payload Pointer to payload data
      * @param length Length of payload
      * @return Constructed LoRa packet
      */
     LoRaPacket createPacket(uint8_t messageType, uint16_t destId, const uint8_t* payload, uint8_t length);
     
     /**
      * @brief Send an acknowledgment for a received packet
      * @param receivedPacket The packet to acknowledge
      * @return true if acknowledgment was sent successfully, false otherwise
      */
     bool sendAcknowledgment(const LoRaPacket& receivedPacket);
 };
 
 // Global instance
 extern LoRaManager loraManager;
 
 #endif // TDECK_COMMS_LORA_H