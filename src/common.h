#pragma once

enum class system_mode { hibernate, sleep, track, ota, idle };
enum system_event { movement, magnet, charger_plugged, change_of_mode };

struct location_update
{
        float lat;
        float lon;
        float speed;
        float alt;
        float accuracy;
};
