#pragma once

#include "Arduino.h"

class Logic
{
    public:
        enum Mode { normal, force, idle, sleep, off };
        void update();

    private:
        uint16_t m_update_interval;

        Mode m_mode;
        Mode m_last_mode;
        
};
