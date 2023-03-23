#include "logic.h"

#include "util.h"

void Logic::update()
{
    // if timer has expired

    //run sublasses updates

    // if hall sensor triggered

    // 

    bool send_location = false;
    bool send_sysinfo = false;

    if(m_location_sending_enabled && communications.gnss_enabled())
    if(get_time_diff(m_last_time_location_sent) < 60)
    if((m_last_time_movement - m_last_time_location_sent) > 0) // if there has been movement since last position send



    /* send location if following criteria is met:
    - location sending is enabled
    - time since last update is over the threshold
    - device has moved since last update

    */

   /* send sysinfo: battery level, signal... if:
    - has changed
    - at least x minutes interval

   /*
   Power saving features
    - turn off GNSS if
        - time to fix is above 5 min: wait movement and 5 min to try again
        - no movement in the last 30 sec and there is GNSS fix
    - deactivate data connection if gnss off and
        - if no movement for 10 min
    - disable 

   */
    if(m_mode == Mode::normal)
    {
        // power saving features here
        if()
    }

    if(send_location)
        communications.send_position();
    if(send_sysinfo)
        communications.send_sysinfo();


    if(m_mode != m_last_mode) // mode has chaged
    {
        if(m_mode == Mode::normal)
        {
            communications.turn_gnss_on();
            communications.activate_data_connection();
            communications.connect_MQTT();
        }
        else if(m_mode == Mode::force)
        {
            communications.turn_gnss_on();
            communications.activate_data_connection();
            communications.connect_MQTT();
            // set sending intervals as low as possible
        }
        else if(m_mode == Mode::idle)
        {
            communications.turn_gnss_off();
            communications.activate_data_connection();
            communications.connect_MQTT();
        }
        else if(m_mode == Mode::sleep || m_mode == Mode::off)
        {
            communications.turn_gnss_off();
            communications.disconnect_MQTT();
            communications.deactivate_data_connection();

            if(m_mode == Mode::sleep)
            {
                communication.set_modem_to_powersave();
                system.goto_light_sleep();
            }
            else // Mode::off
            {
                communications.turn_modem_off();
                system.goto_deep_sleep();
                // should not get here
            }
        }
    }

    // checks every time
    // power saving
    // position sendig
    // sysinfo sending



    //Power saving things

    // if no power saving active, should we update location

    // 
    if(time_since_movement > 60 && !)

    if(movement_detected && gnss_disabled)
    {
        // enable gnss

    }

    bool update_location = false;

    if(m_mode == Mode::normal)
    {
        if(time_since_movement > 60)
        {

        }
    }

    if(m_mode == Mode::force)
    {
        if(time_since_last_update > 10)
        {
            update_location = true;
        }
    }

    if(update_location)
    {
        communication.update_location();
        
    }
        


    // if movement: hotstart gnss, wait until lock, send position and stay on for 2 min, else turn gnss off



    m_last_mode = m_mode;
}