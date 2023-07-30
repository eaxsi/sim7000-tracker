#include "src/hardware_configuration.h"


enum class system_state{
    idle = 0, // stay connected, gps off, wait for commands, idle for 5 min, then go to 
    // connect to MQTT
    hibernate = 1, // herätys vain magneetilla
    sleep = 2, // herätys magneetti + liikesensori
    track = 3, // main mode, go to sleep if no movement, if movement send position
    track_force = 4 
    };

/*
Start
Is this cold start? --> Fetch settings from internet
Warm start --> continue last mode 

Modes: 
- Deep sleep: wake up when plugging charger or magnet
// - Light sleep: wake up when magnet triggers or motion sensor triggers
- FW update: Fetch update from web
- Forced: update position as fast as possible
- Normal: switch between light sleep and sending position when tracker moves


Normal mode:
- send position, wait for X, if no movement set gps to sleep, if no movement for x+y time, set modem and esp to sleep
- when movement, wake up and wait for gps fix and internet connection
- send position
- wait if more movement happens, if not go back to sleep


*/

// store system state in RTC memory so that it will be remembered through out sleep
RTC_DATA_ATTR system_state state;

void setup()
{
    Serial.begin(115200);

}

void loop()
{
    switch (state)
    {
    case system_state::idle:
        if(5 min passed)
        {
            state = system_state::sleep;
        }
        // wait for instructions or OTA update
        break;
    case system_state::hibernate:
    {

    }
    case system_state::track_force:
    {
        // make sure that we are connented to MQTT and internet
        // make sure that we have GNSS fix
        // 
        if(connected_to_mqtt)
        else
        {
            communication.try_to_connect_to_mqtt_broker();

        }

        break;
    }
    default:
        break;
    }

    // Requests


    // Updates
    communications.update(); // Update communications state machine and keep the device in desired state

}
