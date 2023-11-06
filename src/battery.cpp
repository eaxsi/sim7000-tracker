#include "battery.h"

Battery::Battery(uint8_t battery_pin, uint8_t charging_pin)
{
    m_battery_pin = battery_pin;
    m_charging_pin = charging_pin;
    pinMode(m_battery_pin, INPUT);
    pinMode(m_charging_pin, INPUT);

    m_battery_voltage = get_raw_voltage_from_pin(m_battery_pin);
    m_charging_voltage = get_raw_voltage_from_pin(m_charging_pin);
}

uint8_t Battery::get_soc()
{
    int raw_soc;
    if (is_charging()) {
        raw_soc = map(m_battery_voltage, 2400, 3755, 0, 100);
    } else {
        raw_soc = map(m_battery_voltage, 2400, 3560, 0, 100);
    }
    return (uint8_t)constrain(raw_soc, 0, 100);
}

float Battery::get_voltage()
{
    return m_battery_voltage;
}

bool Battery::is_charging()
{
    bool charged_from_usb_port = m_battery_voltage < 100;
    bool charged_from_charging_port = m_charging_voltage > m_battery_voltage;
    return charged_from_usb_port || charged_from_charging_port;
}

float Battery::get_raw_voltage_from_pin(uint8_t pin)
{
    return map(analogRead(pin), 0, 4095, 0, 6600);
}

void Battery::update()
{
    if (util::get_time_diff(m_voltage_measurement_timemstamp) > m_voltage_measurement_interval) {
        m_battery_voltage
            = 0.1f * get_raw_voltage_from_pin(m_battery_pin) + 0.9f * m_battery_voltage;
        m_charging_voltage
            = 0.1f * get_raw_voltage_from_pin(m_charging_pin) + 0.9f * m_charging_voltage;
        m_voltage_measurement_timemstamp = millis();
    }
}
