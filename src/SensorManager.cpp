/**
 * @file SensorManager.cpp
 * @brief Implementation of SEN55 sensor management
 */

#include "SensorManager.h"
#include "SensorUtils.h"

SensorManager::SensorManager(SensirionI2CSen5x* sen5x) 
    : sensor(sen5x), initialized(false), tempOffset(0.0) {
}

bool SensorManager::begin(uint8_t sda, uint8_t scl, float temperatureOffset) {
    Serial.println("Initializing SEN55 sensor...");
    
    // Initialize I2C
    Serial.println("  Setting up I2C...");
    Wire.begin(sda, scl);
    sensor->begin(Wire);
    
    tempOffset = temperatureOffset;
    uint16_t error;
    char errorMessage[256];
    
    // Reset sensor
    Serial.println("  Resetting sensor...");
    error = sensor->deviceReset();
    if (error) {
        Serial.print("  ✗ ERROR resetting device: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return false;
    }
    Serial.println("  ✓ Sensor reset OK");
    
    delay(1000); // Wait for sensor to stabilize after reset
    
    // Set temperature offset
    error = sensor->setTemperatureOffsetSimple(tempOffset);
    if (error) {
        Serial.print("  ✗ ERROR setting temperature offset: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return false;
    }
    Serial.print("  ✓ Temperature offset set to ");
    Serial.print(tempOffset, 1);
    Serial.println("°C");
    
    // Start measurement
    Serial.println("  Starting measurements...");
    error = sensor->startMeasurement();
    if (error) {
        Serial.print("  ✗ ERROR starting measurement: ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return false;
    }
    Serial.println("  ✓ Measurements started");
    
    initialized = true;
    Serial.println("✓ SEN55 sensor initialized successfully!");
    Serial.println();
    
    return true;
}

bool SensorManager::readData(float &pm1, float &pm25, float &pm4, float &pm10,
                              float &humidity, float &temperature, float &voc, float &nox) {
    if (!initialized) {
        Serial.println("✗ Sensor not initialized");
        return false;
    }
    
    uint16_t error = sensor->readMeasuredValues(pm1, pm25, pm4, pm10, 
                                                 humidity, temperature, voc, nox);
    
    if (error) {
        char errorMessage[256];
        errorToString(error, errorMessage, 256);
        Serial.print("✗ ERROR reading sensor: ");
        Serial.println(errorMessage);
        return false;
    }
    
    return true;
}

bool SensorManager::startMeasurement() {
    if (!initialized) {
        Serial.println("✗ Sensor not initialized");
        return false;
    }
    
    uint16_t error = sensor->startMeasurement();
    if (error) {
        char errorMessage[256];
        errorToString(error, errorMessage, 256);
        Serial.print("✗ ERROR starting measurement: ");
        Serial.println(errorMessage);
        return false;
    }
    
    Serial.println("✓ Sensor measurements started");
    return true;
}

bool SensorManager::stopMeasurement() {
    if (!initialized) {
        Serial.println("✗ Sensor not initialized");
        return false;
    }
    
    uint16_t error = sensor->stopMeasurement();
    if (error) {
        char errorMessage[256];
        errorToString(error, errorMessage, 256);
        Serial.print("✗ ERROR stopping measurement: ");
        Serial.println(errorMessage);
        return false;
    }
    
    Serial.println("✓ Sensor measurements stopped");
    return true;
}

void SensorManager::printInfo() {
    if (!initialized) {
        Serial.println("✗ Sensor not initialized");
        return;
    }
    
    Serial.println("--- Sensor Information ---");
    
    // Read serial number
    unsigned char serialNumber[32];
    uint8_t serialNumberSize = 32;
    uint16_t error = sensor->getSerialNumber(serialNumber, serialNumberSize);
    
    if (!error) {
        Serial.print("  Serial Number: ");
        Serial.println((char*)serialNumber);
    } else {
        Serial.println("  Serial Number: Could not read");
    }
    
    Serial.print("  Temperature Offset: ");
    Serial.print(tempOffset, 1);
    Serial.println("°C");
    Serial.println("--------------------------");
}

bool SensorManager::isInitialized() const {
    return initialized;
}

bool SensorManager::reset() {
    uint16_t error = sensor->deviceReset();
    if (error) {
        char errorMessage[256];
        errorToString(error, errorMessage, 256);
        Serial.print("✗ ERROR resetting device: ");
        Serial.println(errorMessage);
        return false;
    }
    
    Serial.println("✓ Sensor reset successfully");
    delay(1000); // Wait for sensor to stabilize
    return true;
}
