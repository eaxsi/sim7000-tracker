#include "sysclock.h"

uint32_t SysClock::get_time_ms()
{
    return millis();
}

uint32_t SysClock::get_time_diff(uint32_t timestamp)
{
    const uint32_t diff = get_time_ms() - timestamp;
    return diff < UINT32_C(0x7FFFFFFF) ? diff : (UINT32_MAX - diff);
}
