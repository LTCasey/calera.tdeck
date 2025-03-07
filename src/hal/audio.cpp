/**
 * @file audio.cpp
 * @brief Implementation of Audio driver for T-Deck hardware
 */

 #include "audio.h"
 #include <driver/i2s.h>
 #include "../config.h"
 
 // Static member initialization
 uint8_t TDeckAudio::_volume = TDECK_SPEAKER_DEFAULT_VOLUME;
 bool TDeckAudio::_speakerEnabled = false;
 bool TDeckAudio::_microphoneEnabled = false;
 bool TDeckAudio::_isPlaying = false;
 bool TDeckAudio::_isRecording = false;
 TaskHandle_t TDeckAudio::_playbackTask = nullptr;
 TaskHandle_t TDeckAudio::_recordTask = nullptr;
 
 // I2S configuration for speaker output (I2S_NUM_0 channel)
 static const i2s_config_t i2s_speaker_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
     .sample_rate = TDECK_AUDIO_SAMPLE_RATE,
     .bits_per_sample = (i2s_bits_per_sample_t)TDECK_AUDIO_BITS_PER_SAMPLE,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = I2S_COMM_FORMAT_STAND_I2S,
     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
     .dma_buf_count = 8,
     .dma_buf_len = TDECK_AUDIO_BUFFER_SIZE,
     .use_apll = false,
     .tx_desc_auto_clear = true,
     .fixed_mclk = 0
 };
 
 // I2S configuration for microphone input (I2S_NUM_1 channel)
 static const i2s_config_t i2s_microphone_config = {
     .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
     .sample_rate = TDECK_AUDIO_SAMPLE_RATE,
     .bits_per_sample = (i2s_bits_per_sample_t)TDECK_AUDIO_BITS_PER_SAMPLE,
     .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
     .communication_format = I2S_COMM_FORMAT_STAND_I2S,
     .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
     .dma_buf_count = 8,
     .dma_buf_len = TDECK_AUDIO_BUFFER_SIZE,
     .use_apll = false,
     .tx_desc_auto_clear = false,
     .fixed_mclk = 0
 };
 
 // I2S pin configuration for speaker output
 static const i2s_pin_config_t i2s_speaker_pins = {
     .bck_io_num = TDECK_I2S_SCK,
     .ws_io_num = TDECK_I2S_WS,
     .data_out_num = TDECK_I2S_SD_OUT,
     .data_in_num = I2S_PIN_NO_CHANGE
 };
 
 // I2S pin configuration for microphone input
 static const i2s_pin_config_t i2s_microphone_pins = {
     .bck_io_num = TDECK_I2S_SCK,
     .ws_io_num = TDECK_I2S_WS,
     .data_out_num = I2S_PIN_NO_CHANGE,
     .data_in_num = TDECK_I2S_SD_IN
 };
 
 bool TDeckAudio::init() {
     TDECK_LOG_I("Initializing audio hardware");
 
     // Initialize speaker enable pin
     pinMode(TDECK_SPEAKER_ENABLE_PIN, OUTPUT);
     digitalWrite(TDECK_SPEAKER_ENABLE_PIN, LOW); // Speaker disabled by default
 
     // Initialize I2S driver for speaker output
     esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_speaker_config, 0, NULL);
     if (err != ESP_OK) {
         TDECK_LOG_E("Failed to install I2S driver for speaker: %d", err);
         return false;
     }
 
     err = i2s_set_pin(I2S_NUM_0, &i2s_speaker_pins);
     if (err != ESP_OK) {
         TDECK_LOG_E("Failed to set I2S pins for speaker: %d", err);
         i2s_driver_uninstall(I2S_NUM_0);
         return false;
     }
 
     // Initialize I2S driver for microphone input
     err = i2s_driver_install(I2S_NUM_1, &i2s_microphone_config, 0, NULL);
     if (err != ESP_OK) {
         TDECK_LOG_E("Failed to install I2S driver for microphone: %d", err);
         i2s_driver_uninstall(I2S_NUM_0);
         return false;
     }
 
     err = i2s_set_pin(I2S_NUM_1, &i2s_microphone_pins);
     if (err != ESP_OK) {
         TDECK_LOG_E("Failed to set I2S pins for microphone: %d", err);
         i2s_driver_uninstall(I2S_NUM_0);
         i2s_driver_uninstall(I2S_NUM_1);
         return false;
     }
 
     // Set default volume
     setVolume(TDECK_SPEAKER_DEFAULT_VOLUME);
 
     TDECK_LOG_I("Audio hardware initialized");
     return true;
 }
 
 void TDeckAudio::deinit() {
     TDECK_LOG_I("Deinitializing audio hardware");
 
     // Stop any ongoing playback or recording
     stopSound();
     stopRecording();
 
     // Disable speaker
     enableSpeaker(false);
 
     // Uninstall I2S drivers
     i2s_driver_uninstall(I2S_NUM_0);
     i2s_driver_uninstall(I2S_NUM_1);
 
     TDECK_LOG_I("Audio hardware deinitialized");
 }
 
 void TDeckAudio::setVolume(uint8_t volume) {
     // Clamp volume to valid range
     if (volume > TDECK_SPEAKER_MAX_VOLUME) {
         volume = TDECK_SPEAKER_MAX_VOLUME;
     }
 
     _volume = volume;
     TDECK_LOG_I("Speaker volume set to %d", _volume);
 }
 
 uint8_t TDeckAudio::getVolume() {
     return _volume;
 }
 
 void TDeckAudio::enableSpeaker(bool enable) {
     _speakerEnabled = enable;
     digitalWrite(TDECK_SPEAKER_ENABLE_PIN, enable ? HIGH : LOW);
     TDECK_LOG_I("Speaker %s", enable ? "enabled" : "disabled");
 }
 
 bool TDeckAudio::isSpeakerEnabled() {
     return _speakerEnabled;
 }
 
 void TDeckAudio::enableMicrophone(bool enable) {
     _microphoneEnabled = enable;
     TDECK_LOG_I("Microphone %s", enable ? "enabled" : "disabled");
 }
 
 bool TDeckAudio::isMicrophoneEnabled() {
     return _microphoneEnabled;
 }
 
 void TDeckAudio::playTone(uint16_t frequency, uint32_t duration) {
     if (!_speakerEnabled) {
         enableSpeaker(true);
     }
 
     if (_isPlaying) {
         stopSound();
     }
 
     TDECK_LOG_I("Playing tone: %d Hz for %d ms", frequency, duration);
 
     // Calculate buffer size and allocate memory
     const int sampleRate = TDECK_AUDIO_SAMPLE_RATE;
     const size_t bufferSize = TDECK_AUDIO_BUFFER_SIZE;
     int16_t* buffer = (int16_t*)malloc(bufferSize * sizeof(int16_t));
     
     if (!buffer) {
         TDECK_LOG_E("Failed to allocate memory for tone buffer");
         return;
     }
 
     // Generate sine wave for the tone
     float period = (float)sampleRate / frequency;
     float volume = _volume / (float)TDECK_SPEAKER_MAX_VOLUME;
     
     for (size_t i = 0; i < bufferSize; i += 2) {
         float angle = 2.0f * PI * (i / 2) / period;
         int16_t sample = (int16_t)(sin(angle) * 32767.0f * volume);
         
         // Stereo output (same on both channels)
         buffer[i] = sample;        // Left channel
         buffer[i + 1] = sample;    // Right channel
     }
 
     // Start playback
     _isPlaying = true;
     size_t bytes_written = 0;
     uint32_t startTime = millis();
     
     while (_isPlaying && (millis() - startTime < duration)) {
         i2s_write(I2S_NUM_0, buffer, bufferSize * sizeof(int16_t), &bytes_written, portMAX_DELAY);
         vTaskDelay(1); // Yield to other tasks
     }
 
     _isPlaying = false;
     free(buffer);
 }
 
 bool TDeckAudio::playSound(const char* filePath) {
     if (!_speakerEnabled) {
         enableSpeaker(true);
     }
 
     if (_isPlaying) {
         stopSound();
     }
 
     TDECK_LOG_I("Playing sound file: %s", filePath);
 
     // Open the file
     FILE* file = fopen(filePath, "rb");
     if (!file) {
         TDECK_LOG_E("Failed to open sound file: %s", filePath);
         return false;
     }
 
     // Create a task to handle playback
     struct PlaybackParams {
         const char* path;
         FILE* file;
     };
 
     PlaybackParams* params = new PlaybackParams{filePath, file};
     
     _isPlaying = true;
     xTaskCreatePinnedToCore(
         playbackTaskFunc,          // Function to implement the task
         "audio_playback",          // Name of the task
         TDECK_SYSTEM_TASK_STACK_SIZE, // Stack size in words
         params,                    // Task input parameter
         TDECK_SYSTEM_TASK_PRIORITY,// Priority of the task
         &_playbackTask,            // Task handle
         0                          // Core where the task should run
     );
 
     return true;
 }
 
 void TDeckAudio::stopSound() {
     if (_isPlaying && _playbackTask != nullptr) {
         TDECK_LOG_I("Stopping sound playback");
         _isPlaying = false;
         
         // Wait for task to complete
         vTaskDelay(100 / portTICK_PERIOD_MS);
         
         // Ensure task is deleted
         if (_playbackTask != nullptr) {
             vTaskDelete(_playbackTask);
             _playbackTask = nullptr;
         }
     }
 }
 
 bool TDeckAudio::recordAudio(const char* filePath, uint32_t duration) {
     if (!_microphoneEnabled) {
         enableMicrophone(true);
     }
 
     if (_isRecording) {
         stopRecording();
     }
 
     TDECK_LOG_I("Recording audio to: %s for %d ms", filePath, duration);
 
     // Open the file for writing
     FILE* file = fopen(filePath, "wb");
     if (!file) {
         TDECK_LOG_E("Failed to open file for recording: %s", filePath);
         return false;
     }
 
     // Create a task to handle recording
     struct RecordParams {
         const char* path;
         FILE* file;
         uint32_t duration;
     };
 
     RecordParams* params = new RecordParams{filePath, file, duration};
     
     _isRecording = true;
     xTaskCreatePinnedToCore(
         recordTaskFunc,            // Function to implement the task
         "audio_recording",         // Name of the task
         TDECK_SYSTEM_TASK_STACK_SIZE, // Stack size in words
         params,                    // Task input parameter
         TDECK_SYSTEM_TASK_PRIORITY,// Priority of the task
         &_recordTask,              // Task handle
         0                          // Core where the task should run
     );
 
     return true;
 }
 
 void TDeckAudio::stopRecording() {
     if (_isRecording && _recordTask != nullptr) {
         TDECK_LOG_I("Stopping audio recording");
         _isRecording = false;
         
         // Wait for task to complete
         vTaskDelay(100 / portTICK_PERIOD_MS);
         
         // Ensure task is deleted
         if (_recordTask != nullptr) {
             vTaskDelete(_recordTask);
             _recordTask = nullptr;
         }
     }
 }
 
 bool TDeckAudio::isPlaying() {
     return _isPlaying;
 }
 
 bool TDeckAudio::isRecording() {
     return _isRecording;
 }
 
 // Implementation of the private methods
 
 void TDeckAudio::playbackTaskFunc(void* parameter) {
     struct PlaybackParams {
         const char* path;
         FILE* file;
     };
     
     PlaybackParams* params = (PlaybackParams*)parameter;
     FILE* file = params->file;
     const char* path = params->path;
     
     // Allocate buffer for audio data
     const size_t bufferSize = TDECK_AUDIO_BUFFER_SIZE * sizeof(int16_t);
     uint8_t* buffer = (uint8_t*)malloc(bufferSize);
     
     if (!buffer) {
         TDECK_LOG_E("Failed to allocate memory for playback buffer");
         fclose(file);
         delete params;
         _isPlaying = false;
         vTaskDelete(NULL);
         return;
     }
 
     size_t bytesRead;
     size_t bytesWritten;
     
     // Simple WAV header parsing (skip the first 44 bytes for WAV files)
     if (strstr(path, ".wav") != NULL || strstr(path, ".WAV") != NULL) {
         fseek(file, 44, SEEK_SET);
     }
     
     // Read and play the file
     while (_isPlaying && (bytesRead = fread(buffer, 1, bufferSize, file)) > 0) {
         // Scale volume
         float volumeScale = _volume / (float)TDECK_SPEAKER_MAX_VOLUME;
         int16_t* samples = (int16_t*)buffer;
         for (size_t i = 0; i < bytesRead / sizeof(int16_t); i++) {
             samples[i] = (int16_t)(samples[i] * volumeScale);
         }
         
         i2s_write(I2S_NUM_0, buffer, bytesRead, &bytesWritten, portMAX_DELAY);
         
         if (bytesRead != bytesWritten) {
             TDECK_LOG_W("Bytes read (%d) != bytes written (%d)", bytesRead, bytesWritten);
         }
     }
 
     // Clean up
     free(buffer);
     fclose(file);
     delete params;
     
     TDECK_LOG_I("Sound playback completed");
     _isPlaying = false;
     _playbackTask = nullptr;
     vTaskDelete(NULL);
 }
 
 void TDeckAudio::recordTaskFunc(void* parameter) {
     struct RecordParams {
         const char* path;
         FILE* file;
         uint32_t duration;
     };
     
     RecordParams* params = (RecordParams*)parameter;
     FILE* file = params->file;
     uint32_t duration = params->duration;
     
     // Allocate buffer for audio data
     const size_t bufferSize = TDECK_AUDIO_BUFFER_SIZE * sizeof(int16_t);
     uint8_t* buffer = (uint8_t*)malloc(bufferSize);
     
     if (!buffer) {
         TDECK_LOG_E("Failed to allocate memory for recording buffer");
         fclose(file);
         delete params;
         _isRecording = false;
         vTaskDelete(NULL);
         return;
     }
 
     // Write WAV header
     // Simple 16-bit PCM WAV header
     const uint32_t sampleRate = TDECK_AUDIO_SAMPLE_RATE;
     const uint16_t numChannels = 2; // Stereo
     const uint16_t bitsPerSample = TDECK_AUDIO_BITS_PER_SAMPLE;
     const uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
     const uint16_t blockAlign = numChannels * (bitsPerSample / 8);
     
     uint8_t wavHeader[44] = {
         'R', 'I', 'F', 'F',         // ChunkID
         0, 0, 0, 0,                 // ChunkSize (filled later)
         'W', 'A', 'V', 'E',         // Format
         'f', 'm', 't', ' ',         // Subchunk1ID
         16, 0, 0, 0,                // Subchunk1Size (16 for PCM)
         1, 0,                       // AudioFormat (1 for PCM)
         numChannels, 0,             // NumChannels
         sampleRate & 0xFF, (sampleRate >> 8) & 0xFF, (sampleRate >> 16) & 0xFF, (sampleRate >> 24) & 0xFF, // SampleRate
         byteRate & 0xFF, (byteRate >> 8) & 0xFF, (byteRate >> 16) & 0xFF, (byteRate >> 24) & 0xFF, // ByteRate
         blockAlign & 0xFF, (blockAlign >> 8) & 0xFF, // BlockAlign
         bitsPerSample & 0xFF, (bitsPerSample >> 8) & 0xFF, // BitsPerSample
         'd', 'a', 't', 'a',         // Subchunk2ID
         0, 0, 0, 0                  // Subchunk2Size (filled later)
     };
     
     fwrite(wavHeader, 1, sizeof(wavHeader), file);
     
     // Record audio
     size_t bytesRead;
     uint32_t totalBytesRead = 0;
     uint32_t startTime = millis();
     
     while (_isRecording && (duration == 0 || (millis() - startTime < duration))) {
         i2s_read(I2S_NUM_1, buffer, bufferSize, &bytesRead, portMAX_DELAY);
         
         if (bytesRead > 0) {
             fwrite(buffer, 1, bytesRead, file);
             totalBytesRead += bytesRead;
         }
     }
     
     // Update WAV header with file size information
     uint32_t dataSize = totalBytesRead;
     uint32_t riffSize = dataSize + 36; // 36 + SubChunk2Size
     
     fseek(file, 4, SEEK_SET);
     fwrite(&riffSize, 4, 1, file);
     
     fseek(file, 40, SEEK_SET);
     fwrite(&dataSize, 4, 1, file);
     
     // Clean up
     free(buffer);
     fclose(file);
     delete params;
     
     TDECK_LOG_I("Audio recording completed, %d bytes recorded", totalBytesRead);
     _isRecording = false;
     _recordTask = nullptr;
     vTaskDelete(NULL);
 }