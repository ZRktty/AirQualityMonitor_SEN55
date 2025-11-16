// config.example.h - Configuration template
// Copy this to config.h and fill in your actual values

#ifndef CONFIG_H
#define CONFIG_H

// WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ThingSpeak settings
const char* THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_API_KEY";
const unsigned long THINGSPEAK_CHANNEL_ID = 0;

// OTA settings
const char* OTA_HOSTNAME = "SEN55-AirQuality";
const char* OTA_PASSWORD = "YOUR_OTA_PASSWORD";

#endif