// SDPlaylist.h
#ifndef SD_PLAYLIST_H
#define SD_PLAYLIST_H

#include <WString.h>

class SDPlaylist {
public:
    SDPlaylist();
    
    // Initialize SD card and scan for music files
    bool begin();
    
    // Get specific track path by index
    const char* getTrack(int index);
    
    // Get number of tracks
    int getTrackCount();
    
    // Print all tracks to serial
    void printPlaylist();

private:
    static const int MAX_TRACKS = 100;
    char* _playlist[MAX_TRACKS];
    int _trackCount;
    
    // Scan directory for audio files
    void scanForMusic(const char* dirname);
    
    // Check if file is audio
    bool isAudioFile(const char* filename);
};

#endif // SD_PLAYLIST_H