#include "settings.h"

Settings::Settings()
{
}

system_mode Settings::get_mode()
{
    return m_mode;
}

void Settings::set_mode(system_mode mode)
{
    m_mode = mode;
}

uint32_t Settings::get_periodic_tracking_interval()
{
    return m_periodic_tracking_interval;
}

void Settings::set_periodic_tracking_interval(uint32_t interval)
{
    m_periodic_tracking_interval = interval;
}
