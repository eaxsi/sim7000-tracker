#pragma once

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <time.h>

#include "../config.h"
#include "common.h"
#include "log.h"
#include "util.h"
#include "hardware_configuration.h"

class ota
{
    public:
        enum status { none, wifi_failed, ota_fail, ota_error, success };
        ota();
        bool try_to_connect_to_wifi(wifi_details*);
        status start();

    private:
        wifi_details m_wifi_details;
};
