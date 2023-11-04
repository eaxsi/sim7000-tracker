#include "vibration_sensor.h"

vibration_sensor::vibration_sensor(uint8_t pin)
{
    m_pin = pin;
    pinMode(pin, INPUT_PULLUP);
    m_state = digitalRead(pin);
    m_old_state = m_state;
}

void vibration_sensor::update()
{
    m_old_state = m_state;
    m_state = digitalRead(m_pin);
}

bool vibration_sensor::activated()
{
    return m_state != m_old_state;
}

bool vibration_sensor::get_state()
{
    return m_state;
}
