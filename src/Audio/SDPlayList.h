// SDPlaylist.h
#ifndef SD_PLAYLIST_H
#define SD_PLAYLIST_H

#include <WString.h>
#include <vector> 
#include <string>

class SDPlaylist {
public:
    SDPlaylist();
    
    bool begin();
    const char* getTrack(int index);
    int getTrackCount();
    void printPlaylist();
    std::vector<std::string> getPlaylist();

private:
    static const int MAX_TRACKS = 100;
    char* _playlist[MAX_TRACKS];
    int _trackCount;
    
    void scanForMusic(const char* dirname);
    
    bool isAudioFile(const char* filename);
};

#endif