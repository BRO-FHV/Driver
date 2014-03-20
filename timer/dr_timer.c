/*
 * Driver: dr_timer.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 16.03.2014
 * Description: 
 * TODO
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
 * \brief   This macro will check for write POSTED status
 *
 * \param   tsicr
 * \param   twps
 * \param   reg			Register whose status has to be checked
 * \param   baseAdd

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

int32_t TimerEnable(Timer timer) {
	if (0 != timers[timer]) {
		return -1; //already enabled
	}

	uint32_t baseAddr = GetTimerBaseAddr(timer);

	if (UINT32_MAX == baseAddr) {
		return -1; //failure
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
	reg32wor(baseAddr, tclr, TCLR_ST);

	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

	timers[timer] = 1;
}

int32_t TimerDisable(Timer timer) {
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
	reg32an(baseAddr, tclr, TCLR_ST);

	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

	//TODO Disable interrupts

	timers[timer] = 0;
}

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
	reg32wor(baseAddr, tisr, RESET_VALUE);

	reg32wor(baseAddr, ttgr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TTGR, baseAddr);

	reg32wor(baseAddr, tclr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr);

	reg32wor(baseAddr, tcrr, RESET_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCRR, baseAddr);
}

//TODO feature wish: no prescaler - pass e.g. 500 ms, 1000 ms,...
int32_t TimerConfiguration(Timer timer, uint32_t milliseconds, InterruptRoutine routine) {
	if (0 < timers[timer] || NULL == routine) {
		return -1;
	}

	uint32_t baseAddr = GetTimerBaseAddr(timer);

	if (UINT32_MAX == baseAddr) {
		return -1; //failure
	}

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

	//use default
	reg32wor(baseAddr, tisr, 0x03);
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

//TODO Method that is passed as interrupt method
//in this method clear the M-Bit of the Interrupt and finally call function method passed via timer interrupt configuration
int32_t TimerInterruptFlagClear(Timer timer) {
	return -1;
}

/**
 * \brief   Configure the posted mode of a timer.
 *
 * \param   baseAdd       Base Address of the timer
 * \param   postMode      TSICR Register
 *
 * \return  None.
 *
 **/
void EnablePostedMode(uint32_t baseAddr, uint32_t tsicr) {
	/* Clear the POSTED field of TSICR */
	reg32an(baseAddr, tsicr, TSICR_POSTED);

	/* Write to the POSTED field of TSICR */
	reg32m(baseAddr, tsicr, TSICR_POSTED);
}

void SetIrqWakeenMode(uint32_t baseAddr, IrqWakeen irqwakeen,
		uint32_t irqWakeenRegister) {
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
		reg32wor(baseAddr, irqWakeenRegister,
				IRQWAKEEN_MAT_WUP_ENA & IRQWAKEEN_OVF_WUP_ENA & IRQWAKEEN_TCAR_WUP_ENA);
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
		reg32wor(baseAddr, irqRegister,
				IRQENABLE_OVF_EN_FLAG & IRQENABLE_MAT_EN_FLAG & IRQENABLE_TCAR_EN_FLAG);
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
