#pragma once
#include <Adafruit_NeoPixel.h>

class StatusLed {
public:
    StatusLed(int pin, int numPixels = 1);
    void begin();
    void update(float pm25);

private:
    Adafruit_NeoPixel pixels;
};
