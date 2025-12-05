/**
 * @file NetworkManager.h
 * @brief WiFi connection management with automatic reconnection
 * 
 * Handles WiFi connectivity, reconnection logic, and connection monitoring
 * for ESP32-based IoT devices.
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

class NetworkManager {
private:
    const char* ssid;
    const char* password;
    int reconnectAttempts;
    unsigned long lastReconnectAttempt;
    static const unsigned long RECONNECT_INTERVAL = 5000; // 5 seconds between attempts
    static const int MAX_RECONNECT_ATTEMPTS = 3;
    
public:
    /**
     * @brief Construct a new Network Manager object
     * 
     * @param ssid WiFi network SSID
     * @param password WiFi network password
     */
    NetworkManager(const char* ssid, const char* password);
    
    /**
     * @brief Initial WiFi connection with blocking wait
     * 
     * @return true if connection successful
     * @return false if connection failed
     */
    bool connect();
    
    /**
     * @brief Check if WiFi is currently connected
     * 
     * @return true if connected
     * @return false if disconnected
     */
    bool isConnected();
    
    /**
     * @brief Attempt to reconnect to WiFi (non-blocking)
     * 
     * @return true if reconnection successful or in progress
     * @return false if max attempts reached
     */
    bool reconnect();
    
    /**
     * @brief Get the current IP address
     * 
     * @return String IP address or "Not Connected"
     */
    String getIP();
    
    /**
     * @brief Reset reconnection attempt counter
     */
    void resetReconnectAttempts();
};

#endif // NETWORK_MANAGER_H
