// SIM7000-tracker
// Eero Silfverberg 2020

#define VERSION "1.5.0"

#define DEBUG 1

// SIM7000 settings
#define UART_BAUD 9600
#define PIN_DTR 25
#define PIN_TX 27
#define PIN_RX 26
#define PWR_PIN 4

#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13
#define LED_PIN 12
#define V_BATT_PIN 35

// debug serial
#define SerialMon Serial
#define TINY_GSM_DEBUG SerialMon

// modem serial
#define SerialAT Serial1

// tinygsm settings
#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#define GSM_PIN ""
//#define DUMP_AT_COMMANDS

#define uS_TO_S_FACTOR 1000000

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <rom/rtc.h>
#include <rom/gpio.h>
#include "driver/rtc_io.h"

// for FOTA update
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <time.h>

#include "arduino_secrets.h"

// settings
#define APN "internet"
#define GPRS_USER ""
#define GPRS_PASS ""

// MQTT settings
#define PROJECT_NAME "tracker"

// global variables
unsigned long location_timestamp;
unsigned long settings_timestamp;
unsigned long sysinfo_timestamp;
unsigned long sleep_timestamp;

unsigned long location_interval = 10;
unsigned long sysinfo_interval = 30;
unsigned long dynamic_interval = 0;
char nodeId[7] = "";
int mode = 1;
RTC_DATA_ATTR int bootCount = 0; // stored in RTC memory
int old_mode = 1;

unsigned int mqttConnectionCount = 0;
uint32_t lastReconnectAttempt = 0;

unsigned long voltage_buf;
unsigned int voltage_buf_i;

struct GnssData
{
    float lat;
    float lon;
    unsigned int speed;
    unsigned int accuracy;
    bool fix;
    unsigned long last_fix; // time when device had gnss fix
};

struct SysinfoData
{
    unsigned int soc;
    unsigned int signal;
    bool charging;
};

GnssData current_gnss_data;
GnssData old_gnss_data;
SysinfoData current_sysinfo_data;
SysinfoData old_sysinfo_data;

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);
PubSubClient mqtt(client);

void setClock()
{
    configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // UTC

    Serial.print(F("Waiting for NTP time sync: "));
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        yield();
        delay(500);
        Serial.print(F("."));
        now = time(nullptr);
    }

    Serial.println(F(""));
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeinfo));
}

boolean mqttConnect()
{
    SerialMon.print("Connecting to ");
    SerialMon.print(SECRET_BROKER_HOST);

    // Connect to MQTT Broker
    char conn_topic_buf[30] = "";
    get_topic_name(conn_topic_buf, "connected");

    boolean status
        = mqtt.connect(nodeId, SECRET_BROKER_USER, SECRET_BROKER_PW, conn_topic_buf, 1, true, "0");

    if (status == false) {
        SerialMon.println(" fail");
        return false;
    }
    SerialMon.println(" success");
    mqtt.publish(conn_topic_buf, "1", true);

    // subscribe to topics
    char settings_wildcard_topic_buf[40] = "";
    get_topic_name(settings_wildcard_topic_buf, "settings/#");
    mqtt.subscribe(settings_wildcard_topic_buf);

    if (mqttConnectionCount == 0 && mqtt.connected()) // first connection
    {
        // send sw-version
        updateValue("version", VERSION);
        updateValue("rebootReason", rtc_get_reset_reason(0));
        send_network_category();
        sendValue("fix", 0);
    }
    return mqtt.connected();
}

