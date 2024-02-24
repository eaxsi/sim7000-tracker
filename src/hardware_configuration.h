#pragma once

// SIM7000 settings
#define TINY_GSM_MODEM_SIM7000
#define MODEM_BAUDRATE 9600
#define PIN_DTR 25
#define MODEM_SERIAL_TX_PIN 27
#define MODEM_SERIAL_RX_PIN 26
#define MODEM_POWER_PIN 4
#define MODEM_RESET_PIN 5
#define MODEM_SERIAL Serial1

#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13
#define LED_PIN 12
#define V_BATT_PIN 35

#define REED_PIN 0
#define ACC_SENSOR_PIN 32

#define BATTERY_CAPACITY 3400 //mAh
#define CHARGING_CURRENT 900 //mA
