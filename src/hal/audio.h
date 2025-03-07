/**
 * @file audio.h
 * @brief Audio driver for T-Deck hardware
 * 
 * This file provides the hardware abstraction layer for audio functionality
 * on the LilyGO T-Deck device, including speaker and microphone control.
 */

 #ifndef TDECK_AUDIO_H
 #define TDECK_AUDIO_H
 
 #include <Arduino.h>
 #include "../config.h"
 
 // Audio hardware pins
 #define TDECK_I2S_SCK       42  // I2S clock (BCLK)
 #define TDECK_I2S_WS        41  // I2S word select (LRCLK)
 #define TDECK_I2S_SD_OUT    40  // I2S data out (speaker)
 #define TDECK_I2S_SD_IN     39  // I2S data in (microphone)
 
 // Audio configuration
 #define TDECK_AUDIO_SAMPLE_RATE     16000  // Sample rate in Hz
 #define TDECK_AUDIO_BITS_PER_SAMPLE 16     // Bits per sample
 #define TDECK_AUDIO_BUFFER_SIZE     512    // DMA buffer size in bytes
 
 // Speaker volume control
 #define TDECK_SPEAKER_ENABLE_PIN    1      // Speaker enable pin
 #define TDECK_SPEAKER_MAX_VOLUME    20     // Maximum volume level
 #define TDECK_SPEAKER_DEFAULT_VOLUME 10    // Default volume level
 
 /**
  * @class TDeckAudio
  * @brief Audio driver for T-Deck hardware
  * 
  * This class provides methods to control the audio hardware on the T-Deck,
  * including speaker output and microphone input.
  */
 class TDeckAudio {
 public:
     /**
      * @brief Initialize the audio hardware
      * 
      * Sets up the I2S interface and audio hardware
      * 
      * @return true if successful, false otherwise
      */
     static bool init();
 
     /**
      * @brief Deinitialize the audio hardware
      * 
      * Releases resources used by the audio hardware
      */
     static void deinit();
 
     /**
      * @brief Set speaker volume
      * 
      * @param volume Volume level (0-20)
      */
     static void setVolume(uint8_t volume);
 
     /**
      * @brief Get current speaker volume
      * 
      * @return Current volume level (0-20)
      */
     static uint8_t getVolume();
 
     /**
      * @brief Enable/disable speaker
      * 
      * @param enable true to enable, false to disable
      */
     static void enableSpeaker(bool enable);
 
     /**
      * @brief Check if speaker is enabled
      * 
      * @return true if enabled, false if disabled
      */
     static bool isSpeakerEnabled();
 
     /**
      * @brief Enable/disable microphone
      * 
      * @param enable true to enable, false to disable
      */
     static void enableMicrophone(bool enable);
 
     /**
      * @brief Check if microphone is enabled
      * 
      * @return true if enabled, false if disabled
      */
     static bool isMicrophoneEnabled();
 
     /**
      * @brief Play a tone through the speaker
      * 
      * @param frequency Tone frequency in Hz
      * @param duration Duration in milliseconds
      */
     static void playTone(uint16_t frequency, uint32_t duration);
 
     /**
      * @brief Play a sound file from the filesystem
      * 
      * @param filePath Path to the sound file
      * @return true if playback started successfully, false otherwise
      */
     static bool playSound(const char* filePath);
 
     /**
      * @brief Stop current sound playback
      */
     static void stopSound();
 
     /**
      * @brief Record audio from the microphone
      * 
      * @param filePath Path to save the recorded audio
      * @param duration Recording duration in milliseconds (0 for manual stop)
      * @return true if recording started successfully, false otherwise
      */
     static bool recordAudio(const char* filePath, uint32_t duration = 0);
 
     /**
      * @brief Stop current audio recording
      */
     static void stopRecording();
 
     /**
      * @brief Check if audio is currently playing
      * 
      * @return true if audio is playing, false otherwise
      */
     static bool isPlaying();
 
     /**
      * @brief Check if audio is currently being recorded
      * 
      * @return true if recording, false otherwise
      */
     static bool isRecording();
 
 private:
     static uint8_t _volume;              // Current volume level
     static bool _speakerEnabled;         // Speaker enabled flag
     static bool _microphoneEnabled;      // Microphone enabled flag
     static bool _isPlaying;              // Audio playback flag
     static bool _isRecording;            // Audio recording flag
     static TaskHandle_t _playbackTask;   // Playback task handle
     static TaskHandle_t _recordTask;     // Recording task handle
 
     /**
      * @brief Audio playback task
      * 
      * @param parameter Task parameters
      */
     static void playbackTaskFunc(void* parameter);
 
     /**
      * @brief Audio recording task
      * 
      * @param parameter Task parameters
      */
     static void recordTaskFunc(void* parameter);
 };
 
 #endif // TDECK_AUDIO_H