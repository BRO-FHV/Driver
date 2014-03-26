/*
 * Driver: dr_timer.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 16.03.2014
 * Description: 
 * This is the implementation file of the timer driver. It
 * offers different methods for configuration and enabling/disabling
 * a specific timer.
 *
 * A already enabled timer cannot be enabled or configured again. It must
 * be disabled first.
 *
 * All timers are configured with auto-reload, capture enabled, posted mode and
 * to raise an interrupt when the set time (passed as parameter to TimerConfiguration method=
 * is over.
 */
#include <stdio.h>
#include <inttypes.h>
#include <hw_timer.h>
#include "../interrupt/dr_interrupt.h"
#include <basic.h>
#include <hw_types.h>
#include "dr_timer.h"

#define NUMBER_OF_TIMERS	8
#define RESET_VALUE			0x00

/**
 * \brief   This macro will check for write POSTED status.
 *
 * \param   tsicr		Timer Synchronous Interface Control Register of a specific timer
 * \param   twps		Timer Write Posting Bits Register of a specific timer
 * \param   reg			Register whose status has to be checked
 * \param   baseAdd		Base address of the timer
 **/
#define WaitForWrite(tsicr, twps, reg, baseAdd) if(reg32r(baseAdd, tsicr) & TSICR_POSTED)\
            										while((reg & (reg32r(baseAdd, twps))));

uint16_t timers[NUMBER_OF_TIMERS];

typedef enum {
	IrqMode_MATCH = 0, IrqMode_OVERFLOW, IrqMode_CAPTURE, IrqMode_ALL
} IrqMode;

typedef enum {
	IrqWakeen_MAT_WUP_ENA = 0,
	IrqWakeen_OVF_WUP_ENA,
	IrqWakeen_TCAR_WUP_ENA,
	IrqWakeen_ALL
} IrqWakeen;

uint32_t GetTimerBaseAddr(Timer timer);
uint32_t GetTimerInterruptCode(Timer timer);

void SetIrqWakeenMode(uint32_t baseAddr, IrqWakeen irqwakeen, uint32_t irqWakeenRegister);
void SetIrqMode(uint32_t baseAddr, IrqMode irqMode, uint32_t irqRegister);

void EnablePostedMode(uint32_t baseAddr, uint32_t tsicr);
void EnableCore(Timer timer, uint32_t baseAddr, uint32_t tclr, uint32_t tsicr, uint32_t twps);
void DisableCore(Timer timer, uint32_t baseAddr, uint32_t tclr, uint32_t tsicr, uint32_t twps);
void ResetCore(uint32_t baseAddr, uint32_t tmar, uint32_t tldr, uint32_t twer, uint32_t tisr, uint32_t ttgr, uint32_t tclr, uint32_t tcrr, uint32_t tsicr, uint32_t twps);
void ConfigurationCore(uint32_t baseAddr, uint32_t tmar, uint32_t tldr,	uint32_t tisr, uint32_t ttgr, uint32_t tclr, uint32_t twer, uint32_t tier, uint32_t tsicr, uint32_t twps);

void ClearTimer0IrqStatus();
void ClearTimer1IrqStatus();
void ClearTimer2IrqStatus();
void ClearTimer3IrqStatus();
void ClearTimer4IrqStatus();
void ClearTimer5IrqStatus();
void ClearTimer6IrqStatus();
void ClearTimer7IrqStatus();
void ClearTimerCore(uint32_t baseAddr, uint32_t tisr);

