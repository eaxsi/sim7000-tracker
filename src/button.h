#pragma once

#include "Arduino.h"

class Button
{
    public:
        Button(uint8_t pin) : m_pin(pin) {pinMode(m_pin, INPUT_PULLUP);};
        bool is_pushed();

    private:
        uint8_t m_pin;
};
