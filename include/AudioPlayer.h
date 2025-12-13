// ============================================================================
// AudioPlayer.h
// ============================================================================
#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <Audio.h>
#include <FS.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_TLV320DAC3100.h>

class AudioPlayer {
public:
    // I2S Pins for Adafruit TLV320DAC3100 (avoiding SD card pins: 21, 39, 42, 45)
    static constexpr int BCLK_PIN = 9;   // Bit Clock (D9 in example)
    static constexpr int LRCK_PIN = 10;   // Word Select (WSEL) (D10 in example)
    static constexpr int DOUT_PIN = 11;  // Data Out to DAC (DIN on DAC) (D11 in example)
    
    // Hardware reset pin for DAC
    static constexpr int DAC_RST_PIN = 7; // Hardware reset (D7 in example)
    
    // I2C Pins (STEMMA QT connector on Metro ESP32-S3)
    static constexpr int I2C_SDA = 47;   // STEMMA QT SDA
    static constexpr int I2C_SCL = 48;   // STEMMA QT SCL
    
    // TLV320DAC3100 I2C address (handled by Adafruit library)
    static constexpr uint8_t DAC_I2C_ADDR = 0x18;

    AudioPlayer();

    bool begin();
    void playTrack(const char* path);
    void loop();
    bool isRunning();
    bool hasFinished();
    void hasFinished(bool finished);
    void setVolume(uint8_t volume);
    void playCurrentTrack();
    void pauseCurrentTrack();
    void nextTrack();
    void previousTrack();

private:
    Audio audio;
    Adafruit_TLV320DAC3100 codec;  // Codec object
    bool _finished = false;
    
    bool initDAC();
};

// Global callback functions for Audio library
void audio_info(const char *info);
void audio_id3data(const char *info);
void audio_eof_mp3(const char *info);
void audio_showstation(const char *info);
void audio_showstreamtitle(const char *info);

#endif // AUDIOPLAYER_H