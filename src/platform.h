#pragma once

#include "Arduino.h"
#include "driver/rtc_io.h"

#include "hardware_configuration.h"
#include "log.h"

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

        event get_event();
        void set_wake_up_device(wake_up_device);
        void sleep();
        void sleep(uint32_t timeout); // in s
        void deep_sleep();
        void deep_sleep(uint32_t timeout); // in s

        void update();

    private:
        uint16_t m_update_interval;
        pinstates m_pinstates;
        pinstates m_oldpinstates;
};
