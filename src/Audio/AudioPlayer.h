// ============================================================================
// AudioPlayer.h (UPDATED)
// ============================================================================
#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <Arduino.h>
#include <Audio.h>
#include <SD.h>
#include "DACController.h"
#include "SDPlaylist.h"
#include <vector> 
#include <string>
#include <algorithm>

class AudioPlayer {
public:
    AudioPlayer();
    SDPlaylist _playlist; 

    bool begin();
    void play();
    void pause();
    void playNext();
    void playPrevious();
    void loop();
    bool isRunning();
    bool hasFinished();
    void playTrack(int index);
    void setVolume(uint8_t volume);
    void hasFinished(bool finished);

    int getCurrentTrackIndex() const { return _currentTrackIndex; }
    std::vector<std::string> getPlaylist();
    String getCurrentStateJSON();

private:
    Audio audio;
    DACController dacController; 

    int _currentTrackIndex = 0;
    bool _finished = false;
    uint32_t _pausePosition = 0;
    int _currentVolume = 10;
    
    void _startPlayback();
    void _advanceTrack(int direction);
        
    void _startTrack(const char* path);
};

// Global callback functions for Audio library
void audio_info(const char *info);
void audio_id3data(const char *info);
void audio_eof_mp3(const char *info);
void audio_showstation(const char *info);
void audio_showstreamtitle(const char *info);

#endif 