void send_network_category()
{
    //network type
    char net_modes_buf[100] = "";
    modem.sendAT(GF("+CREG=2"));
    modem.waitResponse();
    modem.sendAT(GF("+CREG?"));
    if (modem.waitResponse(GF(GSM_NL "+CREG:")) != 1) {
        return;
    }

    modem.stream.readStringUntil(','); // n
    modem.stream.readStringUntil(','); // stat
    modem.stream.readStringUntil(','); // lac
    modem.stream.readStringUntil(','); // ci
    String net_mode = modem.stream.readStringUntil('\n'); //netcat

    switch (net_mode.toInt()) {
        case 1: updateValue("networkReg", "GSM"); break;
        case 3: updateValue("networkReg", "EDGE"); break;
        case 7: updateValue("networkReg", "LTE-M"); break;
        case 9: updateValue("networkReg", "NB-IoT"); break;
        default: break;
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int len)
{
    SerialMon.print("Message arrived [");
    SerialMon.print(topic);
    SerialMon.print("]: ");
    SerialMon.write(payload, len);
    SerialMon.println();

    char shadow_topic_pub[50] = "";
    char value_buffer[20] = "";

    // parse topic and update device shadow
    char* pt;
    int i = 0;
    pt = strtok(topic, "/");
    while (pt != NULL) {
        if (i == 0 && strcmp(pt, "tracker") != 0)
            return;
        else if (i == 1 && strcmp(pt, nodeId) != 0)
            return;
        else if (i == 2 && strcmp(pt, "settings") != 0)
            return;
        else if (i == 3) {
            payload[len] = '\0';
            String s = String((char*)payload);
            if (s.toInt()) {
                if (strcmp(pt, "pos_int") == 0 && s.toInt() > 0) {
                    location_interval = s.toInt();
                    updateValue("shadow/pos_int", location_interval);
                    return;
                } else if (strcmp(pt, "sysinfo_int") == 0 && s.toInt() > 0) {
                    sysinfo_interval = s.toInt();
                    updateValue("shadow/sysinfo_int", sysinfo_interval);
                    return;
                } else if (strcmp(pt, "mode") == 0 && s.toInt() > 0 && s.toInt() <= 4) {
                    mode = s.toInt();
                    updateValue("shadow/mode", mode);
                    return;
                }
            } else if (strcmp(pt, "action") == 0) {
                if (s == "reboot") {
                    updateValue("shadow/action", "reboot");
                    ESP.restart();
                } else if (s == "restart_modem") {
                    updateValue("shadow/action", "restart_modem");
                    //implement here
                } else if (s == "restart_gnss") {
                    updateValue("shadow/action", "restart_gnss");
                    disableGPS();
                    delay(200);
                    enableGPS();
                } else if (s == "get_netcat") {
                    send_network_category();
                }
                return;
            } else if (strcmp(pt, "fw_upgrade") == 0) {
                char wifi_ssid[50] = "";
                char wifi_passwd[50] = "";

                updateValue("shadow/fw_upgrade", "1");

                //get wifi details from the message
                // loacte separating character
                int sep_index = s.indexOf(":");
                // get ssid
                s.substring(0, sep_index).toCharArray(wifi_ssid, 50);
                // get password
                s.substring(sep_index + 1).toCharArray(wifi_passwd, 50);

                SerialMon.println(wifi_ssid);
                SerialMon.println(wifi_passwd);

                WiFi.begin(wifi_ssid, wifi_passwd);

                long wifi_connecting_timestamp = millis();
                while (WiFi.status() != WL_CONNECTED
                       || wifi_connecting_timestamp + 15 * 1000 < millis()) {
                    delay(500);
                    Serial.println("Connecting to WiFi..");
                }

                if (WiFi.status() == WL_CONNECTED) {
                    updateValue("shadow/fota_status", "wifi_connected");
                    setClock();
                    WiFiClient updater_client;
                    updater_client.setTimeout(12000);
                    digitalWrite(LED_PIN, LOW);

                    t_httpUpdate_return ret = httpUpdate.update(updater_client, HTTP_OTA_URL);
                    switch (ret) {
                        case HTTP_UPDATE_FAILED:
                            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n",
                                          httpUpdate.getLastError(),
                                          httpUpdate.getLastErrorString().c_str());
                            updateValue("shadow/fota_status", "fota_fail");
                            break;

                        case HTTP_UPDATE_NO_UPDATES:
                            updateValue("shadow/fota_status", "fota_noupdates");
                            Serial.println("HTTP_UPDATE_NO_UPDATES");
                            break;

                        case HTTP_UPDATE_OK:
                            updateValue("shadow/fota_status", "fota_updateok");
                            Serial.println("HTTP_UPDATE_OK");
                            break;
                    }
                } else
                    updateValue("shadow/fota_status", "wifi_fail");
                WiFi.disconnect();
                digitalWrite(LED_PIN, HIGH);

                return;
            }
        }
        pt = strtok(NULL, "/");
        i++;
    }
}

