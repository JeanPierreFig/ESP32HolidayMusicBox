#ifndef SERVER_H
#define SERVER_H
#include <AsyncEventSource.h>

class AudioPlayer;  // Forward declaration - tells compiler the class exists

extern AsyncEventSource events;

void initServer(AudioPlayer* player);

#endif