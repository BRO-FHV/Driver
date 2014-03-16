/*
 * Driver: dr_timer.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 16.03.2014
 * Description: 
 * TODO
 */

#include <inttypes.h>
#include <hw_timer.h>
#include <basic.h>
#include "dr_timer.h"

#define NUMBER_OF_TIMERS	8
#define RESET_VALUE			0x00


uint16_t timers[NUMBER_OF_TIMERS];

/**
 * \brief enable a timer
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int TimerEnable(Timer timer) {
	if(0 != timers[timer]) {
		return -1; //already configured
	}

	uint16_t baseAdr = GetTimerBaseAdr(timer);

	if(-1 == baseAdr) {
		return -1; //failure
	}

	if(1 == timer) {
		//TODO enable
	} else {
		//TODO enable
	}
}

/**
 * \brief disable a timer
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int TimerDisable(Timer timer) {
	uint16_t baseAdr = GetTimerBaseAdr(timer);

	if(-1 == baseAdr) {
		return -1; //failure
	}

	if(1 == timer) {
		//TODO disable
	} else {
		//TODO disable
	}

	timers[timer] = 0;

	return 0;
}


/**
 * \brief reset a timer (counting state)
 *
 * \param timer
 *
 * \return 0 on success, -1 on failure
 */
int TimerReset(Timer timer) {
	uint16_t baseAdr = GetTimerBaseAdr(timer);

	if(-1 == baseAdr) {
		return -1; //failure
	}

	if(1 == timer) {
		reg32w(baseAdr, TIMER1_TCRR, RESET_VALUE);
	} else {
		reg32w(baseAdr, TIMER_TCRR, RESET_VALUE);
	}

	return 0;
}

/**
 * \brief configure the compare mode and the auto reaload mode. Timermust be disabled
 *
 * \param timer
 * \param compareModeOn
 * \param autoReload
 *
 * \return 0 on success, -1 on failure
 */
int TimerConfigure(Timer timer, uint8_t compareModeOn, uint8_t autoReload) {
	if(0 < timers[timer]) {
		return -1;
	}

	uint16_t baseAdr = GetTimerBaseAdr(timer);

	if(-1 == baseAdr) {
		return -1; //failure
	}

	if(1 == timer) {
		//TODO configure
	} else {
		//TODO configure
	}
}


/**
 * \brief PRIVATE: returns the corresponding memory address of a timer
 *
 * \param timer
 *
 * \return the address on success, -1 on failure
 */
uint16_t GetTimerBaseAdr(Timer timer) {
	switch(timer) {
	case TIMER0:
		return TIMER_0;
	case TIMER1MS:
		return TIMER_1MS;
	case TIMER2:
		return TIMER_2;
	case TIMER3:
		return TIMER_3;
	case TIMER4:
		return TIMER_4;
	case TIMER5:
		return TIMER_5;
	case TIMER6:
		return TIMER_6;
	case TIMER7:
		return TIMER_7;
	default:
		return -1;
	}
}
