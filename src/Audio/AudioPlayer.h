// ============================================================================
// AudioPlayer.h
// ============================================================================
#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <Arduino.h>
#include <Audio.h>
#include <SD.h>
#include "DACController.h"

class AudioPlayer {
public:
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
    DACController dacController; 
    bool _finished = false;
};


// Global callback functions for Audio library
void audio_info(const char *info);
void audio_id3data(const char *info);
void audio_eof_mp3(const char *info);
void audio_showstation(const char *info);
void audio_showstreamtitle(const char *info);

#endif // AUDIOPLAYER_H