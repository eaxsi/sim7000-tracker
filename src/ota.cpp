#include "ota.h"

ota::ota()
{
}

bool ota::try_to_connect_to_wifi(wifi_details* ota_wifi)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ota_wifi->wifi_ssid, ota_wifi->wifi_passwd);

    uint32_t wifi_connection_start_timestamp = millis();
    while ((util::get_time_diff(wifi_connection_start_timestamp) < 15 * 1000)
           && WiFi.status() != WL_CONNECTED) {
        INFO("Connecting to Wifi...");
        delay(500);
    }
    return (WiFi.status() == WL_CONNECTED);
}

ota::status ota::start()
{
    status ota_status = status::none;
    WiFiClient updater_client;
    updater_client.setTimeout(12000);
    httpUpdate.setLedPin(LED_PIN, LOW);

    t_httpUpdate_return ret = httpUpdate.update(updater_client, HTTP_OTA_URL);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            ota_status = status::ota_error;
            break;

        case HTTP_UPDATE_NO_UPDATES:
            INFO("HTTP_UPDATE_NO_UPDATES");
            ota_status = status::ota_fail;
            break;

        case HTTP_UPDATE_OK:
            INFO("HTTP_UPDATE_OK");
            ota_status = status::success;
            break;
    }
    WiFi.disconnect();
    return ota_status;
}
