#ifndef IO_MANAGER_H
#define IO_MANAGER_H

#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Streaming.h>

struct Buttons{
   int button1;
   int button2;
   int button3;
   int button4;
   int button5;
   int button6;
   int button7;
   int button8;
};

struct JacksIn{
   int jack1;
   int jack2;
   int jack3;
   int jack4;
};

struct JacksOut{
   int jack1;
   int jack2;
   int jack3;
   int jack4;
};

struct Pots{
   int pot1;
   int pot2;
   int pot3;
   int pot4;
   int pot5;
   int pot6;
   int pot7;
   int pot8;
};

class HardwareManager
{
private:
    const uint16_t PixelCount = 2; // this example assumes 4 pixels, making it smaller will cause a failure
    const uint8_t PixelPin = 33;   // make sure to set this to the correct pin, ignored for Esp8266
    static const uint8_t A6 = (uint8_t)34U;
    NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip;

    void initButtons();
    void initLEDs();
    void updateLEDs();

public:
    Buttons buttons;
    JacksIn jacksIn;
    JacksOut jacksOut;
    Pots pots;

    HardwareManager();

    void begin();
    void loop();
    
};

#endif