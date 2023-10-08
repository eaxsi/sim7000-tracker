#pragma once

#include "Arduino.h"
#include "driver/rtc_io.h"

#include "hardware_configuration.h"
#include "log.h"
#include "util.h"

class platform
{
    public:
        platform();

        enum class event { movement, magnet, charger_plugged, charger_unplugged, none };
        enum class wake_up_device { magnet, movement };
        struct pinstates{
            bool acc;
            bool magnet;
            bool charger;
        };

        uint8_t get_soc();
        float get_voltage();

        event get_event();
        void set_wake_up_device(wake_up_device);
        void sleep();
        void sleep(uint32_t timeout); // in s
        void deep_sleep();
        void deep_sleep(uint32_t timeout); // in s
        float get_raw_battery_voltage();
        bool charging();

        void update();

    private:
        uint16_t m_update_interval;
        pinstates m_pinstates;
        pinstates m_oldpinstates;
        float m_battery_voltage;
        uint32_t m_voltage_measurement_timemstamp;
        uint32_t m_voltage_measurement_interval = 1000;
};
