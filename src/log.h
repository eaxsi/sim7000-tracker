#pragma once

#define INFO(x); Serial.print(F("[  INFO ] ")); Serial.println(F(x))
#define INFO_VALUE(x, y); Serial.print(F("[  INFO ] ")); Serial.print(F(x)); Serial.println(y)
#define DEBUG_VALUE(value) Serial.println(value)
#define WARNING(value) Serial.print(F("[WARNING] ")); Serial.println(value)
#define ERROR(value) Serial.print(F("[ ERROR ] ")); Serial.println(value)
