#include "platform.h"

platform::platform()
{
    pinMode(REED_PIN, INPUT_PULLUP);
    m_led.turn_on();

    setCpuFrequencyMhz(80); // Save enery by lowering CPU-frequency
}

platform::event platform::get_event()
{
    m_oldpinstates = m_pinstates;

    m_pinstates.magnet = digitalRead(REED_PIN);
    m_pinstates.charger = m_battery.is_charging();

    if(m_vibration_sensor.activated())
    {
        return event::movement;
    }
    else if(!m_pinstates.magnet && m_pinstates.magnet != m_oldpinstates.magnet)
    {
        return event::magnet;
    }
    else if(!m_pinstates.charger && m_pinstates.charger != m_oldpinstates.charger)
    {
        return event::charger_plugged;
    }
    else if(m_pinstates.charger && m_pinstates.charger != m_oldpinstates.charger)
    {
        return event::charger_unplugged;
    }
    else
    {
        return event::none;
    }
}

void platform::set_wake_up_device(platform::wake_up_device wake_up_device)
{
    switch (wake_up_device) {
        case wake_up_device::magnet:
            rtc_gpio_pullup_en((gpio_num_t)LED_PIN); // Turn LED off during sleep
            rtc_gpio_pullup_en((gpio_num_t)REED_PIN);
            esp_sleep_enable_ext0_wakeup((gpio_num_t)REED_PIN, 0);
            break;
        case wake_up_device::movement: // light sleep
            rtc_gpio_pullup_en((gpio_num_t)LED_PIN); // Turn LED off during sleep
            rtc_gpio_pullup_en((gpio_num_t)ACC_SENSOR_PIN);
            esp_sleep_enable_ext0_wakeup((gpio_num_t)ACC_SENSOR_PIN, !m_vibration_sensor.get_state()); //1 = Low to High, 0 = High to Low. Pin pulled HIGH      
            break;
        default: break;
    }
}
void platform::sleep()
{
    m_led.turn_off();
    esp_light_sleep_start();
    m_led.turn_on();
}

void platform::sleep(uint32_t timeout)
{
    esp_sleep_enable_timer_wakeup(timeout * 1000 * 1000);
    sleep();
}

void platform::deep_sleep()
{
    esp_deep_sleep_start();
}

void platform::deep_sleep(uint32_t timeout)
{
    esp_sleep_enable_timer_wakeup(timeout * 1000 * 1000);
    deep_sleep();
}

uint8_t platform::get_soc()
{
    return m_battery.get_soc();
}

float platform::get_voltage()
{
    return m_battery.get_voltage();
}

bool platform::charging()
{
    return m_battery.is_charging();
}

void platform::update()
{
    if(m_battery.is_charging())
    {
        if(m_battery.get_soc() == 100)
            m_blink_duty_cycle = 100;
        else
            m_blink_duty_cycle = 50;

        //m_blink_duty_cycle = m_battery.get_soc() == 100 ? 100 : 50;
    }
    else
    {
        m_blink_duty_cycle = 10;
    }
    // Led blinking logic
    uint8_t place_in_duty_cycle = (millis() % m_led_blink_interval)*100 / m_led_blink_interval;
    if(place_in_duty_cycle > m_blink_duty_cycle && m_led.get())
    {
        m_led.turn_off();
    }
    else if(place_in_duty_cycle < m_blink_duty_cycle && !m_led.get())
    {
        m_led.turn_on();
    }

    m_vibration_sensor.update();
    m_battery.update();
}
