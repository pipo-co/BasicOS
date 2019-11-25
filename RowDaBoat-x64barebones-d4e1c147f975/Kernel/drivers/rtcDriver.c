//rtcDriver.c
#include <rtcDriver.h>

uint8_t getTime(uint64_t timeDescriptor){
    uint8_t aux = getRtc(timeDescriptor);
    return aux / 16 * 10 + aux  %16;   
}
