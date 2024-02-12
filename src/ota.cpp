#include "ota.h"

Ota::Ota()
{

}

bool Ota::try_to_connect_to_wifi(wifi_details * ota_wifi)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ota_wifi->wifi_ssid, ota_wifi->wifi_passwd);

    uint32_t wifi_connection_start_timestamp = millis();
    while((util::get_time_diff(wifi_connection_start_timestamp) < 15*1000) && WiFi.status() != WL_CONNECTED)
    {
        INFO("Connecting to Wifi...");
        delay(500);
    }
    return (WiFi.status() == WL_CONNECTED);
}

void Ota::start()
{
    WiFiClient updater_client;
    updater_client.setTimeout(12000);

    t_httpUpdate_return ret = httpUpdate.update(updater_client, HTTP_OTA_URL);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Serial.println("HTTP_UPDATE_OK");
            break;
    }
    WiFi.disconnect();
}
