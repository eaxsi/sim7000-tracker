#include "util.h"

uint32_t util::get_time_diff(uint32_t timestamp)
{
    const uint32_t diff = millis() - timestamp;
    return diff < UINT32_C(0x7FFFFFFF) ? diff : (UINT32_MAX - diff);
}
