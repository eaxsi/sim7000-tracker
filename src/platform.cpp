#include "platform.h"

platform::platform()
{
    //Pinmodes: voltagePin, accPin, reedPin
    pinMode(REED_PIN_ARD, INPUT_PULLUP);
    pinMode(ACC_SENSOR_PIN_ARD, INPUT_PULLUP);
    pinMode(V_BATT_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);

    setCpuFrequencyMhz(80); // Save enery by lowering CPU-frequency

    // init battery voltage
    m_battery_voltage = get_raw_battery_voltage();
}

platform::event platform::get_event()
{
    m_oldpinstates = m_pinstates;

    m_pinstates.magnet = digitalRead(REED_PIN_ARD);
    m_pinstates.acc = digitalRead(ACC_SENSOR_PIN_ARD);
    m_pinstates.charger = analogRead(V_BATT_PIN) > 30;

    if(m_pinstates.acc && m_pinstates.acc != m_oldpinstates.acc)
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
            rtc_gpio_pullup_en((gpio_num_t)LED_PIN); // Turn LED off
            rtc_gpio_pullup_en(REED_PIN);
            rtc_gpio_pullup_en(GPIO_NUM_12);
            //rtc_gpio_pulldown_en(GPIO_NUM_4); // TODO
            esp_sleep_enable_ext0_wakeup((gpio_num_t)REED_PIN_ARD, 0);
            break;
        case wake_up_device::movement: // light sleep
            rtc_gpio_pullup_en((gpio_num_t)LED_PIN); // Turn LED off
            rtc_gpio_pullup_en((gpio_num_t)32);
            esp_sleep_enable_ext0_wakeup((gpio_num_t)32,1); //1 = Low to High, 0 = High to Low. Pin pulled HIGH            
            break;
        default: break;
    }
}
void platform::sleep()
{
    digitalWrite(LED_PIN, HIGH);
    esp_light_sleep_start();
    digitalWrite(LED_PIN, LOW);
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
    int raw_soc = map(m_battery_voltage, 2400, 3560, 0, 100);
    return (uint8_t)constrain(raw_soc, 0, 100);
}

float platform::get_voltage()
{
    return m_battery_voltage;
}

float platform::get_raw_battery_voltage()
{
    return map(analogRead(V_BATT_PIN), 0, 4095, 0, 6600);
}

bool platform::charging()
{
    return m_battery_voltage < 100;
}

void platform::update()
{
    if (util::get_time_diff(m_voltage_measurement_timemstamp) > m_voltage_measurement_interval) {
        m_battery_voltage = 0.1f * get_raw_battery_voltage() + 0.9f * m_battery_voltage;
        m_voltage_measurement_timemstamp = millis();
    }
    // Check if

    // Raising edge magnet

    // Raising edge of motion sensor

    // Charger plugged

    // Charget disconnected
}
