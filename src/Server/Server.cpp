#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "AudioPlayer.h"
#include "Server.h"
#include "Index.h"
#include "Script.h"

// TODO: Include your audioPlayer class header here
// #include "audioPlayer.h"

// WiFi credentials
const char* ssid = "howyoudoing";
const char* password = "123456789";

// mDNS hostname - access via http://audioplayer.local
const char* mdnsName = "Music";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


// Store pointer to audioPlayer
AudioPlayer* playerPtr = nullptr;

void initServer(AudioPlayer* player) {
    playerPtr = player;  // Save the player pointer
    
    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());
    
    // Start mDNS
    if (!MDNS.begin(mdnsName)) {
        Serial.println("Error starting mDNS");
        return;
    }
    Serial.println("mDNS started. Access at http://" + String(mdnsName) + ".local");
    
    // Serve the main HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = String(index_html);
        html.replace("{{SCRIPT_CONTENT}}", script_js);
        request->send_P(200, "text/html", html.c_str());
    });
    
    // API: Set volume
    server.on("/api/volume", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("volume", true)) {
            int vol = request->getParam("volume", true)->value().toInt();
            
            // Call your audioPlayer setVolume method
            if (playerPtr != nullptr) {
                playerPtr->setVolume(vol);
            }
            
            request->send(200, "application/json", "{\"status\":\"ok\",\"volume\":" + String(vol) + "}");
        } else {
            request->send(400, "application/json", "{\"error\":\"Missing volume parameter\"}");
        }
    });
    
    // Handle 404
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
    
    // Start server
    server.begin();
    Serial.println("HTTP server started");
}