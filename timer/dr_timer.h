/*
 * Driver: dr_timer.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 16.03.2014
 * Description: 
 * This is the driver header file. It offers all needed functions to
 * configure a timer.
 *
 * This file also provides an enumeration called Timer. These enums can
 * be used to choose the correct timer for the function calls.
 *
 * NOTE: TIMER 7 will be used as delay timer!
 */

#ifndef DR_TIMER_H_
#define DR_TIMER_H_

#include <inttypes.h>

typedef enum {
	Timer_TIMER1MS = 1,
	Timer_TIMER2,
	Timer_TIMER3,
	Timer_TIMER4,
	Timer_TIMER5,
	Timer_TIMER6,
	Timer_TIMER7
} Timer;

typedef void (*InterruptRoutine)(void);

int32_t TimerEnable(Timer timer);

int32_t TimerDisable(Timer timer);

int32_t TimerReset(Timer timer);

int32_t TimerConfiguration(Timer timer, uint32_t milliseconds, InterruptRoutine routine);

void TimerDelaySetup();
void TimerDelayDelay(uint32_t milliSec);
void TimerDelayStart(uint32_t millisec);
void TimerDelayStop();
uint32_t TimerDelayIsElapsed();

#endif /* DR_TIMER_H_ */
