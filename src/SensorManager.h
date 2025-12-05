/**
 * @file SensorManager.h
 * @brief SEN55 sensor management and data reading
 * 
 * Handles initialization, measurement control, and data reading
 * for the Sensirion SEN55 environmental sensor.
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>

class SensorManager {
private:
    SensirionI2CSen5x* sensor;
    bool initialized;
    float tempOffset;
    
public:
    /**
     * @brief Construct a new Sensor Manager object
     * 
     * @param sen5x Pointer to SensirionI2CSen5x instance
     */
    SensorManager(SensirionI2CSen5x* sen5x);
    
    /**
     * @brief Initialize sensor with I2C and configure settings
     * 
     * @param sda I2C SDA pin
     * @param scl I2C SCL pin
     * @param temperatureOffset Temperature offset for calibration (default 0.0)
     * @return true if initialization successful
     * @return false if initialization failed
     */
    bool begin(uint8_t sda, uint8_t scl, float temperatureOffset = 0.0);
    
    /**
     * @brief Read all sensor measurements
     * 
     * @param pm1 PM1.0 concentration (µg/m³)
     * @param pm25 PM2.5 concentration (µg/m³)
     * @param pm4 PM4.0 concentration (µg/m³)
     * @param pm10 PM10 concentration (µg/m³)
     * @param humidity Relative humidity (%)
     * @param temperature Temperature (°C)
     * @param voc VOC index
     * @param nox NOx index
     * @return true if read successful
     * @return false if read failed
     */
    bool readData(float &pm1, float &pm25, float &pm4, float &pm10,
                  float &humidity, float &temperature, float &voc, float &nox);
    
    /**
     * @brief Start continuous measurements
     * 
     * @return true if measurement started
     * @return false if failed to start
     */
    bool startMeasurement();
    
    /**
     * @brief Stop continuous measurements
     * 
     * @return true if measurement stopped
     * @return false if failed to stop
     */
    bool stopMeasurement();
    
    /**
     * @brief Print sensor information (serial number, firmware)
     */
    void printInfo();
    
    /**
     * @brief Check if sensor is initialized
     * 
     * @return true if initialized
     * @return false if not initialized
     */
    bool isInitialized() const;
    
    /**
     * @brief Reset the sensor device
     * 
     * @return true if reset successful
     * @return false if reset failed
     */
    bool reset();
};

#endif // SENSOR_MANAGER_H
