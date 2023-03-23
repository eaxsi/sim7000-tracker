# include "device.h"
/*
Device::deep_sleep()
{
    // Wake up only from reed switch
    esp_sleep_enable_ext0_wakeup(REED_PIN, 1);
    rtc_gpio_pullup_en();
    esp_deep_sleep_start();
}

Device::sleep()
{
    uint32_t pin_bitmask = 0;
    pin_bitmask = (1 << REED_PIN) | (1 << ACC_SENSOR_PIN);
    //Wake from reed and movement
    rtc_gpio_pullup_en();
    esp_sleep_enable_ext1_wakeup(pin_bitmask, ESP_EXT1_WAKEUP_ANY_HIGH);

    rtc_gpio_pullup_en();
    esp_light_sleep_start();
}
*/