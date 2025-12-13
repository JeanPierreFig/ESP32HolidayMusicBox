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

#include "Audio/SDPlaylist.h"
#include "Audio/AudioPlayer.h"
#include "Server/Server.h"

// Global Objects
SDPlaylist playlist;
AudioPlayer audioPlayer; // <-- 2. Create an instance of the AudioPlayer

int currentTrack = 1; // Track the currently playing song index

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("--- ESP32 Jukebox System Starting ---");
    
    // 1. Initialize and scan SD card for music
    if (!playlist.begin()) {
        Serial.println("FATAL: Failed to initialize SD card or find music!");
        return;
    }
    
    // Print all tracks found
    playlist.printPlaylist();
    Serial.printf("\nTotal tracks found: %d\n", playlist.getTrackCount());

    // 2. Initialize the Audio Player (I2S DAC)
    if (!audioPlayer.begin()) {
        Serial.println("FATAL: Audio Player initialization failed.");
        return;
    }

    initServer(&audioPlayer);

    // 3. Start Playing the First Track
    if (playlist.getTrackCount() > 0) {
        // Get the path for the first track (index 0)
        const char* firstTrackPath = playlist.getTrack(currentTrack); 
        
        // Tell the AudioPlayer to start playing it
        audioPlayer.playTrack(firstTrackPath);
    } else {
        Serial.println("No tracks found to play. System will idle.");
    }
}

void loop() {
    // CRITICAL: This MUST be called continuously to process audio data 
    // from the MP3 decoder and send it to the I2S DAC.
    audioPlayer.loop();

    // Check if the current song has finished playing (using the feature we added)
    if (audioPlayer.hasFinished()) {
        Serial.println("Current track finished. Advancing to next track.");

        // 1. Increment track index, wrapping around to 0 if necessary
        currentTrack++;
        if (currentTrack >= playlist.getTrackCount()) {
            currentTrack = 0; // Loop back to the start of the playlist
        }

        // 2. Start the next track
        audioPlayer.playTrack(playlist.getTrack(currentTrack));
    }
}