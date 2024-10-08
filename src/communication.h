#pragma once

#include "Arduino.h"

#include "hardware_configuration.h"

#include <TinyGsmClient.h>
#include <PubSubClient.h>

#include "../config.h"
#include "../version.h"
#include "common.h"
#include "log.h"
#include "mqtt_topics.h"
#include "settings.h"
#include "ota.h"

#define MQTT_VERSION MQTT_VERSION_3_1
#define TINY_GSM_USE_GPRS true

class Communication
{
    public:
        enum modem_state {
            off,
            sim_locked,
            sim_unlocked,
            not_registered_to_network,
            registered_to_network,
            active_data_connection,
            mqtt_connected
        };

        Communication(TinyGsm* modem,
                      TinyGsmClient* client,
                      Settings* config,
                      MQTT_CALLBACK_SIGNATURE);
        bool init(ota::status);
        void update();

        void set_state(modem_state);
        modem_state get_state();
        bool connected_to_mqtt_broker();
        bool modem_is_off();
        void mqtt_callback(char* topic, byte* payload, unsigned int len);
        bool send_location(location_update* loc);
        bool send_status(uint8_t soc, bool charging);
        bool request_settings();
        bool get_ota_wifi_details(wifi_details*);
        bool send_ota_status(ota::status status);
        void reset_modem();

    private:
        bool connect_mqtt();
        void disconnect_mqtt();
        void turn_modem_off();
        void turn_modem_on();
        void get_topic_name(char* topic_buf, char* topic_name);
        bool updateValue(char* topic_name, char* value_buffer);
        uint8_t get_signal_strength();

        modem_state m_modem_state;
        modem_state m_requested_modem_state;

        //Settings& m_config;
        Settings* m_config;

        uint8_t m_error;
        ota::status m_ota_status;
        char m_nodeId[8] = "";

        uint32_t m_status_check_timestamp = 0;
        wifi_details m_wifi_details;
        bool m_first_connection = true;
        uint8_t m_modem_failed_turn_on_counter = 0;

        TinyGsm* m_modem;
        TinyGsmClient* m_client;
        PubSubClient m_mqtt;
};
