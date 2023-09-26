

#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <time.h>

#include "../config.h"

class Ota
{
    public:
        Ota();

        // set wifi details
        void set_wifi_details();

        bool wifi_network_exits();

        bool try_to_connect_to_wifi();

        void start_update();

        // check wifi exits

        // check the file

        // start update

        // reboot
    private:
        char m_wifi_ssid[50] = "";
        char m_wifi_passwd[50] = "";
};
