#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Streaming.h>

class IOManager
{
private:
    const uint16_t PixelCount = 2; // this example assumes 4 pixels, making it smaller will cause a failure
    const uint8_t PixelPin = 33;   // make sure to set this to the correct pin, ignored for Esp8266
    static const uint8_t A6 = (uint8_t)34U;
    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip;

    RgbColor white;
    RgbColor black;
    bool mLedOn = false;

    void initButtons();
    void initLEDs();

public:
    IOManager();

    void begin();
    void switchLED();

    void loop();
    
};

#endif