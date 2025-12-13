// ============================================================================
// AudioPlayer.cpp
// ============================================================================
#include "AudioPlayer.h"
#include <Arduino.h>

// --- Global Audio Callbacks ---
static AudioPlayer* audioPlayerInstance = nullptr;

void audio_info(const char *info) {
  Serial.print("INFO: "); Serial.println(info);
}

void audio_id3data(const char *info) {
  Serial.print("ID3: "); Serial.println(info);
}

void audio_eof_mp3(const char *info) {
  Serial.print("EOF: "); Serial.println(info);
  if (audioPlayerInstance) {
      audioPlayerInstance->hasFinished(true);
  }
}

void audio_showstation(const char *info) {
  Serial.print("Station: "); Serial.println(info);
}

void audio_showstreamtitle(const char *info) {
  Serial.print("Stream Title: "); Serial.println(info);
}

// --- AudioPlayer Class Implementation ---

AudioPlayer::AudioPlayer() {
    audioPlayerInstance = this;
}

bool AudioPlayer::begin() {
    Serial.println("\n=== Initializing Audio System ===");
    
    // Initialize the DAC controller
    if (!dacController.begin()) {
        Serial.println("ERROR: Failed to initialize DAC controller!");
        return false;
    }
    
    // Set up the I2S pin configuration for ESP32-S3-Audio library
    audio.setPinout(BCLK_PIN, LRCK_PIN, DOUT_PIN);
    audio.setVolume(10); // Start with higher volume (0-21)
    
    Serial.println("=== Audio System Ready ===\n");

    return true;
}

void AudioPlayer::playTrack(const char* path) {
    if (audio.isRunning()) {
        audio.stopSong();
    }
    
    _finished = false;

    Serial.printf("▶ Playing: %s\n", path);
    audio.connecttoFS(SD, path);
}

void AudioPlayer::loop() {
    audio.loop();
}

bool AudioPlayer::isRunning() {
    return audio.isRunning();
}

bool AudioPlayer::hasFinished() {
    return _finished;
}

void AudioPlayer::hasFinished(bool finished) {
    _finished = finished;
}

void AudioPlayer::setVolume(uint8_t volume) {
    if (volume > 100) volume = 100;  // Clamp to 0-100
    
    // Convert 0-100 to 0-21
    uint8_t mappedVolume = map(volume, 0, 100, 0, 21);
    
    audio.setVolume(mappedVolume);
    Serial.printf("Volume set to: %d%% (%d/21)\n", volume, mappedVolume);
}

void AudioPlayer::playCurrentTrack() {

}

void AudioPlayer::pauseCurrentTrack() {

}

void AudioPlayer::nextTrack() {

}

void AudioPlayer::previousTrack() {

}