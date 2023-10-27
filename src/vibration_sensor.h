#pragma once

#include "hardware_configuration.h"
#include "Arduino.h"

class vibration_sensor
{
    public:
        vibration_sensor(uint8_t pin);
        void update();
        bool activated();
        bool get_state();

    private:
        uint8_t m_pin;
        bool m_state;
        bool m_old_state;
};