/**
 * \brief Enable a timer. ST bit of TCLR is set to 1. No registers were reset!
 *
 * \param timer Timer that should be enabled.
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerEnable(Timer timer) {
	if (1 == timers[timer]) {
		return -1; //already enabled
	}

	uint32_t baseAddr = GetTimerBaseAddr(timer);

	if (UINT32_MAX == baseAddr) {
		return -1; //timer does not exist
	}

	uint32_t tcrr = 1 == timer ? TIMER1_TCRR : TIMER_TCRR;

	//reset counter register
	reg32wor(baseAddr, tcrr, RESET_VALUE);

	if (1 == timer) {
		EnableCore(timer, baseAddr, TIMER1_TCLR, TIMER1_TSICR, TIMER1_TWPS);
	} else {
		EnableCore(timer, baseAddr, TIMER_TCLR, TIMER_TSICR, TIMER_TWPS);
	}

	return 0;
}

void EnableCore(Timer timer, uint32_t baseAddr, uint32_t tclr, uint32_t tsicr, uint32_t twps) {
	//turn on timer
	reg32wor(baseAddr, tclr, TCLR_ST);

	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

	//set to enabled
	timers[timer] = 1;
}

/**
 * \brief Disable a timer. ST bit of TCLR is set to 0. No registers were reset!
 *
 * \param timer Timer that should be disabled.
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerDisable(Timer timer) {
	if (0 == timers[timer]) {
		return -1; //already disabled
	}

	uint32_t baseAddr = GetTimerBaseAddr(timer);

	if (UINT32_MAX == baseAddr) {
		return -1; //failure
	}

	if (1 == timer) {
		DisableCore(timer, baseAddr, TIMER1_TCLR, TIMER1_TSICR, TIMER1_TWPS);
	} else {
		DisableCore(timer, baseAddr, TIMER_TCLR, TIMER_TSICR, TIMER_TWPS);
	}

	return 0;
}

void DisableCore(Timer timer, uint32_t baseAddr, uint32_t tclr, uint32_t tsicr, uint32_t twps) {
	//shut down timer
	reg32an(baseAddr, tclr, TCLR_ST);

	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

	//disable interrupt routine
	uint32_t irqCode = GetTimerInterruptCode(timer);
	IntUnRegister(irqCode);
	IntHandlerDisable(irqCode);

	//set to disabled
	timers[timer] = 0;
}

/**
 * \brief Reset a timer (e.g. counting state, interrupt status,..)
 *
 * \param timer Timer that should be reset.
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerReset(Timer timer) {
	uint32_t baseAddr = GetTimerBaseAddr(timer);

	if (UINT32_MAX == baseAddr) {
		return -1; //failure
	}

	//reset registers
	if (1 == timer) {
		ResetCore(baseAddr, TIMER1_TMAR, TIMER1_TLDR, TIMER1_TWER, TIMER1_TISR, TIMER1_TTGR, TIMER1_TCLR, TIMER1_TCRR, TIMER1_TSICR, TIMER1_TWPS);
	} else {
		ResetCore(baseAddr, TIMER_TMAR, TIMER_TLDR, TIMER_IRQWAKEEN, TIMER_IRQSTATUS, TIMER_TTGR, TIMER_TCLR, TIMER_TCRR, TIMER_TSICR, TIMER_TWPS);
	}

	return 0;
}

void ResetCore(uint32_t baseAddr, uint32_t tmar, uint32_t tldr, uint32_t twer, uint32_t tisr, uint32_t ttgr, uint32_t tclr, uint32_t tcrr, uint32_t tsicr, uint32_t twps) {
	reg32wor(baseAddr, tmar, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TMAR, baseAddr);

	reg32wor(baseAddr, tldr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TLDR, baseAddr);

	reg32wor(baseAddr, twer, RESET_VALUE);

	reg32wor(baseAddr, ttgr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TTGR, baseAddr);

	reg32wor(baseAddr, tclr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr);

	reg32wor(baseAddr, tcrr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCRR, baseAddr);

	ClearTimerCore(baseAddr, tisr);
}

/**
 * \brief Configure the passed timer to raise an interrupt (after passed milliseconds).
 *
 * \param timer	Timer that should be configured.
 * \param milliseconds Time slice after which the interrupt is raised.
 * \param routine This routine is called to handle the interrupt.
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerConfiguration(Timer timer, uint32_t milliseconds, InterruptRoutine routine) {
	if (1 == timers[timer] || NULL == routine || 0 >= milliseconds) {
		return -1; //timer is already enabled or routine is not set
	}

	uint32_t baseAddr = GetTimerBaseAddr(timer);

	if (UINT32_MAX == baseAddr) {
		return -1; //failure
	}

	//TODO feature wish: no prescaler - pass e.g. 500 ms, 1000 ms,...
//	//TMAR should be less than 0xFFFF FFFF
//	if(UINT32_MAX == matchValue) {
//		return -1;
//	}

	if (1 == timer) {
		//timer 1 ms
		DisableCore(timer, baseAddr, TIMER1_TCLR, TIMER1_TSICR, TIMER1_TWPS);

		ResetCore(baseAddr, TIMER1_TMAR, TIMER1_TLDR, TIMER1_TWER, TIMER1_TISR, TIMER1_TTGR, TIMER1_TCLR, TIMER1_TCRR, TIMER1_TSICR, TIMER1_TWPS);

		ConfigurationCore(baseAddr, TIMER1_TMAR, TIMER1_TLDR, TIMER1_TISR, TIMER1_TTGR, TIMER1_TCLR, TIMER1_TWER, TIMER1_TIER, TIMER1_TSICR, TIMER_TWPS);
	} else {
		//timer 0, 2 - 7
		DisableCore(timer, baseAddr, TIMER_TCLR, TIMER_TSICR, TIMER_TWPS);

		ResetCore(baseAddr, TIMER_TMAR, TIMER_TLDR, TIMER_IRQWAKEEN, TIMER_IRQSTATUS, TIMER_TTGR, TIMER_TCLR, TIMER_TCRR, TIMER_TSICR, TIMER_TWPS);

		ConfigurationCore(baseAddr, TIMER_TMAR, TIMER_TLDR, TIMER_IRQSTATUS, TIMER_TTGR, TIMER_TCLR, TIMER_IRQWAKEEN, TIMER_IRQENABLE_SET, TIMER_TSICR, TIMER_TWPS);
	}

	//configure interrupt routine
	uint32_t irqCode = GetTimerInterruptCode(timer);
	IntRegister(irqCode, routine);
	IntHandlerEnable(irqCode);

	return 0;
}

void ConfigurationCore(uint32_t baseAddr, uint32_t tmar, uint32_t tldr,	uint32_t tisr, uint32_t ttgr, uint32_t tclr, uint32_t twer, uint32_t tier, uint32_t tsicr, uint32_t twps) {

	//enable posted mode for checking pending writes
	EnablePostedMode(baseAddr, tsicr);

	//defines the match value (e.g. interrupt is raised, if this value is reached)
	reg32wor(baseAddr, tmar, (1 << 17));
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TMAR, baseAddr)

	//defines where the timer should start to count (e.g. after a auto reload)
	reg32wor(baseAddr, tldr, 0x00);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TLDR, baseAddr)

	//clear pending interrupts
	reg32an(baseAddr, tisr, TISR_ALL_FLAGS);

	//Writing in the TTGR register, TCRR will be loaded from TLDR and prescaler counter will be cleared.
	//Reload will be done regardless of the AR field value of TCLR register.
	reg32wor(baseAddr, ttgr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TTGR, baseAddr)

	//set compare enabled and auto reload
	reg32wor(baseAddr, tclr, TCLR_CE + TCLR_AR);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

	SetIrqWakeenMode(baseAddr, IrqWakeen_MAT_WUP_ENA, twer);
	SetIrqMode(baseAddr, IrqMode_MATCH, tier);
}

void EnablePostedMode(uint32_t baseAddr, uint32_t tsicr) {
	/* Clear the POSTED field of TSICR */
	reg32an(baseAddr, tsicr, TSICR_POSTED);

	/* Write to the POSTED field of TSICR */
	reg32m(baseAddr, tsicr, TSICR_POSTED);
}

