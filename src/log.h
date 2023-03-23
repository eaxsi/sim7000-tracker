#pragma once


#ifdef NDEBUG
#define DEBUG_PRINTF(label, format, args...) (void)(0)
#else
#define DEBUG_PRINTF(label, format, args...) do { Serial.printf("[" label "] " format " (%s:%d:%s)\r\n", ##args, __FILE__, __LINE__, __func__); Serial.flush(); } while(0)
#endif

#define INFO(format, args...) DEBUG_PRINTF("  INFO ", format, ##args)
#define WARNING(format, args...) DEBUG_PRINTF("WARNING", format, ##args)
#define ERROR(format, args...) DEBUG_PRINTF(" ERROR ", format, ##args)
