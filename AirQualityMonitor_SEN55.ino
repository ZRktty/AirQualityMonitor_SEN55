
/*
 * SEN55 with ThingSpeak Data Logging + OTA Updates
 * Sends sensor data to ThingSpeak cloud
 * Supports Over-The-Air firmware updates
 */

#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoOTA.h>
#include "config.h"  // Local configuration file (not in Git)
#include "DataAveraging.h"
#include "SensorUtils.h"

// WiFi credentials (from config.h)
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ThingSpeak settings (from config.h)
const char* writeAPIKey = THINGSPEAK_API_KEY;
unsigned long channelID = THINGSPEAK_CHANNEL_ID;

// OTA settings (from config.h)
const char* otaHostname = OTA_HOSTNAME;
const char* otaPassword = OTA_PASSWORD;

// I2C pins for ESP32-S3
#define I2C_SDA 1
#define I2C_SCL 2

// Send interval (ThingSpeak free tier: minimum 15 seconds between updates)
const unsigned long SEND_INTERVAL = 20000; // 20 seconds to be safe
const unsigned long SENSOR_READ_INTERVAL = 1000; // Read sensor every second

unsigned long lastSendTime = 0;
unsigned long lastSensorReadTime = 0;

// OTA update flag
bool otaInProgress = false;

SensirionI2CSen5x sen5x;
DataAveraging dataAveraging;

void setupOTA() {
    Serial.println("Configuring OTA updates...");
    
    ArduinoOTA.setHostname(otaHostname);
    ArduinoOTA.setPassword(otaPassword);
    
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("\n🔄 OTA: Starting update (" + type + ")");
        Serial.println("⚠️  Do not power off!");
        
        // Stop sensor measurements during OTA
        otaInProgress = true;
        uint16_t error = sen5x.stopMeasurement();
        if (error) {
            Serial.println("⚠️  Could not stop sensor measurements");
        } else {
            Serial.println("✓ Sensor measurements stopped");
        }
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("\n✓ OTA: Update complete!");
        Serial.println("Rebooting...");
        // No need to restart measurements - device will reboot
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static unsigned int lastPercent = 0;
        unsigned int percent = 0;
        if (total != 0) {
            percent = (progress * 100) / total;
        }
        if (percent != lastPercent && percent % 10 == 0) {
            Serial.printf("OTA Progress: %u%%\n", percent);
            lastPercent = percent;
        }
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("\n✗ OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
        
        // Restart measurements on OTA error
        otaInProgress = false;
        uint16_t restartError = sen5x.startMeasurement();
        if (restartError) {
            Serial.println("⚠️  Could not restart sensor measurements");
        } else {
            Serial.println("✓ Sensor measurements restarted");
        }
    });
    
    ArduinoOTA.begin();
    
    Serial.println("✓ OTA Ready!");
    Serial.print("  Hostname: ");
    Serial.println(otaHostname);
    Serial.print("  IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("  Upload via: Tools → Port → Network ports");
    Serial.println();
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

     // Setup OTA
    setupOTA();

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

bool sendToThingSpeak(float pm1, float pm25, float pm4, float pm10, 
                      float humidity, float temperature, float voc, float nox) {
    // Validate data before uploading
    if (!isValidReading(pm1, pm25, pm4, pm10, humidity, temperature, voc, nox)) {
        Serial.println("✗ Skipping upload - invalid data detected");
        return false;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("✗ WiFi Disconnected! Reconnecting...");
        WiFi.begin(ssid, password);
        return false;
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
    bool success = false;
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        response.trim();
        
        Serial.print("HTTP Response Code: ");
        Serial.println(httpResponseCode);
        Serial.print("Server Response: ");
        Serial.println(response);
        
        if (httpResponseCode == 200 && response.toInt() > 0) {
            Serial.println("✓ SUCCESS! Entry #" + response);
            success = true;
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
    return success;
}

void loop() {
     // Handle OTA updates (must be called frequently)
    ArduinoOTA.handle();
    
    // Skip sensor operations during OTA update
    if (otaInProgress) {
        delay(10); // Very short delay, allows OTA.handle() to be called ~100x per second
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Non-blocking sensor reading - only read every SENSOR_READ_INTERVAL
    if (currentTime - lastSensorReadTime < SENSOR_READ_INTERVAL) {
        delay(10); // Short delay to prevent tight loop, but still responsive to OTA
        return;
    }
    
    lastSensorReadTime = currentTime;

    // Read sensor values
    float pm1, pm25, pm4, pm10;
    float humidity, temperature, voc, nox;

    uint16_t error = sen5x.readMeasuredValues(
        pm1, pm25, pm4, pm10, humidity, temperature, voc, nox);

    if (error) {
        char errorMessage[256];
        errorToString(error, errorMessage, 256);
        Serial.print("ERROR reading sensor: ");
        Serial.println(errorMessage);
        Serial.println("⚠️  Check wiring! Skipping this reading...");
        return;
    }

    // Validate sensor data
    if (!isValidReading(pm1, pm25, pm4, pm10, humidity, temperature, voc, nox)) {
        Serial.println("⚠️  WARNING: Invalid sensor data detected");
        Serial.println("   Check I2C connections and power supply!");
        delay(1000); // Brief delay before retrying
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
    
    // Show averaging progress
    Serial.print(" | Avg:");
    Serial.print(dataAveraging.getCount());
    Serial.print("/");
    Serial.print(AVERAGING_SAMPLES);
    
    // Show countdown to next upload
    if (currentTime >= lastSendTime) {
        unsigned long timeSinceLast = currentTime - lastSendTime;
        if (timeSinceLast < SEND_INTERVAL) {
            unsigned long timeUntilSend = (SEND_INTERVAL - timeSinceLast) / 1000;
            Serial.print(" | Upload in ");
            Serial.print(timeUntilSend);
            Serial.print("s");
        }
    }
    Serial.println();

    // Send to ThingSpeak periodically with averaged data
    if (currentTime - lastSendTime >= SEND_INTERVAL) {
        if (dataAveraging.hasEnoughSamples()) {
            float avgPm1, avgPm25, avgPm4, avgPm10;
            float avgHumidity, avgTemperature, avgVoc, avgNox;
            
            dataAveraging.getAveraged(avgPm1, avgPm25, avgPm4, avgPm10,
                                     avgHumidity, avgTemperature, avgVoc, avgNox);
            
            Serial.println();
            Serial.print("📊 Uploading averaged data (");
            Serial.print(AVERAGING_SAMPLES);
            Serial.println(" samples)");
            
            bool uploadSuccess = sendToThingSpeak(avgPm1, avgPm25, avgPm4, avgPm10,
                                                  avgHumidity, avgTemperature, avgVoc, avgNox);
            
            if (uploadSuccess) {
                dataAveraging.reset(); // Only reset on successful upload
            } else {
                Serial.println("⚠️  Data preserved for retry on next interval");
            }
            
            lastSendTime = currentTime;
        } else {
            Serial.println();
            Serial.print("⏳ Collecting more samples (");
            Serial.print(dataAveraging.getCount());
            Serial.print("/");
            Serial.print(AVERAGING_SAMPLES);
            Serial.println(") before upload...");
            lastSendTime = currentTime; // Update to prevent spam on next iteration
        }
    }
    
    // Add reading to running average after upload check
    dataAveraging.addReading(pm1, pm25, pm4, pm10, humidity, temperature, voc, nox);
}