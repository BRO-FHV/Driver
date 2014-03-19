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
#include "dr_timer.h"

#define NUMBER_OF_TIMERS	8
#define RESET_VALUE			0x00

uint16_t timers[NUMBER_OF_TIMERS];

uint32_t GetTimerBaseAdr(volatile Timer timer);
void SetIrqWakeenMode(volatile uint32_t baseAdr,volatile IrqWakeen irqwakeen,volatile uint32_t irqWakeenRegister);
void SetIrqMode(volatile uint32_t baseAdr,volatile IrqMode irqMode,volatile uint32_t irqRegister);
void ResetTimer(volatile uint32_t baseAdr,volatile uint32_t tmar,volatile uint32_t tldr,volatile  uint32_t twer, volatile uint32_t tisr, volatile uint32_t ttgr,volatile uint32_t tclr,volatile uint32_t tcrr);
void TimerBasicConfigurationCore(volatile uint32_t baseAdr,volatile uint32_t tmar,volatile uint32_t tldr,volatile  uint32_t tisr,volatile  uint32_t ttgr,volatile uint32_t matchValue,volatile uint32_t loadValue);
void TimerConfigureCE(volatile uint32_t baseAdr,volatile uint32_t tclr,volatile uint8_t enable);
void TimerConfigureAR(volatile uint32_t baseAdr,volatile uint32_t tclr,volatile uint8_t enable);
uint32_t GetTimerInterruptCode(Timer timer);

int TimerEnable(Timer timer) {
	if(0 != timers[timer]) {
		return -1; //already enabled
	}

	uint32_t baseAdr = GetTimerBaseAdr(timer);

	if(UINT32_MAX == baseAdr) {
		return -1; //failure
	}

	if(1 == timer) {
		reg32wor(baseAdr, TIMER1_TCLR, 0x01);
	} else {
		reg32wor(baseAdr, TIMER_TCLR, 0x01);
	}

	timers[timer] = 1;

	return 0;
}

int TimerDisable(Timer timer) {
	if(1 != timers[timer]) {
		return -1; //not enabled
	}

	uint32_t baseAdr = GetTimerBaseAdr(timer);

	if(UINT32_MAX == baseAdr) {
		return -1; //failure
	}

	if(1 == timer) {
		reg32wor(baseAdr, TIMER1_TCLR, TCLR_ST);
	} else {
		reg32wor(baseAdr, TIMER_TCLR, TCLR_ST);
	}

	//TODO disable interrupts

	timers[timer] = 0;

	return 0;
}

int TimerReset(Timer timer) {
	uint32_t baseAdr = GetTimerBaseAdr(timer);

	if(UINT32_MAX == baseAdr) {
		return -1; //failure
	}

	//reset registers
	if(1 == timer) {
		ResetTimer(baseAdr, TIMER1_TMAR, TIMER1_TLDR, TIMER1_TWER, TIMER1_TISR, TIMER1_TTGR, TIMER1_TCLR, TIMER1_TCRR);
	} else {
		ResetTimer(baseAdr, TIMER_TMAR, TIMER_TLDR, TIMER_IRQWAKEEN, TIMER_IRQSTATUS, TIMER_TTGR, TIMER_TCLR, TIMER_TCRR);
	}

	return 0;
}

// TODO: change compareModeOn,  autoReloadOn,... to boolean..
// TODO: prescaler, timer source
int TimerBasicConfiguration(volatile Timer timer,volatile uint8_t enableCompareMode,volatile uint8_t enableAutoReload,volatile uint32_t matchValue,volatile uint32_t loadValue,volatile uint16_t clockSource,volatile uint8_t pre,volatile uint8_t ptv) {
	if(0 < timers[timer]) {
		return -1;
	}

	uint32_t baseAdr = GetTimerBaseAdr(timer);

	if(UINT32_MAX == baseAdr) {
		return -1; //failure
	}

	//TMAR should be less than 0xFFFF FFFF
	if(UINT32_MAX == matchValue) {
		return -1;
	}

	TimerReset(timer);
	volatile uint8_t eins = 1;
	if(eins == timer) {
		//timer 1 ms
		TimerBasicConfigurationCore(baseAdr, TIMER1_TMAR, TIMER1_TLDR, TIMER1_TISR, TIMER1_TTGR, matchValue, loadValue);

		TimerConfigureCE(baseAdr, TIMER1_TCLR, enableCompareMode);
		TimerConfigureAR(baseAdr, TIMER1_TCLR, enableAutoReload);
	} else {
		//timer 0, 2 - 7
		TimerBasicConfigurationCore(baseAdr, TIMER_TMAR, TIMER_TLDR, TIMER_IRQSTATUS, TIMER_TTGR, matchValue, loadValue);

		TimerConfigureCE(baseAdr, TIMER_TCLR, enableCompareMode);
		TimerConfigureAR(baseAdr, TIMER_TCLR, enableAutoReload);
	}

	//reset counter register
	reg32wor(baseAdr, 1 == timer ? TIMER1_TCRR : TIMER_TCRR, RESET_VALUE);

	return 0;
}


