#pragma once
/*
#include "led.h"
#include "sysclock.h"

#include "hardware_configuration.h"

#include <stdint.h>


class Leds
{
    public:
        Leds(SysClock& clock) : m_clock(clock) {};

        enum Mode { fixed, blink, off };
        
        void turn_off();
        void update();
        void static_light();
        void blink_light(uint16_t speed);

    private:
        SysClock& m_clock;

        Led m_internal_led = Led(LED_PIN, true);

        Mode m_mode = Mode::off;

        uint16_t m_blink_interval = 1000;
        uint32_t m_blink_timestamp;
};
*/
