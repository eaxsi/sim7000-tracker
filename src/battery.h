
#include "Arduino.h"
#include "util.h"

class Battery
{
    public:
        Battery(uint8_t battery_pin, uint8_t charging_pin);
        uint8_t get_soc();
        float get_voltage();
        bool is_charging();
        void update();

    private:
        float get_raw_voltage_from_pin(uint8_t pin);
        uint8_t m_battery_pin;
        uint8_t m_charging_pin;
        float m_battery_voltage;
        float m_charging_voltage;
        uint32_t m_voltage_measurement_timemstamp;
        uint32_t m_voltage_measurement_interval = 1000; //ms
};
