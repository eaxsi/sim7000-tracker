#include "communication.h"

#include "util.h"

Communication::Communication(TinyGsm* modem,
                             TinyGsmClient* client,
                             Settings* config,
                             MQTT_CALLBACK_SIGNATURE)
{
    m_modem = modem;
    m_client = client;
    m_config = config;

    m_mqtt = PubSubClient(*m_client);
    m_mqtt.setCallback(callback);

    pinMode(MODEM_POWER_PIN, OUTPUT);
    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_POWER_PIN, HIGH);
    digitalWrite(MODEM_RESET_PIN, LOW);
}

bool Communication::init(ota::status ota_status)
{
    // init modem
    MODEM_SERIAL.begin(MODEM_BAUDRATE, SERIAL_8N1, MODEM_SERIAL_RX_PIN, MODEM_SERIAL_TX_PIN);
    MODEM_SERIAL.flush();
    if(m_modem->testAT(200))
    {
        m_modem->restart();
        delay(1000);
    }

    while (!m_modem->testAT(200)) {
        INFO("...");
        delay(1000);
    }

    m_modem->init();
    bool imei_success = false;
    while (!imei_success) {
        String imei = m_modem->getIMEI();
        if (imei.toInt() != 0 && imei.length() > 5) {
            imei_success = true;
            imei.substring(8, 15).toCharArray(m_nodeId, 8);
        }
    }

    INFO_VALUE("Node ID: ", m_nodeId);

    m_mqtt.setServer(BROKER_HOST, BROKER_PORT);
    m_ota_status = ota_status;

    uint8_t sim_status = m_modem->getSimStatus();
    if (sim_status == SIM_LOCKED) {
        m_error = 2;
        ERROR("SIM-card locked!");
        return false;
    } else if (sim_status == SIM_ERROR) {
        m_error = 1;
        ERROR("SIM-card missing!");
        return false;
    }
    m_modem->setNetworkMode(38); //  2 = auto, 13=GSM, 38=LTE, 51=LTE+GSM
    m_modem->setPreferredMode(1); // 1=LTE-M, 2=NB-IoT, 3=LTE-M and NB-IoT

    return true;
}

Communication::modem_state Communication::get_state()
{
    return m_modem_state;
}

void Communication::set_state(Communication::modem_state state)
{
    m_requested_modem_state = state;
}

bool Communication::modem_is_off()
{
    return m_modem_state == modem_state::off;
}

bool Communication::connected_to_mqtt_broker()
{
    return m_modem_state == modem_state::mqtt_connected;
}

void Communication::turn_modem_off()
{
    m_modem->poweroff();
    digitalWrite(MODEM_POWER_PIN, LOW);
    delay(100);
}

void Communication::turn_modem_on()
{
    if (!m_modem->testAT(500)) {
        digitalWrite(MODEM_POWER_PIN, HIGH);
        delay(100);
        digitalWrite(MODEM_POWER_PIN, LOW);
        delay(100);
    }
}

void Communication::reset_modem()
{
    digitalWrite(MODEM_RESET_PIN, HIGH);
    delay(500);
    digitalWrite(MODEM_RESET_PIN, LOW);
    delay(500);
}

bool Communication::connect_mqtt()
{
    char topic_buf[40] = "";

    get_topic_name(topic_buf, CONNECTED_TOPIC);
    bool status = m_mqtt.connect(m_nodeId, BROKER_USER, BROKER_PASSWD, topic_buf, 1, true, "0");
    if (status) {
        INFO("MQTT Connect Success");
    } else {
        ERROR("MQTT Connect fail");
        ERROR(m_mqtt.state());
    }
    m_mqtt.publish(topic_buf, "1", true);

    // subscribe to topics
    get_topic_name(topic_buf, SETTINGS_SUBSCRIBE_TOPIC);
    m_mqtt.subscribe(topic_buf);

    get_topic_name(topic_buf, ERROR_SUBSCRIBE_TOPIC);
    m_mqtt.subscribe(topic_buf);

    get_topic_name(topic_buf, OTA_SUBSCRIBE_TOPIC);
    m_mqtt.subscribe(topic_buf);

    // send status messages
    //send_status();
    if (m_first_connection) {
        get_topic_name(topic_buf, VERSION_TOPIC);
        m_mqtt.publish(topic_buf, VERSION);
        if(m_ota_status != ota::status::none)
        {
            INFO_VALUE("Ota status: ", m_ota_status);
            Serial.println(m_ota_status);
            send_ota_status(m_ota_status);
        }
        
        m_first_connection = false;
    }

    m_mqtt.loop();
    return status;
}

