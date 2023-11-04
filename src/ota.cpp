#include "ota.h"

Ota::Ota()
{

}

bool Ota::try_to_connect_to_wifi(wifi_details * ota_wifi)
{
    //strcpy(wifi_ssid, ota_wifi->wifi_ssid);
    //strcpy(wifi_passwd, ota_wifi->wifi_passwd);
    //Serial.println(ota_wifi->wifi_ssid);
    //Serial.println(ota_wifi->wifi_passwd);
    //Serial.println(wifi_ssid);
    //Serial.println(wifi_passwd);

    /*
    long wifi_connecting_timestamp = millis();
    while (WiFi.status() != WL_CONNECTED || wifi_connecting_timestamp + 15 * 1000 < millis()) {
        delay(500);
        INFO("Connecting to WiFi...");
    }
    return (WiFi.status() == WL_CONNECTED);
    */
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
