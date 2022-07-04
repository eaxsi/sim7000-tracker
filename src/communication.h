#pragma once

#include "Arduino.h"

class Communication
{
    public:
        /** Init
        *
        * Initialize communication:
        * - start up the modem
        * - wait until connected to network
        * - connect to gprs
        * - connect to MQTT broker
        * - wait for settings
        * - if timeout, use default settings
        *
        */
        void init(); 
        void update();

        void enable_gnss();
        void disable_gnss();

        void enable_modem();
        void disable_modem();

        void enable_data_connection();
        void disable_data_connection();

        void set_modem_to_sleep();
        void wake_up_modem();

        
        bool is_modem_powered();
        bool is_gnss_fix_available();
        bool is_data_connection_available();

        void send_location();
        


        void send_location
        
    private:
        
}