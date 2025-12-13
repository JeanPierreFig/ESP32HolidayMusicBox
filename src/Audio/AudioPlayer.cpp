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
    
    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    delay(100);
    
    // Initialize codec using Adafruit library with hardware reset pin
    Serial.println("Initializing codec with hardware reset...");
    if (!codec.begin()) {
        Serial.println("ERROR: Failed to initialize TLV320DAC3100!");
        Serial.println("Check wiring:");
        Serial.printf("  RST pin: GPIO %d to DAC RST\n", DAC_RST_PIN);
        Serial.printf("  I2C SDA: GPIO %d to DAC SDA\n", I2C_SDA);
        Serial.printf("  I2C SCL: GPIO %d to DAC SCL\n", I2C_SCL);
        return false;
    }
    
    Serial.println("✓ TLV320DAC3100 initialized with hardware reset");
    
    // Initialize the DAC with proper configuration
    if (!initDAC()) {
        Serial.println("ERROR: Failed to configure DAC!");
        return false;
    }
    
    // Set up the I2S pin configuration for ESP32-S3-Audio library
    audio.setPinout(BCLK_PIN, LRCK_PIN, DOUT_PIN);
    audio.setVolume(8); // Start with higher volume (0-21)
    
    Serial.printf("✓ I2S Pins: BCLK=%d, WSEL=%d, DIN=%d\n", BCLK_PIN, LRCK_PIN, DOUT_PIN);
    Serial.printf("✓ I2C Pins: SDA=%d, SCL=%d\n", I2C_SDA, I2C_SCL);
    Serial.printf("✓ Reset Pin: GPIO %d\n", DAC_RST_PIN);
    Serial.println("=== Audio System Ready ===\n");

    return true;
}

bool AudioPlayer::initDAC() {
    Serial.println("Configuring TLV320DAC3100...");
    
    // Reset codec (hardware reset already done in begin())
    codec.reset();
    delay(100);
    
    Serial.println("  [1/6] Configuring codec interface...");
    // Step 1: Set codec interface to I2S with 16-bit data
    if (!codec.setCodecInterface(TLV320DAC3100_FORMAT_I2S, TLV320DAC3100_DATA_LEN_16)) {
        Serial.println("Failed to configure codec interface!");
        return false;
    }
    
    Serial.println("  [2/6] Configuring codec clocks...");
    // Step 2: Configure clock - using PLL with BCLK as input
    if (!codec.setCodecClockInput(TLV320DAC3100_CODEC_CLKIN_PLL) ||
        !codec.setPLLClockInput(TLV320DAC3100_PLL_CLKIN_BCLK)) {
        Serial.println("Failed to configure codec clocks!");
        return false;
    }
    
    Serial.println("  [3/6] Configuring PLL...");
    // Step 3: Set up PLL - these values work well for audio playback
    if (!codec.setPLLValues(1, 1, 8, 0)) {
        Serial.println("Failed to configure PLL values!");
        return false;
    }
    
    Serial.println("  [4/6] Configuring DAC dividers...");
    // Step 4: Configure DAC dividers
    if (!codec.setNDAC(true, 8) ||
        !codec.setMDAC(true, 2) ||
        !codec.setDOSR(128)) {
        Serial.println("Failed to configure DAC dividers!");
        return false;
    }
    
    // Step 5: Power up PLL
    if (!codec.powerPLL(true)) {
        Serial.println("Failed to power up PLL!");
        return false;
    }
    
    Serial.println("  [5/6] Configuring DAC path...");
    // Step 6: Configure DAC path - power up both left and right DACs
    if (!codec.setDACDataPath(true, true, 
                             TLV320_DAC_PATH_NORMAL,
                             TLV320_DAC_PATH_NORMAL,
                             TLV320_VOLUME_STEP_1SAMPLE)) {
        Serial.println("Failed to configure DAC data path!");
        return false;
    }
    
    // Step 7: Route DAC output to mixer
    if (!codec.configureAnalogInputs(TLV320_DAC_ROUTE_MIXER, // Left DAC to mixer
                                     TLV320_DAC_ROUTE_MIXER, // Right DAC to mixer
                                     false, false, false,    // No AIN routing
                                     false)) {               // No HPL->HPR
        Serial.println("Failed to configure DAC routing!");
        return false;
    }
    
    Serial.println("  [6/6] Setting volume and powering up speaker...");
    // Step 8: Unmute DAC and set volume
    if (!codec.setDACVolumeControl(
            false, false, TLV320_VOL_INDEPENDENT) || // Unmute both channels
        !codec.setChannelVolume(false, 18) ||        // Left DAC 0dB
        !codec.setChannelVolume(true, 18)) {         // Right DAC 0dB
        Serial.println("Failed to configure DAC volume control!");
        return false;
    }
    
    // Set channel volumes
    if (!codec.setChannelVolume(false, 5.0) ||
        !codec.setChannelVolume(true, 5.0)) {
        Serial.println("Failed to set DAC channel volumes!");
        return false;
    }
    
    // Enable and configure speaker output
    if (!codec.enableSpeaker(true) ||                // Enable speaker amp
        !codec.configureSPK_PGA(TLV320_SPK_GAIN_6DB, // Set gain to 6dB
                                true) ||             // Unmute
        !codec.setSPKVolume(true, 0)) {              // Enable and set volume to 0dB
        Serial.println("Failed to configure speaker output!");
        return false;
    }
    
    delay(100);
    
    Serial.println("✓ DAC Configuration Complete!");
    Serial.println("\nHardware Checklist (Based on Example Wiring):");
    Serial.println("  □ Board 5V to DAC VIN (power the DAC with 5V, NOT 3.3V!)");
    Serial.println("  □ Board GND to DAC GND");
    Serial.println("  □ Board SCL to DAC SCL (I2C clock)");
    Serial.println("  □ Board SDA to DAC SDA (I2C data)");
    Serial.printf("  □ Board GPIO %d to DAC BCK (I2S bit clock)\n", BCLK_PIN);
    Serial.printf("  □ Board GPIO %d to DAC WSEL (I2S word select)\n", LRCK_PIN);
    Serial.printf("  □ Board GPIO %d to DAC DIN (I2S data)\n", DOUT_PIN);
    Serial.printf("  □ Board GPIO %d to DAC RST (hardware reset)\n", DAC_RST_PIN);
    Serial.println("  □ DAC JST-PH to speaker (4-8Ω speaker)\n");
    
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

     // Check if the current song has finished playing (using the feature we added)
    if (audioPlayer.hasFinished()) {
        Serial.println("Current track finished. Advancing to next track.");

        // 1. Increment track index, wrapping around to 0 if necessary
        currentTrack++;
        if (currentTrack >= playlist.getTrackCount()) {
            currentTrack = 0; // Loop back to the start of the playlist
        }

        // 2. Start the next track
        audioPlayer.playTrack(playlist.getTrack(currentTrack));
    }
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