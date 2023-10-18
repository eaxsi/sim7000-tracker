#pragma once

#include "hardware_configuration.h"
#include <TinyGsmClient.h>
#include "common.h"

class Gnss
{
    public:
        enum state { off, no_fix, fix };

        Gnss(TinyGsm* modem);
        void turn_on();
        void turn_off();
        bool is_on();
        bool has_fix();
        bool has_initial_fix();
        bool get_location(location_update* l);
        void update();

    private:
        bool has_fix_impl();
        bool turn_on_impl();
        bool turn_off_impl();
        float get_bearing(float lat,float lon,float lat2,float lon2);

        TinyGsm* m_modem;
        state m_state;
        state m_requested_state;
        location_update m_loc;
        location_update m_old_loc;
        bool m_device_stuck = false;
        bool m_initial_fix_received = false;
};
