#include "ota.h"

bool Ota::try_to_connect_to_wifi()
{
    WiFi.begin(m_wifi_ssid, m_wifi_passwd);

    long wifi_connecting_timestamp = millis();
    while (WiFi.status() != WL_CONNECTED || wifi_connecting_timestamp + 15 * 1000 < millis()) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    return (WiFi.status() == WL_CONNECTED);
}

void Ota::start_update()
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
    //else updateValue("shadow/fota_status", "wifi_fail");
    WiFi.disconnect();
}