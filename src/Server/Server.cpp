#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncEventSource.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "Audio/AudioPlayer.h"
#include "Server.h"
#include "Index.h"
#include "Script.h"

// TODO: Include your audioPlayer class header here
// #include "audioPlayer.h"

// WiFi credentials
const char* ssid = "howyoudoing";
const char* password = "123456789";

// mDNS hostname - access via http://audioplayer.local
const char* mdnsName = "SantaBox";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

AsyncEventSource events("/events");

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

    // Optional: Log when a client connects to the SSE stream
    events.onConnect([](AsyncEventSourceClient *client){
      if(client->lastId()){
        Serial.printf("SSE Client reconnected! Last ID: %u\n", client->lastId());
      }
    
      client->send(playerPtr->getCurrentStateJSON().c_str(), "audio_state"); 
    });

    server.addHandler(&events);
    
    // Serve the main HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = String(index_html);
        html.replace("{{SCRIPT_CONTENT}}", script_js);
        request->send_P(200, "text/html", html.c_str());
    });
    
    // API: Set volume
    server.on("/api/volume", HTTP_POST, [](AsyncWebServerRequest *request) {
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

        events.send(playerPtr->getCurrentStateJSON().c_str(), "audio_state");
    });

    server.on("/api/control", HTTP_POST, [](AsyncWebServerRequest *request){
        if (!request->hasParam("action", true)) {
            request->send(400, "application/json", "{\"error\":\"Missing action parameter\"}");
            return;
        }
        
        if (playerPtr == nullptr) {
            request->send(500, "application/json", "{\"error\":\"Player not initialized\"}");
            return;
        }
        
        String action = request->getParam("action", true)->value();
        
        if (action == "play") {
            playerPtr->play();
            request->send(200, "application/json", "{\"status\":\"ok\",\"action\":\"play\"}");
        } 
        else if (action == "pause") {
            playerPtr->pause();
            request->send(200, "application/json", "{\"status\":\"ok\",\"action\":\"pause\"}");
        } 
        else if (action == "next") {
            playerPtr->playNext();
            request->send(200, "application/json", "{\"status\":\"ok\",\"action\":\"next\"}");
        } 
        else if (action == "previous") {
            playerPtr->playPrevious();
            request->send(200, "application/json", "{\"status\":\"ok\",\"action\":\"previous\"}");
        } 
        else {
            request->send(400, "application/json", "{\"error\":\"Invalid action\"}");
        }

        events.send(playerPtr->getCurrentStateJSON().c_str(), "audio_state");
    });

    server.on("/api/playlist", HTTP_GET, [](AsyncWebServerRequest *request){
        if (playerPtr == nullptr) {
            request->send(500, "application/json", "{\"error\":\"Player not initialized\"}");
            return;
        }

        // 1. Get the playlist data from the AudioPlayer (which calls SDPlaylist::getPlaylist())
        std::vector<std::string> titles = playerPtr->getPlaylist();

        // 2. Estimate the required size for the JSON document.
        //    We need space for:
        //    - JSON_OBJECT_SIZE(1): The outer "playlist" object.
        //    - JSON_ARRAY_SIZE(titles.size()): The array container.
        //    - The combined length of all track titles (approx 30 bytes per title).
        const size_t JSON_CAPACITY = JSON_OBJECT_SIZE(1) + 
                                    JSON_ARRAY_SIZE(titles.size()) + 
                                    (titles.size() * 30); // ~30 bytes/string
      
    
        // // Check if we can allocate the memory. If not, return an error.
        // we are not checking this this could cause a memory overflow.
        // if (JSON_CAPACITY > ARDUINOJSON_DEFAULT_NESTING_LIMIT) {
        //     Serial.println("Error: JSON capacity estimate too high.");
        //     request->send(500, "application/json", "{\"error\":\"Not enough memory for playlist JSON\"}");
        //     return;
        // }
        
        DynamicJsonDocument doc(JSON_CAPACITY);
        
        // 3. Create the JSON array where the track titles will go.
        JsonArray jsonPlaylist = doc.createNestedArray("playlist");
        
        // 4. Iterate over the C++ vector and populate the JSON array.
        for (const std::string& title : titles) {
            // Use the JsonArray::add() method to insert the title.
            // ArduinoJson automatically handles the conversion from std::string to JSON string.
            jsonPlaylist.add(title);
        }
        
        // 5. Serialize the JSON document to a String buffer.
        String responseJson;
        serializeJson(doc, responseJson);
        
        // 6. Send the response back to the client.
        request->send(200, "application/json", responseJson);
        
        Serial.printf("API: /api/playlist responded with %d tracks.\n", titles.size());
    });

    server.on("/api/selectTrack", HTTP_POST, [](AsyncWebServerRequest *request){
        if (!request->hasParam("index", true)) {
            request->send(400, "application/json", "{\"error\":\"Missing index parameter\"}");
            return;
        }
        
        if (playerPtr == nullptr) {
            request->send(500, "application/json", "{\"error\":\"Player not initialized\"}");
            return;
        }

        int index = request->getParam("index", true)->value().toInt();
        playerPtr->playTrack(index);

        //String responseJson = "{\"status\":\"ok\",\"selected_index\":" + String(index) + "}";
        request->send(200, "application/json", "");

        events.send(playerPtr->getCurrentStateJSON().c_str(), "audio_state");
     });

    
    // Handle 404
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
    
    // Start server
    server.begin();
    Serial.println("HTTP server started");
}

