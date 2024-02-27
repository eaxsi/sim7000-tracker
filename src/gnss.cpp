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

bool Gnss::has_initial_fix()
{
    return m_initial_fix_received;
}

bool Gnss::is_moving()
{
    return m_loc.speed >= m_moving_speed_threshold;
}

bool Gnss::get_location(location_update* loc)
{
    m_old_loc = m_loc;

    bool fix = m_modem->getGPS(&m_loc.lat,
                               &m_loc.lon,
                               &m_loc.speed,
                               &m_loc.alt,
                               &m_loc.vsat,
                               &m_loc.usat,
                               &m_loc.accuracy,
                               &m_loc.year,
                               &m_loc.month,
                               &m_loc.day,
                               &m_loc.hour,
                               &m_loc.minute,
                               &m_loc.second);

    loc->lat = m_loc.lat;
    loc->lon = m_loc.lon;
    loc->alt = m_loc.alt;
    loc->vsat = m_loc.vsat;
    loc->usat = m_loc.usat;
    loc->accuracy = m_loc.accuracy;
    loc->speed = max(m_loc.speed, 0.0f);

    bool time_not_changed = m_loc.year == m_old_loc.year && m_loc.month == m_old_loc.month
                            && m_loc.day == m_old_loc.day && m_loc.hour == m_old_loc.hour
                            && m_loc.minute == m_old_loc.minute && m_loc.second == m_old_loc.second;
    bool location_not_changed
        = m_loc.lat == m_old_loc.lat && m_loc.lon == m_old_loc.lon && m_loc.alt == m_old_loc.alt;

    loc->course = get_bearing(m_old_loc.lat, m_old_loc.lon, m_loc.lat, m_loc.lon);
    m_device_stuck = time_not_changed && location_not_changed;

    return fix;
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

float Gnss::get_bearing(float lat, float lon, float lat2, float lon2)
{
    float teta1 = radians(lat);
    float teta2 = radians(lat2);
    float delta1 = radians(lat2 - lat);
    float delta2 = radians(lon2 - lon);

    float y = sin(delta2) * cos(teta2);
    float x = cos(teta1) * sin(teta2) - sin(teta1) * cos(teta2) * cos(delta2);
    float brng = atan2(y, x);
    brng = degrees(brng);
    brng = (((int)brng + 360) % 360);

    return brng;
}

void Gnss::update()
{
    switch (m_state) {
        case state::off: {
            if (m_requested_state > m_state) {
                turn_on_impl();
                m_state = state::no_fix;
            }
            break;
        }
        case state::no_fix: {
            if (m_requested_state < m_state) {
                //turn off
                turn_off_impl();
                m_state = state::off;
            } else if (has_fix_impl()) {
                m_state = state::fix;
                m_initial_fix_received = true;
            }
            break;
        }
        case state::fix: {
            if (m_requested_state < m_state) {
                // turn off
                turn_off_impl();
                m_state = state::off;
            }

            bool non_valid_data = m_loc.vsat > 10000 && m_loc.usat == 0;
            if (non_valid_data || m_device_stuck) {
                turn_off_impl();
                turn_on_impl();
                m_state = state::no_fix;
            }
            break;
        }
        default: break;
    }
}
