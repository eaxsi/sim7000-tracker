#pragma once

#include "Arduino.h"

enum class system_mode { hibernate, sleep, track, ota, idle };
enum system_event { movement, magnet, charger_plugged, change_of_mode };

struct location_update
{
        float lat;
        float lon;
        float speed;
        float alt;
        float accuracy;
        uint16_t course;
        int vsat;
        int usat;
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
};

struct wifi_details{
        char wifi_ssid[50] = "";
        char wifi_passwd[50] = "";
};
