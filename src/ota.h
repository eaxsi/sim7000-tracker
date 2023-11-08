#pragma once

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <time.h>

#include "../config.h"
#include "common.h"
#include "log.h"

class Ota
{
    public:
        Ota();
        bool try_to_connect_to_wifi(wifi_details *);
        void start();
    private:
        wifi_details m_wifi_details;
};
