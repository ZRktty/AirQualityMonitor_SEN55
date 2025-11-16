/*
 * SensorUtils.h
 * Utility functions for sensor data processing
 */

#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#include <Arduino.h>

// Function to get PM2.5 air quality status
void getPM25Quality(float pm25, String &quality, String &colorIcon);

// Validate sensor reading values
bool isValidReading(float pm1, float pm25, float pm4, float pm10,
                   float humidity, float temperature, float voc, float nox);

// Wait for sensor to stabilize with countdown
void waitForSensorStabilization();

#endif