int TimerInterruptConfiguration(volatile Timer timer,volatile IrqMode irqMode,volatile IrqWakeen irqwakeen,volatile InterruptRoutine routine) {
	if(0 < timers[timer] || NULL == routine) {
		return -1;
	}

	uint32_t baseAdr = GetTimerBaseAdr(timer);

	if(UINT32_MAX == baseAdr) {
		return -1; //failure
	}

	if(1 == timer) {
		SetIrqWakeenMode(baseAdr, irqwakeen, TIMER1_TWER);
		SetIrqMode(baseAdr, irqMode, TIMER1_TIER);

		reg32wor(baseAdr, TIMER1_TISR, 0x03);
	} else {
		SetIrqWakeenMode(baseAdr, irqwakeen, TIMER_IRQWAKEEN);
		SetIrqMode(baseAdr, irqMode, TIMER_IRQENABLE_SET);

		reg32wor(baseAdr, TIMER_IRQSTATUS, 0x03);
	}
	IntHandlerEnable(GetTimerInterruptCode(timer));
	IntRegister(GetTimerInterruptCode(timer),routine);

	return 0;
}

void TimerConfigureCE(volatile uint32_t baseAdr, volatile uint32_t tclr,volatile uint8_t enable)
{
	if(1 == enable) {
		reg32wor(baseAdr, tclr, TCLR_CE);
	} else {
		reg32wxor(baseAdr, tclr, TCLR_CE);
	}
}

void TimerConfigureAR(volatile uint32_t baseAdr,volatile uint32_t tclr,volatile uint8_t enable) {
	if(1 == enable) {
		reg32wor(baseAdr, tclr, TCLR_AR);
	} else {
		reg32wxor(baseAdr, tclr, TCLR_AR);
	}
}

void TimerBasicConfigurationCore(volatile uint32_t baseAdr, volatile uint32_t tmar,volatile uint32_t tldr, volatile uint32_t tisr, volatile uint32_t ttgr,volatile uint32_t matchValue,volatile uint32_t loadValue) {
	//defines the match value (e.g. interrupt is raised, if this value is reached)
	reg32wor(baseAdr, tmar, matchValue);
	//defines where the timer should start to count (e.g. after a auto reload)
	reg32wor(baseAdr, tldr, loadValue);
	//use default
	reg32wor(baseAdr, tisr, 0x03);
	//Writing in the TTGR register, TCRR will be loaded from TLDR and prescaler counter will be cleared.
	//Reload will be done regardless of the AR field value of TCLR register.
	reg32wor(baseAdr, ttgr, RESET_VALUE);
}

void SetIrqWakeenMode(volatile uint32_t baseAdr,volatile IrqWakeen irqwakeen,volatile uint32_t irqWakeenRegister) {
	//Wakeup-enabled events taking place when module is idle will generate an asynchronous wakeup
	switch(irqwakeen) {
	case IrqWakeen_MAT_WUP_ENA:
		reg32wor(baseAdr, irqWakeenRegister, IRQWAKEEN_MAT_WUP_ENA);
		break;
	case IrqWakeen_OVF_WUP_ENA:
		reg32wor(baseAdr, irqWakeenRegister, IRQWAKEEN_OVF_WUP_ENA);
		break;
	case IrqWakeen_TCAR_WUP_ENA:
		reg32wor(baseAdr, irqWakeenRegister, IRQWAKEEN_TCAR_WUP_ENA);
		break;
	case IrqWakeen_ALL:
		reg32wor(baseAdr, irqWakeenRegister, IRQWAKEEN_MAT_WUP_ENA & IRQWAKEEN_OVF_WUP_ENA & IRQWAKEEN_TCAR_WUP_ENA);
		break;
	case IrqWakeen_NONE:
		//TODO
		break;
	}
}

void SetIrqMode(uint32_t baseAdr, IrqMode irqMode, uint32_t irqRegister) {
	switch (irqMode) {
	case IrqMode_MATCH:
		reg32wor(baseAdr, irqRegister, IRQENABLE_MAT_EN_FLAG);
		break;
	case IrqMode_CAPTURE:
		reg32wor(baseAdr, irqRegister, IRQENABLE_TCAR_EN_FLAG);
		break;
	case IrqMode_OVERFLOW:
		reg32wor(baseAdr, irqRegister, IRQENABLE_OVF_EN_FLAG);
		break;
	case IrqMode_ALL:
		reg32wor(baseAdr, irqRegister, IRQENABLE_OVF_EN_FLAG & IRQENABLE_MAT_EN_FLAG & IRQENABLE_TCAR_EN_FLAG);
		break;
	case IrqMode_NONE:
		//TODO
	default:
		break;
	}
}

uint32_t GetTimerBaseAdr(Timer timer) {
	switch(timer) {
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
	switch(timer) {
	case Timer_TIMER0:
		return SYS_INT_TINT0;
	case Timer_TIMER1MS:
		return SYS_INT_TINT1_1MS;
	case Timer_TIMER2:
		return SYS_INT_TINT2;
	case Timer_TIMER3:
		return SYS_INT_TINT3;
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

void ResetTimer(uint32_t baseAdr, uint32_t tmar, uint32_t tldr,  uint32_t twer,  uint32_t tisr,  uint32_t ttgr, uint32_t tclr, uint32_t tcrr) {
	reg32wor(baseAdr, tmar, RESET_VALUE);
	reg32wor(baseAdr, tldr, RESET_VALUE);
	reg32wor(baseAdr, twer, RESET_VALUE);
	reg32wor(baseAdr, tisr, RESET_VALUE);
	reg32wor(baseAdr, ttgr, RESET_VALUE);
	reg32wor(baseAdr, tclr, RESET_VALUE);
	reg32wor(baseAdr, tcrr, RESET_VALUE);
}
