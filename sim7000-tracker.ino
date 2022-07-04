// SIM7000-tracker
// Eero Silfverberg 2021

#include "src/hardware_configuration.h"
#include "src/sysclock.h"
#include "src/ui.h"


//#include "arduino_secrets.h"

SysClock sysclock;
Ui ui(sysclock);
//communications: tinygsm, gps, mqtt
//logic: when to send location, when to power saving mode
//system: battery soc, power saving


void setup()
{
    Serial.begin(115200);
    //init sim7000
    //try to connect to 
}


void loop()
{
    ui.testmode();

    /*
    Things to react:
    - Movement
    - Reed/magnet
    - Time
    - Mqtt message
    - battery staus
    - charger status
    - communication status
    - battery voltage


    Possible actions:
    - device moves
    - magnet is sensed
    - timer expires
    - external command
    - change of mode
    - change of settings
    - go to hibernation/power save mode


    Hibernation:
    - come online once in 6h / when reed sensor activates
    - stay 30s online
    - go back to sleep

    Power save mode:
    - wait until GPS fix
    - send location every 20s if device is moving
    - if no movement for 1 min disable gps
    - if no movement for 10 min, disconnect mqtt and active data connection
    - if movement detected, gps hotstart




    Modem modes: off, on, registered to net, active data connection
    GNSS modes: off, searching for fix, fix


    Main logic:
    - see if momement:
    
    logic.update()
    ui.update()
    communication.update()
    system.update()
    settings.update()
    */
}
