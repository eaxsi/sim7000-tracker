#pragma once

#include "Arduino.h"
#include "driver/rtc_io.h"

#include "battery.h"
#include "hardware_configuration.h"
#include "led.h"
#include "log.h"
#include "util.h"
#include "binary_sensor.h"

class platform
{
    public:
        platform();

        enum class event {
            movement,
            magnet,
            charger_plugged,
            charger_unplugged,
            long_magnet_hold,
            none
        };
        enum class wake_up_device { magnet, movement };
        struct pinstates
        {
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
        void restart();
        bool charging();
        void turn_off_led();
        void turn_on_led();
        void update();

    private:
        uint16_t m_update_interval;
        pinstates m_pinstates;
        pinstates m_oldpinstates;
        uint32_t m_sensor_hold_timestamp;

        Binary_sensor m_vibration_sensor = Binary_sensor(ACC_SENSOR_PIN);
        Binary_sensor m_reed_sensor = Binary_sensor(REED_PIN);
        Led m_led = Led(LED_PIN, true);
        Battery m_battery = Battery(V_BATT_PIN);

};