void SetIrqWakeenMode(uint32_t baseAddr, IrqWakeen irqwakeen, uint32_t irqWakeenRegister) {
	//Wakeup-enabled events taking place when module is idle will generate an asynchronous wakeup
	switch (irqwakeen) {
	case IrqWakeen_MAT_WUP_ENA:
		reg32wor(baseAddr, irqWakeenRegister, IRQWAKEEN_MAT_WUP_ENA);
		break;
	case IrqWakeen_OVF_WUP_ENA:
		reg32wor(baseAddr, irqWakeenRegister, IRQWAKEEN_OVF_WUP_ENA);
		break;
	case IrqWakeen_TCAR_WUP_ENA:
		reg32wor(baseAddr, irqWakeenRegister, IRQWAKEEN_TCAR_WUP_ENA);
		break;
	case IrqWakeen_ALL:
		reg32wor(baseAddr, irqWakeenRegister, IRQWAKEEN_MAT_WUP_ENA & IRQWAKEEN_OVF_WUP_ENA & IRQWAKEEN_TCAR_WUP_ENA);
		break;
	}
}

void SetIrqMode(uint32_t baseAddr, IrqMode irqMode, uint32_t irqRegister) {
	switch (irqMode) {
	case IrqMode_MATCH:
		reg32wor(baseAddr, irqRegister, IRQENABLE_MAT_EN_FLAG);
		break;
	case IrqMode_CAPTURE:
		reg32wor(baseAddr, irqRegister, IRQENABLE_TCAR_EN_FLAG);
		break;
	case IrqMode_OVERFLOW:
		reg32wor(baseAddr, irqRegister, IRQENABLE_OVF_EN_FLAG);
		break;
	case IrqMode_ALL:
		reg32wor(baseAddr, irqRegister, IRQENABLE_OVF_EN_FLAG & IRQENABLE_MAT_EN_FLAG & IRQENABLE_TCAR_EN_FLAG);
		break;
	default:
		break;
	}
}

