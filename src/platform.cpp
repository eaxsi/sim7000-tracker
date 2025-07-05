#include "platform.h"

#define uS_TO_S_FACTOR 1000000LL  /* Conversion factor for micro seconds to seconds */

platform::platform()
{
    pinMode(REED_PIN, INPUT_PULLUP);
    m_led.turn_on();

    setCpuFrequencyMhz(80); // Save enery by lowering CPU-frequency
}

platform::event platform::get_event()
{
    m_oldpinstates = m_pinstates;
    m_pinstates.charger = m_battery.is_charging();

    if (m_vibration_sensor.onChange()) {
        return event::movement;
    } else if (m_reed_sensor.onRisingEdge()) {
        m_sensor_hold_timestamp = millis();
        while (m_reed_sensor.get_state() == false && !m_vibration_sensor.onChange()) // while reed sensor is activated
        {
            m_reed_sensor.update();
            m_vibration_sensor.update();
            if (util::get_time_diff(m_sensor_hold_timestamp) > 10 * 1000) {
                m_led.turn_off();
                delay(100);
                m_led.turn_on();
                delay(100);
                m_led.turn_off();

                return event::long_magnet_hold;
            }
        }
        return event::magnet;
    } else if (!m_pinstates.charger && m_pinstates.charger != m_oldpinstates.charger) {
        return event::charger_plugged;
    } else if (m_pinstates.charger && m_pinstates.charger != m_oldpinstates.charger) {
        return event::charger_unplugged;
    } else {
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
            //esp_sleep_enable_ext1_wakeup((gpio_num_t)V_BATT_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
            break;
        case wake_up_device::movement: // light sleep
            rtc_gpio_pullup_en((gpio_num_t)LED_PIN); // Turn LED off during sleep
            rtc_gpio_pullup_en((gpio_num_t)ACC_SENSOR_PIN);
            esp_sleep_enable_ext0_wakeup(
                (gpio_num_t)ACC_SENSOR_PIN,
                !m_vibration_sensor
                     .get_state()); //1 = Low to High, 0 = High to Low. Pin pulled HIGH
            break;
        default: break;
    }
}
void platform::sleep()
{
    m_led.turn_off();
    vTaskSuspendAll();
    esp_light_sleep_start();
    xTaskResumeAll();
    m_led.turn_on();
}

void platform::sleep(uint32_t timeout)
{
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    esp_sleep_enable_timer_wakeup(uint64_t(timeout) * uS_TO_S_FACTOR);
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

void platform::restart()
{
    delay(100); // finish sending serial data
    deep_sleep(1); // program starts from the beginning but RTC memory is not erased
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

void platform::turn_off_led()
{
    m_led.turn_off();
}

void platform::turn_on_led()
{
    m_led.turn_on();
}

void platform::update()
{
    m_vibration_sensor.update();
    m_reed_sensor.update();
    m_battery.update();
}
