# Development environment
- Create a file called `config.h`

The file should have the following structure
```
#define PROJECT_NAME "tracker"
#define BROKER_HOST ""
#define BROKER_PORT 1883
#define BROKER_USER ""
#define BROKER_PASSWD ""

#define TINY_GSM_USE_GPRS true
#define GSM_PIN ""
#define APN "iot.1nce.net" //  1NCE: iot.1nce.net, Elisa/Telia/DNA: internet
#define GPRS_USER ""
#define GPRS_PASS ""

#define HTTP_OTA_URL ""
```

# Settings
- Mode
    - 0: Hibernate, wake up only with magnet
    - 1: Sleep, Go online every 2 hours, also wake up by magnet
    - 2: Track, Send device position if device sensed movement
    - ~~3: OTA, update device firmware via wifi~~(Not implemented yet)
    - 4: Idle, stay connected to the MQTT Broker

enum class system_mode { hibernate, sleep, track, ota, idle };


Library versions
- TinyGSM: v0.11.7
- PubSubClient: v2.8.0


# Messages sent to the MQTT Broker
- Location update, location related data
- status update, battery soc, cellular signal strength, battery charging status
- Settings request: request mode setting
- other data: error, software version
