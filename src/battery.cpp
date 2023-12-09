#include "battery.h"

Battery::Battery(uint8_t battery_pin)
{
    m_battery_pin = battery_pin;
    pinMode(m_battery_pin, INPUT);

    m_battery_voltage = get_raw_voltage_from_pin(m_battery_pin);
}

uint8_t Battery::get_soc()
{
    return m_soc;
}

float Battery::get_voltage()
{
    return m_battery_voltage;
}

bool Battery::is_charging()
{
    return get_raw_voltage_from_pin(m_battery_pin) < 2000;
}

float Battery::get_raw_voltage_from_pin(uint8_t pin)
{
    return map(analogRead(pin), 0, 4095, 0, 6600);
}

void Battery::update()
{
    if (util::get_time_diff(m_voltage_measurement_timemstamp) > m_voltage_measurement_interval) {
        int raw_soc = m_soc;
        bool charging = is_charging();
        if(charging)
        {
            if(charging && !m_last_charging_state) // started charging
            {
                m_charing_start_soc = m_soc;
                m_charging_start_time = millis();
            }
            else // continue charging
            {
                float hours_since_charging_started = (millis()-m_charging_start_time)/3600000.0f;
                raw_soc = m_charing_start_soc + (100*(hours_since_charging_started * CHARGING_CURRENT) / BATTERY_CAPACITY);
            }
        }
        else
        {
            m_battery_voltage
                = 0.1f * get_raw_voltage_from_pin(m_battery_pin) + 0.9f * m_battery_voltage;
            raw_soc = map(m_battery_voltage, 2400, 3560, 0, 100);
        }
        m_soc = (uint8_t)constrain(raw_soc, 0, 100);
        m_last_charging_state = charging;
        m_voltage_measurement_timemstamp = millis();
    }
}
