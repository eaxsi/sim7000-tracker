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
