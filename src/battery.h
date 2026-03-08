#pragma once

#include "Arduino.h"

#include "util.h"
#include "hardware_configuration.h"

class Battery
{
    public:
        Battery(uint8_t battery_pin);
        uint8_t get_soc();
        float get_voltage();
        bool is_charging();
        void update();

    private:
        float get_raw_voltage_from_pin(uint8_t pin);
        uint8_t m_battery_pin;
        float m_battery_voltage = 0.0f;
        uint32_t m_voltage_measurement_timestamp = 0;
        uint32_t m_voltage_measurement_interval = 1000; //ms
        uint8_t m_soc = 0;
        bool m_charging = false;
        bool m_last_charging_state = false;
        uint8_t m_charging_start_soc = 0;
        uint32_t m_charging_start_time = 0;
        bool m_has_measured_battery_voltage = false;
};
