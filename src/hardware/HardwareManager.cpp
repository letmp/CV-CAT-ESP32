#include "HardwareManager.h"

HardwareManager::HardwareManager() : strip(PixelCount, PixelPin)
{
    buttons = (Buttons){0, 0, 0, 0, 0, 0, 0, 0};
    jacksIn = (JacksIn){0, 0, 0, 0};
    jacksOut = (JacksOut){0, 0, 0, 0};
    pots = (Pots){0, 0, 0, 0, 0, 0, 0, 0};
}

void HardwareManager::begin()
{
    initButtons();
    initLEDs();
}

void HardwareManager::initButtons()
{
    pinMode(36, INPUT_PULLUP); // Buttons
    pinMode(39, INPUT_PULLUP);
}

void HardwareManager::initLEDs()
{
    // this resets all the neopixels to an off state
    strip.Begin();
    strip.Show();
}

void HardwareManager::updateLEDs()
{
    strip.SetPixelColor(0, RgbColor(jacksOut.jack1 / 32));
    strip.SetPixelColor(1, RgbColor(jacksOut.jack2 / 32));
    strip.Show();
}

void HardwareManager::loop()
{
    buttons = (Buttons){.button1 = digitalRead(36), .button2 = digitalRead(39)};
    jacksIn = (JacksIn){.jack1 = analogRead(A4)};
    pots = (Pots){.pot1 = analogRead(A6), .pot2 = analogRead(A7)};

    updateLEDs();

    /*static const int interval = 500; // publishes every second
    static uint32_t timer = millis() + interval;

    if (millis() > timer)
    {
        int sensorValue1 = analogRead(A4); // klinke
        int sensorValue2 = analogRead(A6); // Poti1
        int sensorValue3 = analogRead(A7); // Poti2
        Serial << "Jack: " << sensorValue1 << " | Pot1: " << sensorValue2 << " | Pot2: " << sensorValue3 << endl;

        int buttonVal1 = digitalRead(36);
        int buttonVal2 = digitalRead(39);
        Serial << "Button1: " << buttonVal1 << " | Button2: " << buttonVal2 << endl;

        if (!mLedOn)
        {
            strip.SetPixelColor(0, white);
            strip.SetPixelColor(1, white);
            mLedOn = true;
        }
        else
        {
            strip.SetPixelColor(0, black);
            strip.SetPixelColor(1, black);
            mLedOn = false;
        }
        strip.Show();

        timer += interval;
    }*/
}