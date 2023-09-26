#include "buttons.h"

/*
void Buttons::update()
{
    uint8_t i;
    for(i = 0; i < 4; i++)
    {
        if(m_button_lock[i] && !m_buttons[i].is_pushed())
            m_button_lock[i] = false;
    }
}

uint8_t Buttons::which_button_pressed()
{
    uint8_t i;
    for(i = 0; i < 4; i++)
    {
        if(m_buttons[i].is_pushed() && !m_button_lock[i])
        {
            m_button_lock[i] = true;
            return i+1;
        }
    }
    return 0;
}

bool Buttons::all_pressed()
{
    bool ret = true;
    uint8_t i;
    for(i = 0; i < 4; i++)
    {
        if(!m_buttons[i].is_pushed())
            ret = false;
    }
    return ret;
}
*/
