#include "binary_sensor.h"

Binary_sensor::Binary_sensor(uint8_t pin)
{
    m_pin = pin;
    pinMode(pin, INPUT_PULLUP);
    m_state = digitalRead(pin);
    m_old_state = m_state;
}

void Binary_sensor::update()
{
    m_old_state = m_state;
    m_state = digitalRead(m_pin);
}

bool Binary_sensor::activated()
{
    return m_state != m_old_state;
}

bool Binary_sensor::get_state()
{
    return m_state;
}

bool Binary_sensor::onChange()
{
    return m_state != m_old_state;
}

bool Binary_sensor::onRisingEdge()
{
    return m_old_state && !m_state;
}