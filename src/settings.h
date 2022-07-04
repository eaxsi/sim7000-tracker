#pragma once

#include "Arduino.h"


class Settings
{
    public:
    private:
        position_interval;
        sysinfo_interval;
        enum m_mode { hibernate = 1, normal = 2, always_on = 3, power_save = 4 };
}