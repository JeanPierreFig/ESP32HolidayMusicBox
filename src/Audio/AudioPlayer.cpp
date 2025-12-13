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
    
    // 1. Initialize the DAC controller
    if (!dacController.begin()) {
        Serial.println("ERROR: Failed to initialize DAC controller!");
        return false;
    }
    
    // 2. Initialize the internal SDPlaylist
    Serial.println("\n--- Initializing SD Card and Playlist ---");
    if (!_playlist.begin()) { 
        Serial.println("FATAL: Failed to initialize SD card or find music!");
        return false;
    }
    
    _playlist.printPlaylist();
    Serial.printf("Total tracks found: %d\n", _playlist.getTrackCount());
    
    // 3. Set up the I2S pin configuration for Audio library
    audio.setPinout(BCLK_PIN, LRCK_PIN, DOUT_PIN);
    audio.setVolume(10);
    
    Serial.println("=== Audio System Ready ===\n");

    return true;
}

// Internal function to start the track based on its path
void AudioPlayer::_startTrack(const char* path) {
    if (audio.isRunning()) {
        audio.stopSong();
    }
    
    _finished = false;
    _pausePosition = 0; // Reset position whenever a new track starts

    Serial.printf("▶ Playing: %s\n", path);
    audio.connecttoFS(SD, path);
}

// Internal function to start the track at the current index (from beginning)
void AudioPlayer::_startPlayback() {
    if (_playlist.getTrackCount() == 0) {
        Serial.println("ERROR: Cannot play track, playlist is empty.");
        return;
    }
    
    // Ensure index is valid
    int trackCount = _playlist.getTrackCount();
    _currentTrackIndex = (_currentTrackIndex % trackCount + trackCount) % trackCount;
    
    const char* path = _playlist.getTrack(_currentTrackIndex);
    _startTrack(path);
}

void AudioPlayer::play() {
    if (_playlist.getTrackCount() == 0) return;
    
    if (_pausePosition > 0) {
        // RESUME LOGIC (Seek)
        
        // Ensure we stop any previous playback attempt
        if (audio.isRunning()) {
            audio.stopSong(); 
        }

        const char* path = _playlist.getTrack(_currentTrackIndex);
        Serial.printf("▶ Resuming: %s at byte position %u\n", path, _pausePosition);
        
        // Reconnect the file stream and seek to the saved position
        // The third parameter is the starting byte position (fpos_t)
        audio.connecttoFS(SD, path, _pausePosition); 
        
        _pausePosition = 0; // Reset stored position after resuming
        
    } else if (!audio.isRunning()) {
        // Start playback from the beginning of the current track
        Serial.println("Audio starting playback from the beginning.");
        _startPlayback(); 
    } else {
        // Audio is running but paused (use internal library resume)
        audio.pauseResume();
        Serial.println("Audio Resumed.");
    }
}

// PUBLIC - Pauses playback and stores position
void AudioPlayer::pause() {
    if (audio.isRunning()) {
        
        // FIX: Use getFilePos() to get the current byte offset
        _pausePosition = audio.getFilePos(); 
        
        // 2. Stop the song to cleanly pause the playback
        audio.stopSong();       
        
        // StopSong will call audio_eof_mp3, We need to make sure to flip it back.
        _finished = false;

        Serial.printf("Audio Paused. Position stored at byte %u.\n", _pausePosition);
    }
}

// Internal helper for changing tracks
void AudioPlayer::_advanceTrack(int direction) {
    if (_playlist.getTrackCount() == 0) return;

    _currentTrackIndex += direction;
    
    // Wrap the index correctly
    int trackCount = _playlist.getTrackCount();
    _currentTrackIndex = (_currentTrackIndex % trackCount + trackCount) % trackCount;
    
    _startPlayback();
}

// PUBLIC - Play the next track
void AudioPlayer::playNext() {
    _advanceTrack(1);
}

// PUBLIC - Play the previous track
void AudioPlayer::playPrevious() {
    _advanceTrack(-1);
}

void AudioPlayer::loop() {
    audio.loop();

    // Auto-advance logic
    if (hasFinished()) {
        Serial.println("Current track finished. Auto-advancing to next track.");
        hasFinished(false); // Reset flag immediately
        playNext();
    }
}

// STATUS & CONTROL SETTERS/GETTERS
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
    if (volume > 100) volume = 100;
    
    uint8_t mappedVolume = map(volume, 0, 100, 0, 21);
    
    audio.setVolume(mappedVolume);
    Serial.printf("Volume set to: %d%% (%d/21)\n", volume, mappedVolume);
}
