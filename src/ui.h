#pragma once

#include "platform.h"

class Ui
{
    public:
        enum state {
            off,
            half_blink,
            single_blink,
            two_blinks,
            full_on
        };

        Ui(platform* platform);
        void set_state(state);
        void Ui_task();


    private:
        state m_state;
        platform *m_platform;
};