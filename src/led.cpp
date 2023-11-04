#include "led.h"

void Led::turn_on()
{
    m_state = true;
    if(m_reverse_logic)
        m_state = !m_state;

    digitalWrite(m_pin, m_state);
}

void Led::turn_off()
{
    m_state = false;
    if(m_reverse_logic)
        m_state = !m_state;
    Serial.println(m_state);
    digitalWrite(m_pin, m_state);
}

void Led::set(bool state)
{
    m_state = state;
    if(m_reverse_logic)
        m_state = !m_state;
    digitalWrite(m_pin, state);
}

bool Led::get()
{
    return m_state;
}
