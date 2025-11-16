/*
 * DataAveraging.cpp
 * Implementation of running average calculation for sensor readings
 */

#include "DataAveraging.h"

DataAveraging::DataAveraging() {
    reset();
}

void DataAveraging::addReading(float pm1, float pm25, float pm4, float pm10,
                               float humidity, float temperature, float voc, float nox) {
    data.pm1 += pm1;
    data.pm25 += pm25;
    data.pm4 += pm4;
    data.pm10 += pm10;
    data.humidity += humidity;
    data.temperature += temperature;
    data.voc += voc;
    data.nox += nox;
    data.count++;
}

void DataAveraging::getAveraged(float &pm1, float &pm25, float &pm4, float &pm10,
                                float &humidity, float &temperature, float &voc, float &nox) {
    if (data.count == 0) return; // Prevent division by zero
    
    pm1 = data.pm1 / data.count;
    pm25 = data.pm25 / data.count;
    pm4 = data.pm4 / data.count;
    pm10 = data.pm10 / data.count;
    humidity = data.humidity / data.count;
    temperature = data.temperature / data.count;
    voc = data.voc / data.count;
    nox = data.nox / data.count;
    
    reset(); // Reset for next averaging cycle
}

void DataAveraging::reset() {
    data = {0, 0, 0, 0, 0, 0, 0, 0, 0};
}

int DataAveraging::getCount() const {
    return data.count;
}

bool DataAveraging::hasEnoughSamples() const {
    return data.count >= AVERAGING_SAMPLES;
}
