// ============================================================================
// AudioPlayer.cpp
// ============================================================================
#include "AudioPlayer.h"
#include <Arduino.h>
#include <ArduinoJson.h>

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

AudioPlayer::AudioPlayer() {
    audioPlayerInstance = this;
}

bool AudioPlayer::begin() {
    Serial.println("\n=== Initializing Audio System ===");
    
    if (!dacController.begin()) {
        Serial.println("ERROR: Failed to initialize DAC controller!");
        return false;
    }
    
    Serial.println("\n--- Initializing SD Card and Playlist ---");
    if (!_playlist.begin()) { 
        Serial.println("FATAL: Failed to initialize SD card or find music!");
        return false;
    }
    
    _playlist.printPlaylist();
    Serial.printf("Total tracks found: %d\n", _playlist.getTrackCount());
    
    audio.setPinout(BCLK_PIN, LRCK_PIN, DOUT_PIN);
    audio.setVolume(_currentVolume);
    
    Serial.println("=== Audio System Ready ===\n");

    return true;
}

void AudioPlayer::playTrack(int index) {
    if (_playlist.getTrackCount() == 0) {
        Serial.println("ERROR: Cannot set track, playlist is empty.");
        return;
    }

    if (audio.isRunning()) {
        audio.stopSong();
    }
    
    _pausePosition = 0; 

    int trackCount = _playlist.getTrackCount();

    _currentTrackIndex = (index % trackCount + trackCount) % trackCount;

    Serial.printf("Switching track to index %d.\n", _currentTrackIndex);
    
   
    _startPlayback();
}

void AudioPlayer::_startTrack(const char* path) {
    if (audio.isRunning()) {
        audio.stopSong();
    }
    
    _finished = false;
    _pausePosition = 0;

    Serial.printf("▶ Playing: %s\n", path);
    audio.connecttoFS(SD, path);
}

void AudioPlayer::_startPlayback() {
    if (_playlist.getTrackCount() == 0) {
        Serial.println("ERROR: Cannot play track, playlist is empty.");
        return;
    }
    
    int trackCount = _playlist.getTrackCount();
    _currentTrackIndex = (_currentTrackIndex % trackCount + trackCount) % trackCount;
    
    const char* path = _playlist.getTrack(_currentTrackIndex);
    _startTrack(path);
}

void AudioPlayer::play() {
    if (_playlist.getTrackCount() == 0) return;
    
    if (_pausePosition > 0) {
        
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
        Serial.println("Audio starting playback from the beginning.");
        _startPlayback(); 
    } else {
        audio.pauseResume();
        Serial.println("Audio Resumed.");
    }
}

// PUBLIC - Pauses playback and stores position
void AudioPlayer::pause() {
    if (audio.isRunning()) {
        
        _pausePosition = audio.getFilePos(); 
        
        audio.stopSong();       
        
        // StopSong will call audio_eof_mp3, We need to make sure to flip it back.
        _finished = false;

        Serial.printf("Audio Paused. Position stored at byte %u.\n", _pausePosition);
    }
}

void AudioPlayer::_advanceTrack(int direction) {
    if (_playlist.getTrackCount() == 0) return;

    _currentTrackIndex += direction;
    
    int trackCount = _playlist.getTrackCount();
    _currentTrackIndex = (_currentTrackIndex % trackCount + trackCount) % trackCount;
    
    _startPlayback();
}

void AudioPlayer::playNext() {
    _advanceTrack(1);
}

void AudioPlayer::playPrevious() {
    _advanceTrack(-1);
}

void AudioPlayer::loop() {
    audio.loop();

    // Auto-advance logic
    if (hasFinished()) {
        Serial.println("Current track finished. Auto-advancing to next track.");
        hasFinished(false);
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
    _currentVolume = volume;
    Serial.printf("Volume set to: %d%% (%d/21)\n", volume, mappedVolume);
}

std::vector<std::string> AudioPlayer::getPlaylist() {
    std::vector<std::string> rawPaths = _playlist.getPlaylist();
    
    std::vector<std::string> formattedTitles;

    for (std::string path : rawPaths) {
        
        size_t lastSeparator = path.find_last_of('/');
        if (lastSeparator != std::string::npos) {
            path = path.substr(lastSeparator + 1);
        }
        
        size_t lastDot = path.find_last_of('.');
        
        if (lastDot != std::string::npos && lastDot > 0) {
            path = path.substr(0, lastDot);
        }

        formattedTitles.push_back(path);
    }
    
    return formattedTitles;
}

String AudioPlayer::getCurrentStateJSON() {
    StaticJsonDocument<512> doc;

    doc["trackIndex"] = _currentTrackIndex;
    doc["isPlaying"] = isRunning();
    doc["volume"] = _currentVolume;

    String json;
    serializeJson(doc, json);
    return json;
}