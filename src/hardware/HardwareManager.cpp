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

    updateLEDs(); // the website switch-button writes to jacksOut struct -> these set the pixelcolor of the LEDs 1 and 2
}