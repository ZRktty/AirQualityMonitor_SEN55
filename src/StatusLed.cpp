#include "StatusLed.h"

StatusLed::StatusLed(int pin, int numPixels) 
    : pixels(numPixels, pin, NEO_GRB + NEO_KHZ800) {
}

void StatusLed::begin() {
    pixels.begin();
    pixels.setBrightness(10);
    pixels.clear();
    pixels.show();
}

void StatusLed::update(float pm25) {
    uint32_t color;
    
    if (pm25 <= 15.0) {
        color = pixels.Color(0, 255, 0); // Green (Good)
    } else if (pm25 <= 35.0) {
        color = pixels.Color(255, 190, 0); // Yellow (Moderate)
    } else if (pm25 <= 55.0) {
        color = pixels.Color(255, 30, 0); // Deep Orange (Unhealthy for Sensitive)
    } else {
        color = pixels.Color(255, 0, 0); // Red (Unhealthy)
    }
    
    pixels.setPixelColor(0, color);
    pixels.show();
}
