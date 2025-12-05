/**
 * @file NetworkManager.cpp
 * @brief Implementation of WiFi connection management
 */

#include "NetworkManager.h"

NetworkManager::NetworkManager(const char* ssid, const char* password) 
    : ssid(ssid), password(password), reconnectAttempts(0), lastReconnectAttempt(0) {
}

bool NetworkManager::connect() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int dots = 0;
    int timeout = 0;
    const int MAX_TIMEOUT = 60; // 30 seconds (60 * 500ms)
    
    while (WiFi.status() != WL_CONNECTED && timeout < MAX_TIMEOUT) {
        delay(500);
        Serial.print(".");
        dots++;
        timeout++;
        
        if (dots % 40 == 0) {
            Serial.println();
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println();
        Serial.println("✓ WiFi connected!");
        Serial.print("  IP address: ");
        Serial.println(WiFi.localIP());
        reconnectAttempts = 0;
        return true;
    } else {
        Serial.println();
        Serial.println("✗ WiFi connection failed!");
        return false;
    }
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::reconnect() {
    unsigned long currentTime = millis();
    
    // Check if already connected
    if (isConnected()) {
        reconnectAttempts = 0;
        return true;
    }
    
    // Check if we've exceeded max attempts
    if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) {
        Serial.println("✗ Max reconnection attempts reached. Please check WiFi credentials.");
        return false;
    }
    
    // Non-blocking reconnection with interval
    if (currentTime - lastReconnectAttempt >= RECONNECT_INTERVAL) {
        reconnectAttempts++;
        lastReconnectAttempt = currentTime;
        
        Serial.print("⟳ WiFi reconnection attempt ");
        Serial.print(reconnectAttempts);
        Serial.print("/");
        Serial.println(MAX_RECONNECT_ATTEMPTS);
        
        WiFi.disconnect();
        delay(100);
        WiFi.begin(ssid, password);
        
        // Give it a moment to attempt connection
        delay(3000);
        
        if (isConnected()) {
            Serial.println("✓ WiFi reconnected successfully!");
            Serial.print("  IP address: ");
            Serial.println(WiFi.localIP());
            reconnectAttempts = 0;
            return true;
        }
    }
    
    return false;
}

String NetworkManager::getIP() {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

void NetworkManager::resetReconnectAttempts() {
    reconnectAttempts = 0;
}
