#pragma once

#include "Arduino.h"

#include "hardware_configuration.h"

class Binary_sensor
{
    public:
        Binary_sensor(uint8_t pin);
        void update();
        bool activated();
        bool onChange();
        bool onRisingEdge();
        bool get_state();

    private:
        uint8_t m_pin;
        bool m_state;
        bool m_old_state;
};
