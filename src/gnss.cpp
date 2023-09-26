#include "gnss.h"

Gnss::Gnss(TinyGsm* m)
{
    m_modem = m;
}

void Gnss::turn_on()
{
    m_requested_state = state::fix;
}

void Gnss::turn_off()
{
    m_requested_state = state::off;
}

bool Gnss::is_on()
{
    return m_state != state::off;
}

bool Gnss::has_fix()
{
    return m_state == state::fix;
}

bool Gnss::get_location(location_update* loc)
{
    return m_modem->getGPS(&loc->lat,
                    &loc->lon,
                    &loc->speed,
                    &loc->alt,
                    nullptr,
                    nullptr,
                    &loc->accuracy,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr);
}

bool Gnss::turn_on_impl()
{
    m_modem->sendAT("+SGPIO=0,4,1,1"); // Power on active antenna
    if (m_modem->waitResponse(1000L) != 1) {
        return false;
    }

    return m_modem->enableGPS();
}

bool Gnss::turn_off_impl()
{
    m_modem->sendAT("+SGPIO=0,4,1,0");
    if (m_modem->waitResponse(1000L) != 1) {
        return false;
    }

    return m_modem->disableGPS();
}

bool Gnss::has_fix_impl()
{
    return m_modem->getGPS(nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr,
                 nullptr);

}

void Gnss::update()
{
    switch (m_state) {
        case state::off:
        {
            if(m_requested_state > m_state)
            {
                turn_on_impl();
                m_state = state::no_fix;
            }
            break;
        }
        case state::no_fix:
        {
            if(m_requested_state < m_state)
            {
                //turn off
                turn_off_impl();
                m_state = state::off;
            }
            else if(has_fix_impl())
            {
                m_state = state::fix;
            }
            break;
        }
        case state::fix:
        {
            if(m_requested_state < m_state)
            {
                // turn off
                turn_off_impl();
                m_state = state::off;
            }
            break;
        }
        default: break;
    }
}
