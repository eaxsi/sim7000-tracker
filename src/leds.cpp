#include "leds.h"

void Leds::turn_off()
{
    m_mode = Mode::off;
    m_internal_led.turn_off();
}

void Leds::update()
{
    if(m_mode == Mode::blink)
    {
        if(m_clock.get_time_diff(m_blink_timestamp) > m_blink_interval)
        {
            bool led_state = !m_internal_led.get();
            m_internal_led.set(led_state);
            m_blink_timestamp = m_clock.get_time_ms();
        }
    }
}

void Leds::blink_light(uint16_t speed)
{
    if(m_mode != Mode::blink || m_blink_interval != speed)
    {
        turn_off();
        m_mode = Mode::blink;
        m_blink_interval = speed;
    }
}

void Leds::static_light()
{
    Serial.println("Static light");
    if(m_mode != Mode::fixed)
    {
        Serial.println("in if");
        turn_off();
        m_mode = Mode::fixed;
        m_internal_led.turn_on();
    }
}