void Communication::disconnect_mqtt()
{
    char topic_buf[40] = "";
    get_topic_name(topic_buf, CONNECTED_TOPIC);
    m_mqtt.publish(topic_buf, "0", true);
    m_mqtt.disconnect();
}

void Communication::mqtt_callback(char* topic, byte* payload, unsigned int len)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.write(payload, len);
    Serial.println();

    // parse topic and update device shadow
    char* pt;
    uint8_t i = 0;
    pt = strtok(topic, "/");
    while (pt != NULL) {
        if (i == 0 && strcmp(pt, PROJECT_NAME) != 0)
            return;
        else if (i == 2 && strcmp(pt, "settings") == 0) {
            payload[len] = '\0';
            String message = String((char*)payload);

            // Find the index of the first comma
            int commaIndex = message.indexOf(',');
            if (commaIndex == -1) {
                ERROR("Invalid payload format");
                return;
            }

            String modeStr = message.substring(0, commaIndex);
            String settingStr = message.substring(commaIndex + 1);

            // Convert the substrings to appropriate types
            uint8_t received_mode = modeStr.toInt();
            uint32_t received_periocid_tracking_interval = settingStr.toInt();

            m_config->set_mode((system_mode)received_mode);
            m_config->set_periodic_tracking_interval(received_periocid_tracking_interval);
            INFO("Changing device mode");
        } else if (i == 2 && strcmp(pt, "ota") == 0) {
            payload[len] = '\0';
            char wifi_ssid[50] = "";
            char wifi_passwd[50] = "";
            String s = String((char*)payload);
            int sep_index = s.indexOf(":");
            s.substring(0, sep_index).toCharArray(m_wifi_details.wifi_ssid, 40);
            s.substring(sep_index + 1).toCharArray(m_wifi_details.wifi_passwd, 40);
            m_config->set_mode(system_mode::ota);
        }
        pt = strtok(NULL, "/");
        i++;
    }
}

bool Communication::send_location(location_update* l)
{
    if (connected_to_mqtt_broker()) {
        char str[80];
        sprintf(str,
                "%f,%f,%f,%f,%f,%d,%d,%d",
                l->lat,
                l->lon,
                l->speed,
                l->alt,
                l->accuracy,
                l->course,
                l->vsat,
                l->usat);
        updateValue(LOC_UPDATE_TOPIC, str);
        return true;
    } else {
        return false;
    }
}

bool Communication::send_status(uint8_t soc_in, bool charging)
{
    if (connected_to_mqtt_broker()) {
        int soc = soc_in;
        if(soc_in == 255)
        {
            soc = -1;
        }
        char str[80];
        sprintf(str, "%d,%d,%d", soc, get_signal_strength(), charging);
        updateValue(STATUS_TOPIC, str);
        return true;
    } else {
        return false;
    }
}

bool Communication::send_ota_status(ota::status status)
{
    char str[10];
    sprintf(str, "%d", status);
    return updateValue(OTA_STATUS_TOPIC, str);
}

bool Communication::request_settings()
{
    if (connected_to_mqtt_broker()) {
        return updateValue(SETTINGS_REQUEST_TOPIC, "2");
    } else {
        return false;
    }
}

