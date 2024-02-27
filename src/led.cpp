#include "led.h"

void Led::turn_on()
{
    m_state = true;
    digitalWrite(m_pin, m_reverse_logic ? !m_state : m_state);
}

void Led::turn_off()
{
    m_state = false;
    digitalWrite(m_pin, m_reverse_logic ? !m_state : m_state);
}

void Led::set(bool state)
{
    m_state = state;
    if (m_reverse_logic)
        m_state = !m_state;
    digitalWrite(m_pin, state);
}

bool Led::get()
{
    return m_state;
}