void enableGPS()
{
    // power GPIO4/active gnss antenna
    modem.sendAT("+SGPIO=0,4,1,1");
    if (modem.waitResponse(1000L) != 1) {
        SerialMon.print("Failed to set GPIO4");
    }

    // power GNSS
    modem.enableGPS();
    SerialMon.println("GNSS enabled!");
}

void disableGPS()
{
    // turn off GNSS
    modem.disableGPS();

    // turn off GPIO4/active gnss antenna
    modem.sendAT("+SGPIO=0,4,1,0");
    if (modem.waitResponse(1000L) != 1) {
        SerialMon.print("Failed to set GPIO4");
    }
    SerialMon.println("GNSS disabled!");
}

void get_topic_name(char* topic_buf, char* topic_name)
{
    strcpy(topic_buf, "tracker/");
    strcat(topic_buf, nodeId);
    strcat(topic_buf, "/");
    strcat(topic_buf, topic_name);
}

void updateValue(char* topic_name, int value)
{
    char topic_buffer[50] = "";
    char value_buffer[30];
    get_topic_name(topic_buffer, topic_name);
    itoa(value, value_buffer, 10);
    mqtt.publish(topic_buffer, value_buffer);
}

void updateValue(char* topic_name, char* value_buffer)
{
    char topic_buffer[50] = "";
    get_topic_name(topic_buffer, topic_name);
    mqtt.publish(topic_buffer, value_buffer);
}

