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
        float m_battery_voltage;
        uint32_t m_voltage_measurement_timemstamp;
        uint32_t m_voltage_measurement_interval = 1000; //ms
        uint8_t m_soc;
        bool m_charging;
        bool m_last_charging_state;
        uint8_t m_charing_start_soc;
        uint32_t m_charging_start_time;
        bool m_has_measured_battery_voltage;
};
