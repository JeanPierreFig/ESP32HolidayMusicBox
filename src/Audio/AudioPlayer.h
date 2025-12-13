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

class AudioPlayer {
public:
    AudioPlayer();

    // The begin function now initializes both DAC and the internal playlist
    bool begin();
    
    // PUBLIC CONTROL METHODS (Cleaned up as requested)
    void play();        // Starts/Resumes playback of the current track
    void pause();       // Pauses the current track
    void playNext();    // Moves to the next track and starts playback
    void playPrevious();// Moves to the previous track and starts playback
    
    // Existing functions
    void loop();
    bool isRunning();
    bool hasFinished();
    
    void setVolume(uint8_t volume);
    void hasFinished(bool finished);

    // Accessors
    int getCurrentTrackIndex() const { return _currentTrackIndex; }

    const SDPlaylist& getPlaylist() const { return _playlist; }
    
private:
    Audio audio;
    DACController dacController; 
    
    // --- TRACK MANAGEMENT MEMBERS ---
    SDPlaylist _playlist;           // <--- NEW: Internal SDPlaylist object
    int _currentTrackIndex = 0;      // Index of the currently playing track
    bool _finished = false;
    uint32_t _pausePosition = 0;
    
    // Internal playback logic
    void _startPlayback();          // Starts the track at the current index
    void _advanceTrack(int direction); // Internal helper for next/previous
        
    // Internal function to start the track at a given path (formerly playTrack)
    void _startTrack(const char* path);
};

// Global callback functions for Audio library (remain the same)
void audio_info(const char *info);
void audio_id3data(const char *info);
void audio_eof_mp3(const char *info);
void audio_showstation(const char *info);
void audio_showstreamtitle(const char *info);

#endif // AUDIOPLAYER_H