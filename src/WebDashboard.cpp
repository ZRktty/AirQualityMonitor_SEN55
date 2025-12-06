/**
 * @file WebDashboard.cpp
 * @brief Implementation of WebDashboard class
 */

#include "WebDashboard.h"

// Static instance pointer for callback
WebDashboard* WebDashboard::instance = nullptr;

WebDashboard::WebDashboard(SensorManager* sm, DataAveraging* da)
    : server(80), ws("/ws"), sensorManager(sm), dataAveraging(da),
      historyIndex(0), historyFull(false) {
    instance = this;
}

bool WebDashboard::begin() {
    // Mount LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("✗ Failed to mount LittleFS");
        return false;
    }
    Serial.println("✓ LittleFS mounted");
    
    // Configure WebSocket
    ws.onEvent(onEventWrapper);
    server.addHandler(&ws);
    
    // Serve static files from LittleFS
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    
    // API endpoint for device reset
    server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest* request) {
        Serial.println("🔄 Device reset requested via web interface");
        request->send(200, "text/plain", "Device resetting...");
        delay(100); // Allow response to be sent
        ESP.restart();
    });
    
    // Handle 404
    server.onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "text/plain", "Not found");
    });
    
    // Start server
    server.begin();
    Serial.println("✓ Web server started on port 80");
    
    return true;
}

void WebDashboard::handle(float pm1, float pm25, float pm4, float pm10,
                         float temperature, float humidity, float voc, float nox) {
    // Add to history buffer
    addToHistory(pm1, pm25, pm4, pm10, temperature, humidity, voc, nox);
    
    // Broadcast to all connected clients
    if (ws.count() > 0) {
        String json = createCurrentReadingJSON(pm1, pm25, pm4, pm10, 
                                              temperature, humidity, voc, nox);
        ws.textAll(json);
    }
}

void WebDashboard::addToHistory(float pm1, float pm25, float pm4, float pm10,
                                float temperature, float humidity, float voc, float nox) {
    SensorReading& reading = history[historyIndex];
    reading.pm1 = pm1;
    reading.pm25 = pm25;
    reading.pm4 = pm4;
    reading.pm10 = pm10;
    reading.temperature = temperature;
    reading.humidity = humidity;
    reading.voc = voc;
    reading.nox = nox;
    reading.timestamp = millis();
    
    historyIndex = (historyIndex + 1) % HISTORY_SIZE;
    if (historyIndex == 0) {
        historyFull = true;
    }
}

String WebDashboard::createCurrentReadingJSON(float pm1, float pm25, float pm4, float pm10,
                                             float temp, float hum, float voc, float nox) {
    JsonDocument doc;
    
    doc["type"] = "current";
    doc["pm1"] = round(pm1 * 10) / 10.0;
    doc["pm25"] = round(pm25 * 10) / 10.0;
    doc["pm4"] = round(pm4 * 10) / 10.0;
    doc["pm10"] = round(pm10 * 10) / 10.0;
    doc["temperature"] = round(temp * 10) / 10.0;
    doc["humidity"] = round(hum * 10) / 10.0;
    doc["voc"] = round(voc);
    doc["nox"] = round(nox);
    doc["timestamp"] = millis();
    
    // Calculate air quality category based on PM2.5
    const char* quality;
    if (pm25 < 15) quality = "GOOD";
    else if (pm25 < 35) quality = "MODERATE";
    else if (pm25 < 55) quality = "UNHEALTHY_SENSITIVE";
    else quality = "UNHEALTHY";
    doc["quality"] = quality;
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WebDashboard::createHistoryJSON() {
    JsonDocument doc;
    JsonArray array = doc["history"].to<JsonArray>();
    
    int count = historyFull ? HISTORY_SIZE : historyIndex;
    int startIdx = historyFull ? historyIndex : 0;
    
    for (int i = 0; i < count; i++) {
        int idx = (startIdx + i) % HISTORY_SIZE;
        JsonObject reading = array.add<JsonObject>();
        reading["pm25"] = round(history[idx].pm25 * 10) / 10.0;
        reading["temperature"] = round(history[idx].temperature * 10) / 10.0;
        reading["humidity"] = round(history[idx].humidity * 10) / 10.0;
        reading["voc"] = round(history[idx].voc);
        reading["timestamp"] = history[idx].timestamp;
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WebDashboard::createStatusJSON() {
    JsonDocument doc;
    
    doc["type"] = "status";
    doc["uptime"] = millis() / 1000;
    doc["freeHeap"] = ESP.getFreeHeap();
    doc["heapSize"] = ESP.getHeapSize();
    doc["clients"] = ws.count();
    doc["sensorInitialized"] = sensorManager->isInitialized();
    doc["averageCount"] = dataAveraging->getCount();
    
    String output;
    serializeJson(doc, output);
    return output;
}

void WebDashboard::onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                   AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("🌐 WebSocket client #%u connected from %s\n", 
                     client->id(), client->remoteIP().toString().c_str());
        
        // Send history data on connect
        String history = createHistoryJSON();
        client->text(history);
        
        // Send status
        String status = createStatusJSON();
        client->text(status);
        
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("🌐 WebSocket client #%u disconnected\n", client->id());
        
    } else if (type == WS_EVT_ERROR) {
        Serial.printf("⚠️ WebSocket client #%u error\n", client->id());
        
    } else if (type == WS_EVT_DATA) {
        // Handle incoming messages if needed
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            if (info->opcode == WS_TEXT) {
                data[len] = 0;  // Null terminate
                String message = (char*)data;
                
                // Handle client requests
                if (message == "getHistory") {
                    String history = createHistoryJSON();
                    client->text(history);
                } else if (message == "getStatus") {
                    String status = createStatusJSON();
                    client->text(status);
                }
            }
        }
    }
}

void WebDashboard::onEventWrapper(AsyncWebSocket* server, AsyncWebSocketClient* client,
                                 AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (instance) {
        instance->onWebSocketEvent(server, client, type, arg, data, len);
    }
}

uint32_t WebDashboard::getClientCount() const {
    return ws.count();
}
