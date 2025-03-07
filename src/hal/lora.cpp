/**
 * @file lora.cpp
 * @brief LoRa module implementation for LilyGO T-Deck
 * 
 * This file implements the LoRa module functionality for the T-Deck hardware.
 */

 #include "lora.h"

 // Create global LoRa module instance
 LoRaModule lora;
 
 LoRaModule::LoRaModule() :
     _available(false),
     _frequency(TDECK_LORA_FREQUENCY),
     _bandwidth(TDECK_LORA_BANDWIDTH),
     _spreadingFactor(TDECK_LORA_SPREADING_FACTOR),
     _codingRate(TDECK_LORA_CODING_RATE),
     _syncWord(TDECK_LORA_SYNC_WORD),
     _txPower(17), // Default to 17 dBm
     _mode(LoRaMode::STANDBY),
     _nodeAddress(0x01), // Default address
     _timeout(1000), // Default timeout 1 second
     _packetId(0)
 {
     _spi = new SPIClass(VSPI);
 }
 
 bool LoRaModule::begin(float frequency, int8_t sckPin, int8_t misoPin, int8_t mosiPin, 
                       int8_t csPin, int8_t resetPin, int8_t irqPin)
 {
     // Initialize SPI for LoRa
     if (sckPin >= 0 && misoPin >= 0 && mosiPin >= 0) {
         _spi->begin(sckPin, misoPin, mosiPin, csPin);
     } else {
         // Use default SPI pins if not specified
         _spi->begin();
     }
     
     // Set SPI frequency
     _spi->setFrequency(8000000); // 8 MHz SPI clock
     
     // Initialize LoRa module
     LoRa.setSPI(*_spi);
     LoRa.setPins(csPin, resetPin, irqPin);
     
     // Set frequency
     _frequency = frequency;
     
     // Try to initialize the radio
     if (!initLoRa()) {
         TDECK_LOG_E("LoRa initialization failed");
         _available = false;
         return false;
     }
     
     _available = true;
     TDECK_LOG_I("LoRa module initialized successfully");
     TDECK_LOG_I("Frequency: %.2f MHz", _frequency / 1.0e6);
     TDECK_LOG_I("Bandwidth: %.2f kHz", _bandwidth / 1.0e3);
     TDECK_LOG_I("Spreading Factor: %d", _spreadingFactor);
     TDECK_LOG_I("Coding Rate: 4/%d", _codingRate);
     TDECK_LOG_I("Sync Word: 0x%02X", _syncWord);
     
     // Set to receive mode by default
     setMode(LoRaMode::RECEIVE);
     
     return true;
 }
 
 void LoRaModule::end()
 {
     if (!_available) {
         return;
     }
     
     // Put LoRa module in sleep mode to save power
     setMode(LoRaMode::SLEEP);
     LoRa.end();
     _available = false;
     TDECK_LOG_I("LoRa module shutdown");
 }
 
 bool LoRaModule::isAvailable() const
 {
     return _available;
 }
 
 bool LoRaModule::setFrequency(float frequency)
 {
     if (!_available) {
         return false;
     }
     
     // Set new frequency
     _frequency = frequency;
     LoRa.setFrequency(_frequency);
     TDECK_LOG_I("LoRa frequency set to %.2f MHz", _frequency / 1.0e6);
     return true;
 }
 
 float LoRaModule::getFrequency() const
 {
     return _frequency;
 }
 
 bool LoRaModule::setBandwidth(float bandwidth)
 {
     if (!_available) {
         return false;
     }
     
     // Check if bandwidth is valid
     // Valid bandwidths: 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, 500E3
     bool valid = (bandwidth == 7.8E3 || bandwidth == 10.4E3 || bandwidth == 15.6E3 ||
                  bandwidth == 20.8E3 || bandwidth == 31.25E3 || bandwidth == 41.7E3 ||
                  bandwidth == 62.5E3 || bandwidth == 125E3 || bandwidth == 250E3 ||
                  bandwidth == 500E3);
     
     if (!valid) {
         TDECK_LOG_E("Invalid LoRa bandwidth: %.2f kHz", bandwidth / 1.0e3);
         return false;
     }
     
     // Set new bandwidth
     _bandwidth = bandwidth;
     LoRa.setSignalBandwidth(_bandwidth);
     TDECK_LOG_I("LoRa bandwidth set to %.2f kHz", _bandwidth / 1.0e3);
     return true;
 }
 
 bool LoRaModule::setSpreadingFactor(uint8_t spreadingFactor)
 {
     if (!_available) {
         return false;
     }
     
     // Check if spreading factor is valid (6-12)
     if (spreadingFactor < 6 || spreadingFactor > 12) {
         TDECK_LOG_E("Invalid LoRa spreading factor: %d", spreadingFactor);
         return false;
     }
     
     // Set new spreading factor
     _spreadingFactor = spreadingFactor;
     LoRa.setSpreadingFactor(_spreadingFactor);
     TDECK_LOG_I("LoRa spreading factor set to %d", _spreadingFactor);
     return true;
 }
 
 bool LoRaModule::setCodingRate(uint8_t codingRate)
 {
     if (!_available) {
         return false;
     }
     
     // Check if coding rate is valid (5-8, for 4/5, 4/6, 4/7, 4/8)
     if (codingRate < 5 || codingRate > 8) {
         TDECK_LOG_E("Invalid LoRa coding rate: 4/%d", codingRate);
         return false;
     }
     
     // Set new coding rate
     _codingRate = codingRate;
     LoRa.setCodingRate4(_codingRate);
     TDECK_LOG_I("LoRa coding rate set to 4/%d", _codingRate);
     return true;
 }
 
 bool LoRaModule::setSyncWord(uint8_t syncWord)
 {
     if (!_available) {
         return false;
     }
     
     // Set new sync word
     _syncWord = syncWord;
     LoRa.setSyncWord(_syncWord);
     TDECK_LOG_I("LoRa sync word set to 0x%02X", _syncWord);
     return true;
 }
 
 bool LoRaModule::setTxPower(int power)
 {
     if (!_available) {
         return false;
     }
     
     // Check if power is valid (2-20 dBm)
     if (power < 2 || power > 20) {
         TDECK_LOG_E("Invalid LoRa TX power: %d dBm", power);
         return false;
     }
     
     // Set new TX power
     _txPower = power;
     LoRa.setTxPower(_txPower);
     TDECK_LOG_I("LoRa TX power set to %d dBm", _txPower);
     return true;
 }
 
 bool LoRaModule::setMode(LoRaMode mode)
 {
     if (!_available) {
         return false;
     }
     
     // Store current mode
     LoRaMode previousMode = _mode;
     
     // Set new mode
     _mode = mode;
     
     switch (_mode) {
         case LoRaMode::STANDBY:
             LoRa.idle();
             TDECK_LOG_I("LoRa mode set to STANDBY");
             break;
             
         case LoRaMode::SLEEP:
             LoRa.sleep();
             TDECK_LOG_I("LoRa mode set to SLEEP");
             break;
             
         case LoRaMode::TRANSMIT:
             // Actual transmission happens in sendPacket()
             LoRa.idle(); // Prepare for transmission
             TDECK_LOG_I("LoRa mode set to TRANSMIT");
             break;
             
         case LoRaMode::RECEIVE:
             LoRa.receive();
             TDECK_LOG_I("LoRa mode set to RECEIVE");
             break;
             
         case LoRaMode::CAD:
             // Channel Activity Detection not directly supported in the library
             // This would require custom implementation using low-level SPI commands
             TDECK_LOG_E("LoRa CAD mode not implemented");
             _mode = previousMode; // Revert to previous mode
             return false;
     }
     
     return true;
 }
 
 LoRaMode LoRaModule::getMode() const
 {
     return _mode;
 }
 
 void LoRaModule::setNodeAddress(uint8_t address)
 {
     _nodeAddress = address;
     TDECK_LOG_I("LoRa node address set to 0x%02X", _nodeAddress);
 }
 
 uint8_t LoRaModule::getNodeAddress() const
 {
     return _nodeAddress;
 }
 
 bool LoRaModule::isPacketAvailable()
 {
     if (!_available) {
         return false;
     }
     
     // Check if module is in receive mode
     if (_mode != LoRaMode::RECEIVE) {
         setMode(LoRaMode::RECEIVE);
     }
     
     // Check if a packet is available
     return LoRa.parsePacket() > 0;
 }
 
 int LoRaModule::getPacketRssi()
 {
     if (!_available) {
         return -150; // Very low value indicating no signal
     }
     
     return LoRa.packetRssi();
 }
 
 float LoRaModule::getPacketSnr()
 {
     if (!_available) {
         return -20.0; // Very low value indicating poor SNR
     }
     
     return LoRa.packetSnr();
 }
 
 long LoRaModule::getPacketFrequencyError()
 {
     if (!_available) {
         return 0;
     }
     
     return LoRa.packetFrequencyError();
 }
 
 LoRaStatus LoRaModule::sendPacket(LoRaPacket& packet, uint32_t timeout)
 {
     if (!_available) {
         return LoRaStatus::ERROR;
     }
     
     // Set transmit mode
     if (_mode != LoRaMode::TRANSMIT) {
         setMode(LoRaMode::TRANSMIT);
     }
     
     // Set source address
     packet.source = _nodeAddress;
     
     // Set packet ID if not already set
     if (packet.id == 0) {
         packet.id = ++_packetId;
     }
     
     // Calculate timeout
     uint32_t effectiveTimeout = (timeout > 0) ? timeout : _timeout;
     
     // Start packet
     LoRa.beginPacket();
     
     // Write header
     LoRa.write(packet.destination);
     LoRa.write(packet.source);
     LoRa.write(packet.id);
     LoRa.write(packet.flags);
     LoRa.write(packet.length);
     
     // Write payload
     LoRa.write(packet.payload, packet.length);
     
     // End packet and send
     bool success = LoRa.endPacket();
     
     // Wait for transmission to complete
     LoRaStatus status = waitForTxReady(effectiveTimeout);
     
     // Check for successful transmission
     if (!success) {
         TDECK_LOG_E("LoRa packet transmission failed");
         return LoRaStatus::ERROR;
     }
     
     // Log transmission
     TDECK_LOG_I("LoRa packet sent: ID=%d, Dst=0x%02X, Src=0x%02X, Len=%d", 
                 packet.id, packet.destination, packet.source, packet.length);
     
     // Handle reliable transmission (requires ACK)
     if (packet.flags & LORA_FLAG_RELIABLE) {
         TDECK_LOG_I("Waiting for ACK for packet ID %d", packet.id);
         
         // Switch to receive mode to wait for ACK
         setMode(LoRaMode::RECEIVE);
         
         // Wait for ACK
         unsigned long startTime = millis();
         while (millis() - startTime < effectiveTimeout) {
             if (isPacketAvailable()) {
                 LoRaPacket ackPacket;
                 if (receivePacket(ackPacket, 100) == LoRaStatus::OK) {
                     // Check if this is an ACK for our packet
                     if (ackPacket.flags & LORA_FLAG_ACK && 
                         ackPacket.id == packet.id && 
                         ackPacket.destination == _nodeAddress && 
                         ackPacket.source == packet.destination) {
                         TDECK_LOG_I("ACK received for packet ID %d", packet.id);
                         return LoRaStatus::OK;
                     }
                 }
             }
             delay(10);
         }
         
         // No ACK received
         TDECK_LOG_W("No ACK received for packet ID %d", packet.id);
         return LoRaStatus::NO_ACK;
     }
     
     return LoRaStatus::OK;
 }
 
 LoRaStatus LoRaModule::receivePacket(LoRaPacket& packet, uint32_t timeout)
 {
     if (!_available) {
         return LoRaStatus::ERROR;
     }
     
     // Set receive mode
     if (_mode != LoRaMode::RECEIVE) {
         setMode(LoRaMode::RECEIVE);
     }
     
     // Check if a packet is already available
     if (!isPacketAvailable()) {
         // Wait for packet
         unsigned long startTime = millis();
         uint32_t effectiveTimeout = (timeout > 0) ? timeout : _timeout;
         
         while (millis() - startTime < effectiveTimeout) {
             if (isPacketAvailable()) {
                 break;
             }
             delay(10);
         }
         
         // Check if we timed out
         if (!isPacketAvailable()) {
             return LoRaStatus::TIMEOUT;
         }
     }
     
     // Read packet header
     packet.destination = LoRa.read();
     packet.source = LoRa.read();
     packet.id = LoRa.read();
     packet.flags = LoRa.read();
     packet.length = LoRa.read();
     
     // Validate packet length
     if (packet.length > sizeof(packet.payload)) {
         TDECK_LOG_E("LoRa packet too large: %d bytes", packet.length);
         return LoRaStatus::ERROR;
     }
     
     // Read payload
     for (uint8_t i = 0; i < packet.length; i++) {
         if (LoRa.available()) {
             packet.payload[i] = LoRa.read();
         } else {
             TDECK_LOG_E("LoRa packet payload incomplete");
             return LoRaStatus::ERROR;
         }
     }
     
     // Log reception
     TDECK_LOG_I("LoRa packet received: ID=%d, Dst=0x%02X, Src=0x%02X, Len=%d, RSSI=%d, SNR=%.1f", 
                 packet.id, packet.destination, packet.source, packet.length, 
                 getPacketRssi(), getPacketSnr());
     
     // Check if packet is for us or broadcast
     if (packet.destination != _nodeAddress && packet.destination != 0xFF) {
         TDECK_LOG_I("LoRa packet not for us (dest=0x%02X, our=0x%02X)", 
                    packet.destination, _nodeAddress);
         return LoRaStatus::OK; // Still return OK, but caller should check destination
     }
     
     // Send ACK if required
     if (packet.flags & LORA_FLAG_RELIABLE && !(packet.flags & LORA_FLAG_ACK)) {
         TDECK_LOG_I("Sending ACK for packet ID %d", packet.id);
         
         // Prepare ACK packet
         LoRaPacket ackPacket;
         ackPacket.destination = packet.source;
         ackPacket.source = _nodeAddress;
         ackPacket.id = packet.id;
         ackPacket.flags = LORA_FLAG_ACK;
         ackPacket.length = 0;
         
         // Send ACK
         sendPacket(ackPacket, 100);
     }
     
     return LoRaStatus::OK;
 }
 
 void LoRaModule::setTimeout(uint32_t timeout)
 {
     _timeout = timeout;
 }
 
 uint32_t LoRaModule::getTimeout() const
 {
     return _timeout;
 }
 
 void LoRaModule::onReceive(void (*callback)(int))
 {
     if (!_available) {
         return;
     }
     
     LoRa.onReceive(callback);
 }
 
 void LoRaModule::onTransmit(void (*callback)(void))
 {
     if (!_available) {
         return;
     }
     
     LoRa.onTxDone(callback);
 }
 
 bool LoRaModule::initLoRa()
 {
     // Try to initialize the LoRa module
     if (!LoRa.begin(_frequency)) {
         return false;
     }
     
     // Configure LoRa parameters
     LoRa.setSignalBandwidth(_bandwidth);
     LoRa.setSpreadingFactor(_spreadingFactor);
     LoRa.setCodingRate4(_codingRate);
     LoRa.setSyncWord(_syncWord);
     LoRa.setTxPower(_txPower);
     
     // Enable CRC checking
     LoRa.enableCrc();
     
     return true;
 }
 
 LoRaStatus LoRaModule::waitForTxReady(uint32_t timeout)
 {
     // Wait for transmission to complete
     unsigned long startTime = millis();
     
     // The LoRa library doesn't provide a direct way to check if transmission
     // is complete. We'll use a delay based on the airtime calculation.
     
     // Estimate airtime based on spreading factor and bandwidth
     // Higher spreading factor and lower bandwidth mean longer airtime
     uint32_t symbolDuration = 1000 * (1 << _spreadingFactor) / (_bandwidth / 1000);
     uint32_t preambleDuration = symbolDuration * 8; // 8 symbols for preamble
     uint32_t headerDuration = symbolDuration * 4.25; // Header is typically 4.25 symbols
     uint32_t payloadSymbols = 8 + (20 * 8) / (4 * _spreadingFactor); // Approximate for typical packet
     uint32_t payloadDuration = symbolDuration * payloadSymbols;
     
     uint32_t estimatedAirtime = preambleDuration + headerDuration + payloadDuration;
     uint32_t waitTime = min(estimatedAirtime + 50, timeout); // Add 50ms safety margin
     
     delay(waitTime);
     
     // Check if we timed out
     if (millis() - startTime >= timeout) {
         TDECK_LOG_W("LoRa transmission timeout");
         return LoRaStatus::TIMEOUT;
     }
     
     return LoRaStatus::OK;
 }