/*
 * SEN55 with ThingSpeak Data Logging
 * Sends sensor data to ThingSpeak cloud
 */

#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"  // Local configuration file (not in Git)

// WiFi credentials (from config.h)
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ThingSpeak settings (from config.h)
const char* writeAPIKey = THINGSPEAK_API_KEY;
unsigned long channelID = THINGSPEAK_CHANNEL_ID;

// I2C pins for ESP32-S3
#define I2C_SDA 1
#define I2C_SCL 2

// Send interval (ThingSpeak free tier: minimum 15 seconds between updates)
const unsigned long sendInterval = 20000; // 20 seconds to be safe
unsigned long lastSendTime = 0;

SensirionI2CSen5x sen5x;

// Function to get PM2.5 air quality status
void getPM25Quality(float pm25, String &quality, String &colorIcon) {
    if (pm25 <= 15) {
        quality = "GOOD";
        colorIcon = "🟢";
    } else if (pm25 <= 35) {
        quality = "MODERATE";
        colorIcon = "🟡";
    } else if (pm25 <= 55) {
        quality = "UNHEALTHY (Sensitive)";
        colorIcon = "🟠";
    } else {
        quality = "UNHEALTHY";
        colorIcon = "🔴";
    }
}

// Wait for sensor to stabilize with countdown
void waitForSensorStabilization() {
    Serial.println();
    Serial.println("Waiting for sensor to stabilize...");
    Serial.println("(SEN55 needs ~10 seconds after power-on)");
    
    for (int i = 10; i > 0; i--) {
        Serial.print("Starting in ");
        Serial.print(i);
        Serial.println(" seconds...");
        delay(1000);
    }
    
    Serial.println();
    Serial.println("Sensor ready! Starting measurements...");
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Give serial time to initialize
    
    Serial.println();
    Serial.println("================================");
    Serial.println("=== SEN55 ThingSpeak Logger ===");
    Serial.println("================================");
    Serial.println();

    // Connect to WiFi
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    int dots = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        dots++;
        if (dots % 40 == 0) Serial.println();
    }
    
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("ThingSpeak Channel: ");
    Serial.println(channelID);
    Serial.println();

    // Initialize I2C
    Serial.println("Initializing I2C...");
    Wire.begin(I2C_SDA, I2C_SCL);
    sen5x.begin(Wire);

    uint16_t error;
    char errorMessage[256];
    
    // Reset sensor
    Serial.println("Resetting sensor...");
    error = sen5x.deviceReset();
    if (error) {
        Serial.print("ERROR resetting device: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.println("Sensor reset OK");
    }

    delay(1000);

    // Print sensor info
    Serial.println("Reading sensor info...");
    unsigned char serialNumber[32];
    uint8_t serialNumberSize = 32;
    error = sen5x.getSerialNumber(serialNumber, serialNumberSize);
    if (!error) {
        Serial.print("Serial Number: ");
        Serial.println((char*)serialNumber);
    } else {
        Serial.println("Could not read serial number");
    }

    // Set temperature offset
    float tempOffset = 0.0;
    error = sen5x.setTemperatureOffsetSimple(tempOffset);
    if (error) {
        Serial.print("ERROR setting temperature offset: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.println("Temperature offset set OK");
    }

    // Start measurement
    Serial.println("Starting measurement...");
    error = sen5x.startMeasurement();
    if (error) {
        Serial.print("ERROR starting measurement: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        Serial.println("Measurement started successfully!");
    }
    
    // Wait for sensor to stabilize and provide valid readings
    waitForSensorStabilization();
    
    Serial.println("Waiting 20 seconds before first upload...");
    Serial.println("================================");
    Serial.println();
}

void sendToThingSpeak(float pm1, float pm25, float pm4, float pm10, 
                      float humidity, float temperature, float voc, float nox) {
    // Double-check data validity before uploading
    if (isnan(pm25) || isnan(temperature) || voc < 0 || voc > 500) {
        Serial.println("✗ Skipping upload - invalid data detected");
        return;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("✗ WiFi Disconnected! Reconnecting...");
        WiFi.begin(ssid, password);
        return;
    }
    
    HTTPClient http;
    
    // Build URL with all 8 fields
    String url = "http://api.thingspeak.com/update?api_key=";
    url += writeAPIKey;
    url += "&field1=" + String(pm1, 2);
    url += "&field2=" + String(pm25, 2);
    url += "&field3=" + String(pm4, 2);
    url += "&field4=" + String(pm10, 2);
    url += "&field5=" + String(temperature, 2);
    url += "&field6=" + String((int)voc);
    url += "&field7=" + String((int)nox);
    url += "&field8=" + String(humidity, 2);
    
    Serial.println();
    Serial.println("--- Uploading to ThingSpeak ---");
    
    http.begin(url);
    http.setTimeout(10000);
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        response.trim();
        
        Serial.print("HTTP Response Code: ");
        Serial.println(httpResponseCode);
        Serial.print("Server Response: ");
        Serial.println(response);
        
        if (httpResponseCode == 200 && response.toInt() > 0) {
            Serial.println("✓ SUCCESS! Entry #" + response);
        } else {
            Serial.println("✗ Upload failed - check API key or rate limit");
        }
    } else {
        Serial.print("✗ HTTP Error Code: ");
        Serial.println(httpResponseCode);
        Serial.print("Error: ");
        Serial.println(http.errorToString(httpResponseCode));
    }
    
    Serial.println("-------------------------------");
    Serial.println();
    
    http.end();
}

void loop() {
    uint16_t error;
    char errorMessage[256];

    delay(1000);

    // Read sensor values
    float pm1, pm25, pm4, pm10;
    float humidity, temperature, voc, nox;

    error = sen5x.readMeasuredValues(
        pm1, pm25, pm4, pm10, humidity, temperature, voc, nox);

    if (error) {
        Serial.print("ERROR reading sensor: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        Serial.println("⚠️  Check wiring! Skipping this reading...");
        return;
    }

    // Validate data - skip if any value is NaN or invalid
    if (isnan(pm1) || isnan(pm25) || isnan(pm4) || isnan(pm10) || 
        isnan(humidity) || isnan(temperature) || isnan(voc) || isnan(nox)) {
        Serial.println("⚠️  WARNING: Invalid sensor data (NaN detected)");
        Serial.println("   Check I2C connections and power supply!");
        delay(2000); // Wait a bit before retrying
        return;
    }

    // Additional sanity check for extreme values
    if (voc < 0 || voc > 500 || nox < 0 || nox > 500) {
        Serial.println("⚠️  WARNING: VOC/NOx values out of range");
        Serial.println("   Sensor communication issue - skipping upload");
        return;
    }

    // Get PM2.5 air quality status
    String pm25Quality, pm25Color;
    getPM25Quality(pm25, pm25Quality, pm25Color);

    // Print to serial (formatted nicely with all PM values)
    Serial.print("PM1.0:");
    Serial.print(pm1, 1);
    Serial.print(" | PM2.5:");
    Serial.print(pm25, 1);
    Serial.print(" µg/m³ ");
    Serial.print(pm25Color);
    Serial.print(" [");
    Serial.print(pm25Quality);
    Serial.print("] | PM4:");
    Serial.print(pm4, 1);
    Serial.print(" | PM10:");
    Serial.print(pm10, 1);
    Serial.print(" | Temp:");
    Serial.print(temperature, 1);
    Serial.print("°C | Hum:");
    Serial.print(humidity, 1);
    Serial.print("% | VOC:");
    Serial.print((int)voc);
    Serial.print(" | NOx:");
    Serial.print((int)nox);
    
    // Show countdown to next upload
    unsigned long currentTime = millis();
    if (currentTime >= lastSendTime) { // Prevent underflow
        unsigned long timeSinceLast = currentTime - lastSendTime;
        if (timeSinceLast < sendInterval) {
            unsigned long timeUntilSend = (sendInterval - timeSinceLast) / 1000;
            Serial.print(" | Upload in ");
            Serial.print(timeUntilSend);
            Serial.print("s");
        }
    }
    Serial.println();

    // Send to ThingSpeak periodically
    if (currentTime - lastSendTime >= sendInterval) {
        sendToThingSpeak(pm1, pm25, pm4, pm10, humidity, temperature, voc, nox);
        lastSendTime = currentTime;
    }
}