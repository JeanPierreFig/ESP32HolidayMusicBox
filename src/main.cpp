// #include <Arduino.h>
// #include <Adafruit_NeoPixel.h>

// #define NEOPIXEL_PIN 46
// #define NUM_PIXELS 1

// Adafruit_NeoPixel pixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// void setup() {
//   pixel.begin();
//   pixel.setBrightness(50);
// }

// void loop() {
//   // Red
//   pixel.setPixelColor(0, pixel.Color(255, 0, 0));
//   pixel.show();
//   delay(1000); // Wait 1 second
  
//   // Green
//   pixel.setPixelColor(0, pixel.Color(0, 255, 0));
//   pixel.show();
//   delay(1000); // Wait 1 second
// }

// ============================================================================
// main.cpp (UPDATED)
// ============================================================================
#include "Audio/AudioPlayer.h"
#include "Server/Server.h"

// REMOVED: #include "Audio/SDPlaylist.h"

// Global Objects
// REMOVED: SDPlaylist playlist;
AudioPlayer audioPlayer; 

// REMOVED: int currentTrack = 1;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("--- ESP32 Jukebox System Starting ---");
    
    // 1. The AudioPlayer::begin() now handles all DAC and SD/Playlist initialization.
    if (!audioPlayer.begin()) {
        Serial.println("FATAL: System initialization failed.");
        // The individual error messages (SD fail, DAC fail) are now printed inside AudioPlayer::begin()
        return;
    }
    
    // 2. The AudioPlayer is ready to go.
    initServer(&audioPlayer);

    // 3. Start Playing the First Track
    // Note: We no longer need to check getTrackCount() here, 
    // as AudioPlayer::begin() handles the fatal check, and play() handles the start.
    audioPlayer.play();
}

void loop() {
    // CRITICAL: This MUST be called continuously. 
    // It handles audio processing AND auto-advancing to the next track.
    audioPlayer.loop();
}