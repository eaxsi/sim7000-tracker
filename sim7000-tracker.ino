/*
        SIM7000-tracker
    Based on LilyGO-T-SIM7000G board
*/

#include "Arduino.h"

#include "src/hardware_configuration.h"

#include <TinyGsmClient.h>

#include "src/common.h"
#include "src/communication.h"
#include "src/gnss.h"
#include "src/log.h"
#include "src/ota.h"
#include "src/platform.h"
#include "src/settings.h"
#include "src/util.h"

TinyGsm modem = TinyGsm(MODEM_SERIAL);
TinyGsmClient client = TinyGsmClient(modem);

void callback_helper(char* topic, byte* payload, unsigned int len);

// store system state in RTC memory so that it will be remembered through out sleep
RTC_DATA_ATTR uint16_t bootcount = 0;
Settings config = Settings();
platform device = platform();
Gnss gnss = Gnss(&modem);
Communication communications = Communication(&modem, &client, &config, callback_helper);

uint32_t mode_change_timestamp;
uint32_t last_location_timestamp;
uint32_t last_movement_timestamp;
uint32_t last_status_timestamp;
uint32_t last_setting_request_timestamp;

static uint32_t time_to_sleep = 60 * 1000; //ms
static uint32_t movement_timeout = 1 * 60 * 1000; //ms
static uint32_t location_min_interval = 10 * 1000; //ms
static uint32_t status_interval = 90 * 1000;
static uint32_t setting_request_interval = 5 * 60 * 1000;

void callback_helper(char* topic, byte* payload, unsigned int len)
{
    communications.mqtt_callback(topic, payload, len);
}

void setup()
{
    Serial.begin(115200);
    while(!Serial);
    delay(1000);
    INFO("SIM7000-tracker, Eero Silfverberg, 2023");

    config.set_mode(system_mode::sleep);
    last_setting_request_timestamp = -setting_request_interval; // request settings at every bootup

    if (bootcount != 0) {
        INFO("Woken up from deep sleep");
    }
    communications.init();
    bootcount++;
    mode_change_timestamp = millis();
}

void loop()
{
    // event state machine
    platform::event event = device.get_event();

    switch (event) {
        case platform::event::charger_plugged: break;
        case platform::event::magnet: INFO("Magnet detected"); break;
        case platform::event::movement: {
            last_movement_timestamp = millis();
            INFO("Movement detected");
            break;
        }
        default: break;
    }

    switch (config.get_mode()) {
        case system_mode::hibernate: {
            //set modem to sleep
            communications.set_state(Communication::modem_state::off);
            if (communications.modem_is_off()) {
                device.set_wake_up_device(platform::wake_up_device::magnet);
                config.set_mode(system_mode::sleep);
                INFO("Going to deep sleep");
                device.deep_sleep();
            }
            break;
        }
        case system_mode::sleep: {
            if (util::get_time_diff(mode_change_timestamp) > time_to_sleep) {
                communications.set_state(Communication::modem_state::off);
                if (communications.modem_is_off()) {
                    device.set_wake_up_device(platform::wake_up_device::magnet);
                    INFO("Going to light sleep");
                    Serial.flush();
                    device.sleep(60*60); // sec <-- should be about 1h
                    // Woken up here
                    mode_change_timestamp = millis();
                    communications.set_state(Communication::modem_state::mqtt_connected);
                    INFO("Woken up from sleep");
                }
            } else {
                communications.set_state(Communication::modem_state::mqtt_connected);
            }
            break;
        }
        case system_mode::track: {
            if (util::get_time_diff(last_movement_timestamp) < movement_timeout || !gnss.has_initial_fix() || gnss.is_moving() || device.charging()) {
                if(!communications.connected_to_mqtt_broker())
                {
                    communications.set_state(Communication::modem_state::mqtt_connected);
                }
                if(!gnss.is_on() && !communications.modem_is_off())
                {
                    gnss.turn_on();
                }

                else if (!communications.connected_to_mqtt_broker()) {
                    communications.set_state(Communication::modem_state::mqtt_connected);
                }
                else
                {
                    // gnss on and connected
                    if(gnss.has_fix())
                    {
                        if (util::get_time_diff(last_location_timestamp) > location_min_interval) {
                            // send location here
                            location_update loc;
                            gnss.get_location(&loc);
                            communications.send_location(&loc);
                            INFO("POSITION SENT!");
                            last_location_timestamp = millis();
                        }
                    }
                    else
                    {
                        INFO("Waiting for GNSS fix");
                    }
                }
            }
            else //kulunut liikaa aikaa
            {
                gnss.turn_off();
                communications.set_state(Communication::modem_state::off);
                if (communications.modem_is_off()) {
                    INFO("Going to sleep");
                    Serial.flush();
                    device.set_wake_up_device(platform::wake_up_device::movement);
                    device.sleep();
                    INFO("Woken up from sleep");
                    
                    // here when woken up by movement
                    last_movement_timestamp = millis();
                    communications.set_state(Communication::modem_state::mqtt_connected);
                }
            }

            break;
        }
        case system_mode::ota: {
            wifi_details ota_wifi;
            Ota ota = Ota();
            /*
            if(communications.get_ota_wifi_details(&ota_wifi))
            {
                if(ota.try_to_connect_to_wifi(&ota_wifi))
                {
                    Serial.println("Starting OTA");
                    delay(100);
                    ota.start();
                }
            }
            */
            config.set_mode(system_mode::sleep);

            break;
        }
        case system_mode::idle: {
            // stay connected, idle
            if (!communications.connected_to_mqtt_broker()) {
                communications.set_state(Communication::modem_state::mqtt_connected);
            }
            break;
        }
        default: break;
    }

    if(communications.connected_to_mqtt_broker())
    {
        if(util::get_time_diff(last_status_timestamp) > status_interval)
        {
            communications.send_status(device.get_soc(), device.charging());
            last_status_timestamp = millis();
        }
        if(util::get_time_diff(last_setting_request_timestamp) > setting_request_interval)
        {
            communications.request_settings();
            last_setting_request_timestamp = millis();
        }
    }


    // Updates
    communications.update();
    device.update();
    gnss.update();
}
