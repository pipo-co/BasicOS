#ifndef __TIMER_TICK_H__
#define __TIMER_TICK_H__

void timer_handler();
int ticks_elapsed();
int seconds_elapsed();
void timer_wait(int time);

#endif
