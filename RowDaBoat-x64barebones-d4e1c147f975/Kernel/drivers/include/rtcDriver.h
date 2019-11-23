#ifndef __RTC_DRIVER_H__
#define __RTC_DRIVER_H__

#include <stdint.h>

extern uint8_t getRtc(uint64_t timeDescriptor);

uint8_t getTime(uint64_t timeDescriptor);

#endif