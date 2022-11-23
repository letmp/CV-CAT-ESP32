#include "HardwareManager.h"

HardwareManager::HardwareManager() : strip(PixelCount, PixelPin), white(10), black(0)
{
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

void HardwareManager::loop()
{

    static const int interval = 500; // publishes every second
    static uint32_t timer = millis() + interval;

    if (millis() > timer)
    {
        int sensorValue1 = analogRead(A4); // klinke
        int sensorValue2 = analogRead(A6); // Poti1
        int sensorValue3 = analogRead(A7); // Poti2
        Serial.println(sensorValue1);
        Serial.println(sensorValue2);
        Serial.println(sensorValue3);

        int buttonVal1 = digitalRead(36);
        int buttonVal2 = digitalRead(39);

        Serial.println(buttonVal1);
        Serial.println(buttonVal2);

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
    }
}