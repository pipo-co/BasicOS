// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <rtcDriver.h>

uint8_t getTime(uint64_t timeDescriptor){
    uint8_t aux = getRtc(timeDescriptor);
    return aux / 16 * 10 + aux  %16;   
}
