// SDPlaylist.cpp
#include "SDPlaylist.h"
#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// Assuming MAX_TRACKS is defined in SDPlaylist.h

SDPlaylist::SDPlaylist() : _trackCount(0) {
    for (int i = 0; i < MAX_TRACKS; i++) {
        _playlist[i] = nullptr;
    }
}

bool SDPlaylist::begin() {
    Serial.println("Initializing SD Playlist...");
    
    // Metro ESP32-S3 SD card pins
    SPI.begin(39, 21, 42, 45);  // SCK, MISO, MOSI, CS
    
    if (!SD.begin(45)) {
        Serial.println("SD Card failed!");
        return false;
    }
    
    Serial.println("SD Card OK - Scanning for music...");
    scanForMusic("/");
    
    Serial.printf("Found %d tracks\n", _trackCount);
    return true;
}

void SDPlaylist::scanForMusic(const char* dirname) {
    File root = SD.open(dirname);
    if (!root || !root.isDirectory()) return;

    File file = root.openNextFile();
    while (file && _trackCount < MAX_TRACKS) {

        if (file.isDirectory()) {
            // Only enter folders named "Music"
            if (strcmp(file.name(), "Music") == 0 ||
                strcmp(file.name(), "/Music") == 0) {
                scanForMusic(file.path());
            }

        } else if (isAudioFile(file.name())) {
            // Only add valid audio files
            _playlist[_trackCount] = strdup(file.path());
            _trackCount++;
        }

        file = root.openNextFile();
    }
}

bool SDPlaylist::isAudioFile(const char* filename) {
    String name = String(filename);
    name.toLowerCase();

    // --- NEW LOGIC ADDED HERE ---
    // 1. Ignore hidden files (starts with .) 
    //    e.g., .DS_Store (macOS)
    if (name.startsWith(".")) {
        return false;
    }
    
    // 2. Ignore macOS resource/metadata files (starts with ._)
    //    e.g., ._Merry_Christmas_to_you.mp3
    if (name.startsWith("._")) {
        return false;
    }
    // ----------------------------

    // 3. Check for valid audio extensions
    return name.endsWith(".mp3") || name.endsWith(".wav");
}

const char* SDPlaylist::getTrack(int index) {
    if (index >= 0 && index < _trackCount) {
        return _playlist[index];
    }
    return "";
}

int SDPlaylist::getTrackCount() {
    return _trackCount;
}

void SDPlaylist::printPlaylist() {
    Serial.println("\n=== Playlist ===");
    for (int i = 0; i < _trackCount; i++) {
        Serial.printf("%d: %s\n", i, _playlist[i]);
    }
}