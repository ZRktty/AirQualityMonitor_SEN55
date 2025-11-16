/*
 * DataAveraging.h
 * Handles running average calculation for sensor readings
 */

#ifndef DATA_AVERAGING_H
#define DATA_AVERAGING_H

#include <Arduino.h>

// Data averaging settings
const int AVERAGING_SAMPLES = 10;  // Average 10 readings before upload

struct SensorData {
    float pm1;
    float pm25;
    float pm4;
    float pm10;
    float humidity;
    float temperature;
    float voc;
    float nox;
    int count;
};

class DataAveraging {
private:
    SensorData data;
    
public:
    DataAveraging();
    
    void addReading(float pm1, float pm25, float pm4, float pm10,
                   float humidity, float temperature, float voc, float nox);
    
    void getAveraged(float &pm1, float &pm25, float &pm4, float &pm10,
                    float &humidity, float &temperature, float &voc, float &nox);
    
    void reset();
    
    int getCount() const;
    
    bool hasEnoughSamples() const;
};

#endif