uint32_t GetTimerBaseAddr(Timer timer) {
	switch (timer) {
	case Timer_TIMER0:
		return TIMER_0;
	case Timer_TIMER1MS:
		return TIMER_1MS;
	case Timer_TIMER2:
		return TIMER_2;
	case Timer_TIMER3:
		return TIMER_3;
	case Timer_TIMER4:
		return TIMER_4;
	case Timer_TIMER5:
		return TIMER_5;
	case Timer_TIMER6:
		return TIMER_6;
	case Timer_TIMER7:
		return TIMER_7;
	default:
		return UINT32_MAX;
	}
}

uint32_t GetTimerInterruptCode(Timer timer) {
	switch (timer) {
	case Timer_TIMER0:
		return SYS_INT_TINT0;
	case Timer_TIMER1MS:
		return SYS_INT_TINT1_1MS;
	case Timer_TIMER2:
		return SYS_INT_TINT2;
	case Timer_TIMER3:
		return SYS_INT_TINT3;
	case Timer_TIMER4:
		return SYS_INT_TINT4;
	case Timer_TIMER5:
		return SYS_INT_TINT5;
	case Timer_TIMER6:
		return SYS_INT_TINT6;
	case Timer_TIMER7:
		return SYS_INT_TINT7;
	default:
		return UINT32_MAX;
	}
}

void ClearTimer0IrqStatus() {
	ClearTimerCore(TIMER_0, TIMER_IRQSTATUS);
}

void ClearTimer1IrqStatus() {
	ClearTimerCore(TIMER_1MS, TIMER1_TISR);
}

void ClearTimer2IrqStatus() {
	ClearTimerCore(TIMER_2, TIMER_IRQSTATUS);
}

void ClearTimer3IrqStatus() {
	ClearTimerCore(TIMER_3, TIMER_IRQSTATUS);
}

void ClearTimer4IrqStatus() {
	ClearTimerCore(TIMER_4, TIMER_IRQSTATUS);
}

void ClearTimer5IrqStatus() {
	ClearTimerCore(TIMER_5, TIMER_IRQSTATUS);
}

void ClearTimer6IrqStatus() {
	ClearTimerCore(TIMER_6, TIMER_IRQSTATUS);
}

void ClearTimer7IrqStatus() {
	ClearTimerCore(TIMER_7, TIMER_IRQSTATUS);
}

void ClearTimerCore(uint32_t baseAddr, uint32_t tisr) {
	//clear all pending interrupt flags
	reg32an(baseAddr, tisr, TISR_ALL_FLAGS);
}
