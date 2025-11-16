/*
 * SensorUtils.cpp
 * Implementation of utility functions for sensor data processing
 */

#include "SensorUtils.h"

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

bool isValidReading(float pm1, float pm25, float pm4, float pm10,
                   float humidity, float temperature, float voc, float nox) {
    // Check for NaN values
    if (isnan(pm1) || isnan(pm25) || isnan(pm4) || isnan(pm10) || 
        isnan(humidity) || isnan(temperature) || isnan(voc) || isnan(nox)) {
        return false;
    }
    
    // Sanity check for extreme values
    if (voc < 0 || voc > 500 || nox < 0 || nox > 500) {
        return false;
    }
    
    return true;
}

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