uint8_t Communication::get_signal_strength()
{
    return (m_modem->getSignalQuality() * 827 + 127) >> 8;
}

void Communication::update()
{
    if (util::get_time_diff(m_status_check_timestamp) > 30 * 1000
        || m_modem_state != m_requested_modem_state) {
        INFO("Net check!");
        // diagnose
        if (m_mqtt.connected())
            m_modem_state = modem_state::mqtt_connected;
        else if (!m_modem->testAT(200))
            m_modem_state = modem_state::off;
        else if (m_modem->isGprsConnected())
            m_modem_state = modem_state::active_data_connection;
        else if (m_modem->isNetworkConnected())
            m_modem_state = modem_state::registered_to_network;
        else if (m_modem->testAT(300))
            m_modem_state = modem_state::not_registered_to_network;

        INFO_VALUE("Modem state: ", m_modem_state);
        INFO_VALUE("Target state: ", m_requested_modem_state);

        if (m_modem_state == modem_state::off) {
            if (m_requested_modem_state > m_modem_state) {
                INFO("Turning modem on");
                if (m_modem_failed_turn_on_counter < 5) {
                    turn_modem_on();
                    m_modem_failed_turn_on_counter++;
                } else {
                    reset_modem();
                    m_modem_failed_turn_on_counter = 0;
                }
            }
        } else if (m_modem_state == modem_state::not_registered_to_network) {
            /*
            //if stays in not reqistered state, raise error
            if(millis() > 90000 && !m_device_has_initialized) // if no network registeration for 1,5 min
                {
                ERROR("Network registeration failed!");
                m_error = 3;
            }
            else 
            */
            if (m_requested_modem_state == modem_state::off) {
                INFO("Turning modem off");
                turn_modem_off();
                m_modem_failed_turn_on_counter = 0;
            }

        } else if (m_modem_state == modem_state::registered_to_network) {
            if (m_requested_modem_state > m_modem_state) {
                INFO("Connecting to GPRS");
                delay(700);
                m_modem->gprsConnect(APN, GPRS_USER, GPRS_PASS);
            } else if (m_requested_modem_state == modem_state::off) {
                INFO("Turning modem off");
                turn_modem_off();
            }
        } else if (m_modem_state == modem_state::active_data_connection) {
            if (m_requested_modem_state > m_modem_state) {
                INFO("Connecting to MQTT Broker");
                connect_mqtt();
            } else if (m_requested_modem_state < m_modem_state) {
                // Deactivate data connection
                INFO("Disconnecting GPRS");
                m_modem->gprsDisconnect();
            } else // Where we want to be
            {
            }
        } else if (m_modem_state == modem_state::mqtt_connected) {
            if (m_requested_modem_state < m_modem_state) {
                INFO("Disconnecting MQTT");
                disconnect_mqtt();
            } else if (m_requested_modem_state == m_modem_state) {
                // Where we want to be
            }
        }

        m_status_check_timestamp = millis();
    }
    m_mqtt.loop();
}

void Communication::get_topic_name(char* topic_buf, char* topic_name)
{
    strcpy(topic_buf, PROJECT_NAME);
    strcat(topic_buf, "/");
    strcat(topic_buf, m_nodeId);
    strcat(topic_buf, "/");
    strcat(topic_buf, topic_name);
}

bool Communication::updateValue(char* topic_name, char* value_buffer)
{
    char topic_buffer[50] = "";
    get_topic_name(topic_buffer, topic_name);
    return m_mqtt.publish(topic_buffer, value_buffer);
}

bool Communication::get_ota_wifi_details(wifi_details* ota_wifi)
{
    if (m_wifi_details.wifi_ssid != "" && m_wifi_details.wifi_passwd != "") {
        strcpy(ota_wifi->wifi_ssid, m_wifi_details.wifi_ssid);
        strcpy(ota_wifi->wifi_passwd, m_wifi_details.wifi_passwd);
        return true;
    } else {
        return false;
    }
}
