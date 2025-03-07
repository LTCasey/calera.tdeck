/**
 * @file lora.h
 * @brief LoRa module interface for LilyGO T-Deck
 * 
 * This file contains the interface for the LoRa module
 * functionality used in the T-Deck hardware.
 */

 #ifndef TDECK_LORA_H
 #define TDECK_LORA_H
 
 #include <Arduino.h>
 #include <SPI.h>
 #include <LoRa.h>
 #include "../../config.h"
 
 // LoRa packet structure
 struct LoRaPacket {
     uint8_t destination;     // Destination address
     uint8_t source;          // Source address
     uint8_t id;              // Packet ID
     uint8_t flags;           // Packet flags
     uint8_t length;          // Payload length
     uint8_t payload[240];    // Payload data (max 240 bytes)
 };
 
 // LoRa transmission modes
 enum class LoRaMode {
     STANDBY,                // Standby mode
     SLEEP,                  // Sleep mode
     TRANSMIT,               // Transmit mode
     RECEIVE,                // Receive mode
     CAD                     // Channel activity detection mode
 };
 
 // LoRa packet flags
 #define LORA_FLAG_ACK       0x01    // Acknowledgment packet
 #define LORA_FLAG_RELIABLE  0x02    // Reliable transmission (requires ACK)
 #define LORA_FLAG_BROADCAST 0x04    // Broadcast packet
 #define LORA_FLAG_ENCRYPTED 0x08    // Encrypted payload
 #define LORA_FLAG_FRAGMENT  0x10    // Fragmented packet
 #define LORA_FLAG_LAST_FRAG 0x20    // Last fragment
 
 // LoRa transmission status
 enum class LoRaStatus {
     OK,                     // Operation successful
     ERROR,                  // General error
     TIMEOUT,                // Timeout occurred
     BUSY,                   // Module busy
     INVALID_PARAM,          // Invalid parameter
     NO_ACK                  // No acknowledgment received
 };
 
 /**
  * @class LoRaModule
  * @brief Hardware abstraction layer for the T-Deck LoRa module
  * 
  * This class handles the initialization and interaction with the
  * LoRa module on the T-Deck hardware.
  */
 class LoRaModule {
 public:
     /**
      * @brief Construct a new LoRaModule object
      */
     LoRaModule();
 
     /**
      * @brief Initialize the LoRa module
      * 
      * @param frequency Frequency in Hz (default: 915E6 for 915MHz)
      * @param sckPin SCK pin number (optional, uses default if not specified)
      * @param misoPin MISO pin number (optional, uses default if not specified)
      * @param mosiPin MOSI pin number (optional, uses default if not specified)
      * @param csPin CS pin number (defaults to TDECK_LORA_CS)
      * @param resetPin Reset pin number (defaults to TDECK_LORA_RST)
      * @param irqPin IRQ pin number (defaults to TDECK_LORA_DIO)
      * @return true if initialization was successful
      * @return false if initialization failed
      */
     bool begin(float frequency = TDECK_LORA_FREQUENCY, 
                int8_t sckPin = -1, 
                int8_t misoPin = -1, 
                int8_t mosiPin = -1, 
                int8_t csPin = TDECK_LORA_CS, 
                int8_t resetPin = TDECK_LORA_RST, 
                int8_t irqPin = TDECK_LORA_DIO);
 
     /**
      * @brief End LoRa operations
      */
     void end();
 
     /**
      * @brief Check if LoRa module is available
      * 
      * @return true if LoRa module is available
      * @return false if LoRa module is not available
      */
     bool isAvailable() const;
 
     /**
      * @brief Set transmission frequency
      * 
      * @param frequency Frequency in Hz
      * @return true if successful
      * @return false if failed
      */
     bool setFrequency(float frequency);
 
     /**
      * @brief Get current transmission frequency
      * 
      * @return float Current frequency in Hz
      */
     float getFrequency() const;
 
     /**
      * @brief Set LoRa signal bandwidth
      * 
      * @param bandwidth Bandwidth in Hz (7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, 500E3)
      * @return true if successful
      * @return false if failed
      */
     bool setBandwidth(float bandwidth);
 
     /**
      * @brief Set LoRa spreading factor
      * 
      * @param spreadingFactor Spreading factor (6-12)
      * @return true if successful
      * @return false if failed
      */
     bool setSpreadingFactor(uint8_t spreadingFactor);
 
     /**
      * @brief Set LoRa coding rate
      * 
      * @param codingRate Coding rate denominator (5-8, giving coding rates 4/5, 4/6, 4/7, 4/8)
      * @return true if successful
      * @return false if failed
      */
     bool setCodingRate(uint8_t codingRate);
 
     /**
      * @brief Set LoRa sync word
      * 
      * @param syncWord Sync word (0-255)
      * @return true if successful
      * @return false if failed
      */
     bool setSyncWord(uint8_t syncWord);
 
     /**
      * @brief Set LoRa output power
      * 
      * @param power Output power in dBm (2-20)
      * @return true if successful
      * @return false if failed
      */
     bool setTxPower(int power);
 
     /**
      * @brief Set LoRa operation mode
      * 
      * @param mode LoRa operation mode
      * @return true if successful
      * @return false if failed
      */
     bool setMode(LoRaMode mode);
 
     /**
      * @brief Get current LoRa operation mode
      * 
      * @return LoRaMode Current mode
      */
     LoRaMode getMode() const;
 
     /**
      * @brief Set node address
      * 
      * @param address Node address (0-254, 255 is broadcast)
      */
     void setNodeAddress(uint8_t address);
 
     /**
      * @brief Get node address
      * 
      * @return uint8_t Node address
      */
     uint8_t getNodeAddress() const;
 
     /**
      * @brief Check if a packet is available
      * 
      * @return true if packet is available
      * @return false if no packet is available
      */
     bool isPacketAvailable();
 
     /**
      * @brief Get the RSSI (Received Signal Strength Indicator) of the last received packet
      * 
      * @return int RSSI in dBm
      */
     int getPacketRssi();
 
     /**
      * @brief Get the SNR (Signal-to-Noise Ratio) of the last received packet
      * 
      * @return float SNR in dB
      */
     float getPacketSnr();
 
     /**
      * @brief Get the frequency error of the last received packet
      * 
      * @return long Frequency error in Hz
      */
     long getPacketFrequencyError();
 
     /**
      * @brief Send a LoRa packet
      * 
      * @param packet Packet to send
      * @param timeout Timeout in milliseconds (0 for no timeout)
      * @return LoRaStatus Transmission status
      */
     LoRaStatus sendPacket(LoRaPacket& packet, uint32_t timeout = 0);
 
     /**
      * @brief Receive a LoRa packet
      * 
      * @param packet Packet to store received data
      * @param timeout Timeout in milliseconds (0 for no timeout)
      * @return LoRaStatus Reception status
      */
     LoRaStatus receivePacket(LoRaPacket& packet, uint32_t timeout = 0);
 
     /**
      * @brief Set packet transmission timeout
      * 
      * @param timeout Timeout in milliseconds
      */
     void setTimeout(uint32_t timeout);
 
     /**
      * @brief Get packet transmission timeout
      * 
      * @return uint32_t Timeout in milliseconds
      */
     uint32_t getTimeout() const;
 
     /**
      * @brief Register a callback function for packet reception
      * 
      * @param callback Function pointer to callback
      */
     void onReceive(void (*callback)(int));
 
     /**
      * @brief Register a callback function for packet transmission completed
      * 
      * @param callback Function pointer to callback
      */
     void onTransmit(void (*callback)(void));
 
 private:
     bool _available;             // LoRa module availability flag
     SPIClass* _spi;              // SPI interface
     float _frequency;            // Current frequency
     float _bandwidth;            // Current bandwidth
     uint8_t _spreadingFactor;    // Current spreading factor
     uint8_t _codingRate;         // Current coding rate
     uint8_t _syncWord;           // Current sync word
     int _txPower;                // Current TX power
     LoRaMode _mode;              // Current operation mode
     uint8_t _nodeAddress;        // Node address
     uint32_t _timeout;           // Transmission timeout
     uint8_t _packetId;           // Packet ID counter
     
     /**
      * @brief Initialize LoRa module with current settings
      * 
      * @return true if successful
      * @return false if failed
      */
     bool initLoRa();
     
     /**
      * @brief Wait for LoRa transmission to complete
      * 
      * @param timeout Timeout in milliseconds
      * @return LoRaStatus Wait status
      */
     LoRaStatus waitForTxReady(uint32_t timeout);
 };
 
 // Global LoRa module instance
 extern LoRaModule lora;
 
 #endif // TDECK_LORA_H