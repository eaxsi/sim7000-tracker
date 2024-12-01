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
RTC_DATA_ATTR wifi_details ota_wifi_details;
RTC_DATA_ATTR ota::status ota_status = ota::status::none;
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

bool periodic_position_sent = false;

void callback_helper(char* topic, byte* payload, unsigned int len)
{
    communications.mqtt_callback(topic, payload, len);
}

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_OFF);
    while (!Serial)
        ;
    delay(1000);
    INFO("SIM7000-tracker, Eero Silfverberg, 2023");

    // OTA mode
    if (strlen(ota_wifi_details.wifi_ssid) > 0) {
        INFO("Staring OTA");
        ota ota_updater = ota();
        if (ota_updater.try_to_connect_to_wifi(&ota_wifi_details)) {
            INFO("Connected to OTA wifi");
            delay(100);
            ota_status = ota_updater.start();
        } else {
            ota_status = ota::status::wifi_failed;
            ERROR("Failed to connect to wifi");
        }
        strcpy(ota_wifi_details.wifi_ssid, "");
        strcpy(ota_wifi_details.wifi_passwd, "");
        device.restart();
    }
    xTaskCreate(
        blinkTask,       // Task function
        "Blink Task",    // Name of the task (for debugging)
        1000,            // Stack size (in words, not bytes)
        NULL,            // Task input parameter
        2,               // Priority of the task
        NULL             // Task handle
    );

    config.set_mode(system_mode::sleep);
    last_setting_request_timestamp = -setting_request_interval; // request settings at every bootup

    if (bootcount != 0) {
        INFO("Woken up from deep sleep");
    }
    device.update();
    communications.init(ota_status);
    ota_status = ota::status::none;
    bootcount++;
    mode_change_timestamp = millis();
}

void blinkTask(void * parameter) {
  for (;;) {
    device.led_blink_update();
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
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
        case platform::event::long_magnet_hold: {
            WARNING("Restarting modem and device");
            communications.reset_modem();
            device.restart();
            // Should not reach this
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
                    device.sleep(60 * 60); // sec <-- should be about 1h
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
            uint32_t timeout = gnss.has_initial_fix() ? movement_timeout : movement_timeout * 10;
            if (util::get_time_diff(last_movement_timestamp) < timeout || gnss.is_moving()
                || device.charging()) {
                if (!communications.connected_to_mqtt_broker()) {
                    communications.set_state(Communication::modem_state::mqtt_connected);
                }
                if (!gnss.is_on() && !communications.modem_is_off()) {
                    gnss.turn_on();
                }

                else if (!communications.connected_to_mqtt_broker()) {
                    communications.set_state(Communication::modem_state::mqtt_connected);
                } else {
                    // gnss on and connected
                    if (gnss.has_fix()) {
                        if (util::get_time_diff(last_location_timestamp) > location_min_interval) {
                            // send location here
                            location_update loc;
                            gnss.get_location(&loc);
                            communications.send_location(&loc);
                            INFO("POSITION SENT!");
                            last_location_timestamp = millis();
                        }
                    } else {
                        INFO("Waiting for GNSS fix");
                    }
                }
            } else //kulunut liikaa aikaa
            {
                gnss.turn_off();
                communications.set_state(Communication::modem_state::off);
                if (communications.modem_is_off()) {
                    INFO("Going to sleep");
                    Serial.flush();
                    device.set_wake_up_device(platform::wake_up_device::movement);
                    device.sleep(24*60*60); // 24h
                    INFO("Woken up from sleep");

                    // here when woken up by movement
                    last_movement_timestamp = millis();
                    communications.set_state(Communication::modem_state::mqtt_connected);
                }
            }

            break;
        }
        case system_mode::ota: {
            if (communications.get_ota_wifi_details(&ota_wifi_details)) {
                INFO("Wifi details copied from communication");
                INFO(ota_wifi_details.wifi_ssid);
                device.deep_sleep(1);
            }
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
        case system_mode::periodic_tracking: {
            if(!periodic_position_sent && util::get_time_diff(last_location_timestamp) < (5*60*1000))
            {
                if (!gnss.is_on() && !communications.modem_is_off()) {
                    gnss.turn_on();
                }
                if(!communications.connected_to_mqtt_broker())
                {
                    communications.set_state(Communication::modem_state::mqtt_connected);
                }
                if(gnss.has_fix() && communications.connected_to_mqtt_broker())
                {
                    INFO("Sending periodic location");
                    location_update loc;
                    gnss.get_location(&loc);
                    communications.send_location(&loc);
                    periodic_position_sent = true;
                }
            }
            else
            {
                if(gnss.is_on())
                {
                    gnss.turn_off();
                }
                if(!communications.modem_is_off())
                {
                    communications.set_state(Communication::modem_state::off);
                }
                if(!gnss.is_on() && communications.modem_is_off())
                {
                    device.set_wake_up_device(platform::wake_up_device::magnet);
                    INFO("Going to light sleep");
                    Serial.flush();
                    device.sleep(config.get_periodic_tracking_interval());
                    INFO("Woken up");
                    periodic_position_sent = false; // reset
                    last_location_timestamp = millis();
                }
            }
            break;
        }
        default: break;
    }

    if (communications.connected_to_mqtt_broker()) {
        if (util::get_time_diff(last_status_timestamp) > status_interval) {
            communications.send_status(device.get_soc(), device.charging());
            last_status_timestamp = millis();
        }
        if (util::get_time_diff(last_setting_request_timestamp) > setting_request_interval) {
            communications.request_settings();
            last_setting_request_timestamp = millis();
        }
    }

    // Updates
    communications.update();
    device.update();
    gnss.update();
}
