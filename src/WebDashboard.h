/**
 * @file WebDashboard.h
 * @brief Web dashboard for real-time air quality monitoring
 * 
 * Provides a WebSocket-based web interface for viewing sensor data
 * in real-time. Serves HTML/CSS/JS from LittleFS filesystem.
 */

#ifndef WEB_DASHBOARD_H
#define WEB_DASHBOARD_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "SensorManager.h"
#include "DataAveraging.h"

// Historical data settings
#define HISTORY_SIZE 60  // Store last 60 readings (1 minute at 1Hz)

struct SensorReading {
    float pm1;
    float pm25;
    float pm4;
    float pm10;
    float temperature;
    float humidity;
    float voc;
    float nox;
    unsigned long timestamp;
};

class WebDashboard {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    SensorManager* sensorManager;
    DataAveraging* dataAveraging;
    
    // Circular buffer for historical data
    SensorReading history[HISTORY_SIZE];
    uint8_t historyIndex;
    bool historyFull;
    
    // WebSocket handlers
    void onWebSocketEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
                         AwsEventType type, void* arg, uint8_t* data, size_t len);
    
    // JSON generation
    String createCurrentReadingJSON(float pm1, float pm25, float pm4, float pm10,
                                   float temp, float hum, float voc, float nox);
    String createHistoryJSON();
    String createStatusJSON();
    
    // Static wrapper for event callback
    static void onEventWrapper(AsyncWebSocket* server, AsyncWebSocketClient* client,
                              AwsEventType type, void* arg, uint8_t* data, size_t len);
    static WebDashboard* instance;

public:
    /**
     * @brief Construct a new Web Dashboard object
     * 
     * @param sm Pointer to SensorManager instance
     * @param da Pointer to DataAveraging instance
     */
    WebDashboard(SensorManager* sm, DataAveraging* da);
    
    /**
     * @brief Initialize web server and mount LittleFS
     * 
     * @return true if initialization successful
     * @return false if initialization failed
     */
    bool begin();
    
    /**
     * @brief Handle WebSocket updates - call in main loop
     * 
     * @param pm1 PM1.0 concentration (µg/m³)
     * @param pm25 PM2.5 concentration (µg/m³)
     * @param pm4 PM4.0 concentration (µg/m³)
     * @param pm10 PM10 concentration (µg/m³)
     * @param temperature Temperature (°C)
     * @param humidity Relative humidity (%)
     * @param voc VOC index
     * @param nox NOx index
     */
    void handle(float pm1, float pm25, float pm4, float pm10,
               float temperature, float humidity, float voc, float nox);
    
    /**
     * @brief Add reading to historical buffer
     * 
     * @param pm1 PM1.0 concentration
     * @param pm25 PM2.5 concentration
     * @param pm4 PM4.0 concentration
     * @param pm10 PM10 concentration
     * @param temperature Temperature
     * @param humidity Humidity
     * @param voc VOC index
     * @param nox NOx index
     */
    void addToHistory(float pm1, float pm25, float pm4, float pm10,
                     float temperature, float humidity, float voc, float nox);
    
    /**
     * @brief Get number of connected WebSocket clients
     * 
     * @return uint32_t Number of clients
     */
    uint32_t getClientCount() const;
};

#endif
