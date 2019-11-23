#ifndef __TIMER_TICK_H__
#define __TIMER_TICK_H__

#include <stdint.h>

void timer_handler();
int ticks_elapsed();
int seconds_elapsed();
void timer_wait(uint16_t time);

#endif
