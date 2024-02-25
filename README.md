# SIM7000 Tracker
Arduino project based on the LilyGO-T-SIM7000G board with some additional sensors added.
The trakcer sends location info from the device to a MQTT broker via LTE connection.

# Hardware
- LilyGO-T-SIM7000G
- Added reed sensor between pin GPIO0 and GND, reed sensor 10mm
- added SENSOLUTE MVS0409.02 vibration sensor between pins GPTIO32 and GND: Datasheet in doc-folder
- External charger connector SP11 connected to VIN and GND

A 3D-printable case can be found from [Printables.com: LilyGO T-SIM7000G case](https://www.printables.com/model/768589-lilygo-t-sim7000g-case)


 ![Picture of modded board](/doc/modified_lilygo_t-sim7000g.jpeg)

# Compilation
This project has been made to work with the Arduino IDE environment. The recommended Arduino IDE version is v2.3.0 and above.

This project depends on the following Arduino libraries. These libraries can be installed from the Arduino IDE: Sketch --> Include libraries --> Manage libraries
- [TinyGSM](https://github.com/vshymanskyy/TinyGSM) (v0.11.7)by Volodymyr Shymanskyy
- [PubSubClient](https://github.com/knolleary/pubsubclient)(v2.8) by Nick O'Leary

This project also depends on the ESP32 board configurations in the Arduino IDE. They can be installed by adding an additional board manager URL. Go to File --> Preferences in the Arduino IDE and add the following URL to the additional boards manager URLs field:
```https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json```


# Config file
Edit the config.h file
```cpp
#define PROJECT_NAME "tracker" // Project name, is used in MQTT topic
#define BROKER_HOST "" // IP or address of the MQTT-broker
#define BROKER_PORT 1883 // MQTT-broker port, MQTT default port is 1883
#define BROKER_USER "" // MQTT-broker username
#define BROKER_PASSWD "" // MQTT-broker password

#define GSM_PIN "" // PIN-code of the SIM-card, if no PIN-code leave empty ""
#define APN "iot.1nce.net" // Carrier APN for internet, 1NCE: iot.1nce.net, Elisa/Telia/DNA: internet
#define GPRS_USER "" // Username for the cellular internet connection
#define GPRS_PASS "" // Password for the cellular internet connection

#define HTTP_OTA_URL "" // Address to the the OTA-update file
```


# Communication
The tracker communicates using the MQTT protocol, which is a publish/subscribe -type protocol. 

The tracker uses also HTTP for OTA-update downloading.

## Tracker ID
Tracker ID id an unique 7-numbered identifier for the tracker. It is made from the cellular modems IMEI-number.

## Connecting
When the tracker connects to the tracker it updates the connected topic to 1. If the tracker connects to the broker first time after boot, it sends the  installed software version.

## Status
The tracker sends periodically a status message that contains the battery state of charge(SOC), cellular signal strength and charging status. These values are sent as a comma separated string.

| Order in message | Data                     | Datatype | Unit |
|------------------|--------------------------|----------|------|
| 0                | Battery State of charge  | uint8    | %    |
| 1                | Cellular signal strength | uint8    | %    |
| 2                | Charger status           | bool     |      |

Example message: ```55,90,0``` 
- Battery SOC 55%
- Cellular signal strength 90%
- Charger not connected


## Location
Location update contains the following fields: Latitude, Longitude, Speed, Altitude, Accuracy, Course, Visible satellites, Used satellites
Latitude and longitude are degrees and in decimal format. 
The data is comma separated in the message.

| Order in message | Data               | Datatype | Unit    |
|------------------|--------------------|----------|---------|
| 0                | Latitude           | float    | degrees |
| 1                | Longitude          | float    | degrees |
| 2                | Speed              | float    | km/h    |
| 3                | Altitude           | float    | m       |
| 4                | Accuracy           | uint16   | m       |
| 5                | Course             | float    | degrees |
| 6                | Visible satellites | uint8    |         |
| 7                | Used satellites    | uint8    |         |

Example message: ```60.169900,24.938400,0.000000,14.500000,1.800000,194,23,4```
- Latitude: 60.169900 deg
- Longitude: 24.938400 deg
- Speed: 0.000000 km/h
- Altitude: 14.500000 m
- Accuracy: 1.800000 m
- Course: 194 deg
- 23 visible satellites
- 4 satellites used

## Settings
The device listens the MQTT broker for settings which in this case is the system mode.
The tracker has the following working modes:
- 0: Hibernate, wake up only with magnet
- 1: Sleep, Go online every 60 minutes, also wake up by magnet
- 2: Track, Send device position if device sensed movement
- ~~3: OTA, update device firmware via wifi~~(Entered by giving OTA wifi details)
- 4: Idle, stay connected to the MQTT Broker


# Extras
The repository also includes an updater utility to help with updating multiple devices. The updater is located in the util-folder. In order to use the updater, the MQTT-broker details needs to be set to the config.ini file.