void setup()
{
    bootCount++;
    setCpuFrequencyMhz(80);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
    rtc_gpio_hold_dis(GPIO_NUM_0);
    SerialMon.begin(115200);
    Serial.print("Version");
    Serial.println(VERSION);

    // Pinmodes
    pinMode(PWR_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(PIN_DTR, OUTPUT);
    pinMode(V_BATT_PIN, INPUT);

    digitalWrite(LED_PIN, LOW); //Led on on the startup
    digitalWrite(PIN_DTR, LOW);

    // power SIM7000
    pinMode(PWR_PIN, OUTPUT);
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
    SerialAT.flush();
    digitalWrite(PWR_PIN, HIGH);
    delay(300);
    digitalWrite(PWR_PIN, LOW);
    SerialMon.println("Wait...");
    delay(6000);
    
    if(bootCount > 1) // Wake up from deepsleep
    {
        SerialMon.println("Wake up from sleep");
        digitalWrite(PWR_PIN, LOW);
        modem.sleepEnable(false);
    }
    else
    {
        modem.sleepEnable(false);
        SerialMon.println("Initializing modem...");
        modem.restart();
        delay(1000);
    }
    

    digitalWrite(LED_PIN, HIGH); // turn internal led off
    
    String modemInfo = modem.getModemInfo();
    SerialMon.print("Modem Info: ");
    SerialMon.println(modemInfo);

    if(bootCount == 1) // first time booting
    {
        enableGPS(); // enable GPS here to get it working as fast as possible
        
        // force modem to auto network mode
        modem.setNetworkMode(38); //  2 = auto, 13=GSM, 38=LTE, 51=LTE+GSM
        modem.setPreferredMode(1); // 1=CAT-M, 2=NB-IoT, 3=CAT-M and NB-IoT
    }
    else
    {
        delay(1);
    }



    // get nodeID
    String imei = modem.getIMEI();
    imei.substring(8, 15).toCharArray(nodeId, 8);
    SerialMon.print("Node ID: ");
    SerialMon.println(nodeId);

    // set broker settings
    mqtt.setServer(SECRET_BROKER_HOST, 1883);
    mqtt.setKeepAlive(120);
    mqtt.setCallback(mqttCallback);
}

float get_speed()
{
    float speed = 0;
    modem.getGPS(nullptr,
                 nullptr,
                 &speed,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr);
    return speed;
}

void update_location()
{
    old_gnss_data = current_gnss_data;

    float lat = 0;
    float lon = 0;
    float speed = 0;
    float alt = 0;
    float accuracy = 0;

    modem.getGPS(&lat,
                 &lon,
                 &speed,
                 &alt,
                 nullptr,
                 nullptr,
                 &accuracy,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr);

    current_gnss_data.lat = lat;
    current_gnss_data.lon = lon;
    current_gnss_data.speed = (unsigned int)speed;
    current_gnss_data.accuracy = (unsigned int)accuracy;
    current_gnss_data.fix = !(lat == 0 && lon == 0 && speed == 0 && alt == 0);
    current_gnss_data.last_fix = millis();

    SerialMon.println(modem.getGPSraw());

    if (current_gnss_data.fix != old_gnss_data.fix)
        updateValue("fix", current_gnss_data.fix);

    if (current_gnss_data.fix) // Send location only if there is a GPS fix
    {
        if (current_gnss_data.lat != old_gnss_data.lat
            || current_gnss_data.lon != old_gnss_data.lon) {
            char pos_buf[50];
            String pos_string = String(lat, 6) + "," + String(lon, 6);
            pos_string.toCharArray(pos_buf, 50);
            updateValue("position", pos_buf);
        }

        if (current_gnss_data.speed != old_gnss_data.speed)
            updateValue("speed", current_gnss_data.speed);

        if (current_gnss_data.accuracy != old_gnss_data.accuracy)
            updateValue("accuracy", current_gnss_data.accuracy);
    }

    // restart gnss if no fix for a long time
    if (!current_gnss_data.fix && current_gnss_data.last_fix + 300000 < millis()) {
        disableGPS();
        delay(200);
        enableGPS();
        // zero out the timer
        current_gnss_data.last_fix = millis();
    }
}

void update_sysinfo()
{
    old_sysinfo_data = current_sysinfo_data;

    float voltage = get_avg_voltage();
    int raw_soc = map(get_avg_voltage(), 2770, 3500, 0, 100);
    current_sysinfo_data.soc = (unsigned int)constrain(raw_soc, 0, 100);
    current_sysinfo_data.charging = raw_soc < 0 ? 1 : 0;
    current_sysinfo_data.signal = (unsigned int)(modem.getSignalQuality() * 827 + 127) >> 8;

    reset_voltage_avg();

    SerialMon.println("Send sysinfo here");
    if (current_sysinfo_data.soc != old_sysinfo_data.soc) {
        updateValue("soc", current_sysinfo_data.soc);
        updateValue("voltage", voltage);
    }

    if (current_sysinfo_data.charging != old_sysinfo_data.charging)
        updateValue("charging", current_sysinfo_data.charging);

    if (current_sysinfo_data.signal != old_sysinfo_data.signal)
        updateValue("signal", current_sysinfo_data.signal);

    sysinfo_timestamp = millis();
}

void add_voltage_to_avg()
{
    voltage_buf += map(analogRead(V_BATT_PIN), 0, 4095, 0, 6600);
    voltage_buf_i++;
}

float get_avg_voltage()
{
    return voltage_buf / voltage_buf_i;
}
void reset_voltage_avg()
{
    voltage_buf = 0;
    voltage_buf_i = 0;
}

void loop()
{
    if (old_mode == 3 && mode != 3) {
        // turn gps back on
        enableGPS();
    }

    switch (mode) {
        case 1: // normal opeating mode

            if (mqtt.connected()) {
                if (millis() > location_interval * 1000 + location_timestamp && mqtt.connected()) {
                    update_location();
                    location_timestamp = millis();
                } else if (millis() > sysinfo_interval * 1000 + sysinfo_timestamp
                           && mqtt.connected()) {
                    update_sysinfo();
                    sysinfo_timestamp = millis();
                }
            }
            break;

        case 2: // dynamic position sending
            if (millis() > dynamic_interval * 1000 + location_timestamp) {
                int accuracy = 30; // meters
                dynamic_interval = constrain((accuracy / (get_speed() * 0.51444444)), 2, 60);
                update_location();
                location_timestamp = millis();
            }
            break;

        case 3:
            if (old_mode != 3)
                disableGPS();
            break;
        
        case 4: // sleep
            disableGPS();
            updateValue("sleep", 1);
            mqtt.disconnect();
            modem.gprsDisconnect();
            modem.sleepEnable(true);
            digitalWrite(PIN_DTR, HIGH);
            
            pinMode(25, OUTPUT);
            digitalWrite(25, HIGH);
            rtc_gpio_hold_en(GPIO_NUM_0);

            esp_sleep_enable_timer_wakeup(30 * 60 * uS_TO_S_FACTOR);
            esp_deep_sleep_start();
            break;
        
        case 5: // check only status, no power to gnss
            delay(100);
            break;

        case 6: // Power off modem and go to deep sleep
            disableGPS();
            mqtt.disconnect();
            modem.gprsDisconnect();
            // AT+CPOWD=1
            modem.sendAT(GF("+CPOWD=1"));
            pinMode(25, OUTPUT);
            digitalWrite(25, HIGH);
            esp_deep_sleep_start();
            break;

        /*
    case 4: // power saving mode
        if(old_mode != 4) // first time in power saving mode
        {
            disableGPS();
            //SIM7000 to sleepmode(disconnect MQTT, disconnect client, sim7000 sleep)
            //updateValue("sleep", 1);

            //mqtt.disconnect();
            //modem.gprsDisconnect();
            //digitalWrite(PIN_DTR, HIGH);
            //sleep_timestamp = millis();
            //delay(10000);
            //esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR);
            //esp_deep_sleep_start();

            // go to deepsleep?
        }
        else
        {
            if(millis() > sleep_timestamp + 60*1000)
            {
                // wake up sim7000
                //digitalWrite(PIN_DTR, LOW);
                //delay(60);
                //SerialMon.println("Modem woken up!");
                //updateValue("sleep", 0);
                enableGPS();
                mode = 1;
                break;

                // wait for response
                // connect client
                // connect MQTT
                // MQTT loop
                // has value changed

            }
            // if sleeptimer passed
            // wake up and check MQTT
            // if mode changed break
            // else go back to sleep



            // still in mode 4 --> go back to sleep
        }
        // if sleeptimer has passed, wake sim7000
        // connect client and MQTT
        //MQTT loop



        // if enough time has passed(5 min), connect back to network and check if status has changed

        // change mode if something changed, else go back to sleep
    
    */
        default: break;
    }
    old_mode = mode;

    if (millis() > sysinfo_interval * 1000 + sysinfo_timestamp && mqtt.connected()) {
        update_sysinfo();
        sysinfo_timestamp = millis();
    }
    add_voltage_to_avg();

    // calculate how much time to next sending
    // if value bigger than x, put gps to sleep
    // if in sleep and sending is closing --> wake up gps
    // try to get gps fix for 1/5 of sending interval, after that go to sleep

    // check mobile network
    if (!modem.isNetworkConnected()) {
        SerialMon.println("No cellular connection");
        if (modem.waitForNetwork(5000L))
            SerialMon.println("Connected back to cellular network");
    }

    // check gprs connection
    if (!modem.isGprsConnected()) {
        modem.gprsConnect(APN, GPRS_USER, GPRS_PASS);
        SerialMon.println("GPRS connection re-established");
    }

    // check MQTT connection
    if (!mqtt.connected()) {
        SerialMon.println("=== MQTT NOT CONNECTED ===");
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 10000L) {
            lastReconnectAttempt = t;
            if (mqttConnect()) {
                lastReconnectAttempt = 0;
                mqttConnectionCount++;
            }
        }
        delay(100);
    }
    mqtt.loop();
}
