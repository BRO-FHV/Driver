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
 *
 * The Timer 1 is configured as 1 ms interrupt timer.
 * The standard clock rate for timer 2 to 6 is 32 KHz. Timer 7 uses the oscillator clock rate.
 */
#include <stdio.h>
#include <timer/hw_timer.h>
#include "../interrupt/dr_interrupt.h"
#include <basic.h>
#include "dr_timer.h"
#include <soc_AM335x.h>
#include <hw_cm_dpll.h>
#include <hw_cm_wkup.h>
#include <hw_cm_per.h>


#define NUMBER_OF_TIMERS		8
#define RESET_VALUE				0x00
#define TRIGGER_VALUE   		(0xFFFFFFFFu)
#define TIMER_INITIAL_COUNT     (0xFFFFA23Fu)
#define TIMER_1MS_COUNT         (0x5DC0u)
#define TIMER_OVERFLOW          (0xFFFFFFFFu)

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

#define TIMER_INITIAL_COUNT             (0xFFFFA23Fu)
#define TIMER_1MS_COUNT                 (0x5DC0u)
#define NUMBER_OF_TIMERS				8
#define RESET_VALUE						0x00
#define DELAY_USE_INTERRUPTS 			1

static volatile Boolean flagIsr = 1;
static uint16_t timers[NUMBER_OF_TIMERS];
static const uint8_t TIMER_FACTOR = 32;

typedef enum {
	IrqMode_MATCH = 0, IrqMode_OVERFLOW, IrqMode_CAPTURE, IrqMode_ALL, IqrMode_OFF
} IrqMode;

typedef enum {
	IrqWakeen_MAT_WUP_ENA = 0,
	IrqWakeen_OVF_WUP_ENA,
	IrqWakeen_TCAR_WUP_ENA,
	IrqWakeen_ALL
} IrqWakeen;

typedef void (*ResetTimerFunc)();

uint32_t GetTimerBaseAddr(Timer timer);
uint32_t GetTimerInterruptCode(Timer timer);
ResetTimerFunc GetResetTimerFunc(Timer timer);

void SetIrqWakeenMode(uint32_t baseAddr, IrqWakeen irqwakeen, uint32_t irqWakeenRegister);
void SetIrqMode(uint32_t baseAddr, IrqMode irqMode, uint32_t irqRegister);

void EnablePostedMode(uint32_t baseAddr, uint32_t tsicr);
void EnableCore(Timer timer, uint32_t baseAddr, uint32_t tclr, uint32_t tsicr, uint32_t twps);
void DisableCore(Timer timer, uint32_t baseAddr, uint32_t tclr, uint32_t tsicr, uint32_t twps);
void ResetCore(uint32_t baseAddr, uint32_t tmar, uint32_t tldr, uint32_t twer, uint32_t tisr, uint32_t ttgr, uint32_t tclr, uint32_t tcrr, uint32_t tsicr, uint32_t twps);
void ConfigurationCore(uint32_t baseAddr, uint32_t tldr, uint32_t tisr, uint32_t ttgr, uint32_t tclr, uint32_t twer, uint32_t tier, uint32_t tsicr, uint32_t twps, uint32_t tcrr);

uint32_t IsClockModuleTimerEnabled(Timer timer);
void ClockModuleEnable(Timer timer);
void ClockModuleEnableCore(uint32_t dpll_clksel_clk, uint32_t dpll_clksel_clk_clksel, uint32_t dpll_clksel_clk_clksel_sel, uint32_t per_clkctrl, uint32_t per_clkctrl_mode, uint32_t per_ctlctrl_modulemode, uint32_t per_clkctrl_idlest, uint32_t per_clkctrl_idelst_func, uint32_t per_clkactivity_gclk);

void ResetTimer1IrqStatus();
void ResetTimer2IrqStatus();
void ResetTimer3IrqStatus();
void ResetTimer4IrqStatus();
void ResetTimer5IrqStatus();
void ResetTimer6IrqStatus();
void ResetTimer7IrqStatus();
void ResetTimerIrqStatusCore(uint32_t baseAddr, uint32_t tisr, uint32_t tsicr, uint32_t tcrr, uint32_t twps, uint32_t ttgr);

void ShutdownDelayTimer();
void EnableDelayTimerInterrupts();
void DisableDelayTimerInterrupts();
void SetDelayTimerCounterValue(uint32_t value);
void SetDelayTimerMatchValue(uint32_t value);
void EnableDelayTimer();
void DisableDelayTimer();
uint32_t GetDelayTimerCounterValue();
static void DelayTimerIsr();


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
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

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
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

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

	ResetTimerIrqStatusCore(baseAddr, tisr, tsicr, tcrr, twps, ttgr);
}

/**
 * \brief Configure the passed timer to raise an interrupt (after passed milliseconds).
 *
 * \param timer	Timer that should be configured.
 * \param milliseconds Time slice after which the interrupt is raised. If Timer_TIMER1MS is configured this value can be zero.
 * \param routine This routine is called to handle the interrupt.
 *
 * \return 0 on success, -1 on failure
 */
int32_t TimerConfiguration(Timer timer, uint32_t milliseconds, InterruptRoutine routine) {
	if (1 == timers[timer] || NULL == routine) {
		return -1; //timer is already enabled or routine is not set
	}

	uint32_t baseAddr = GetTimerBaseAddr(timer);

	if (UINT32_MAX == baseAddr) {
		return -1; //failure
	}

	if (1 == timer) {
		//timer 1 ms
		if(IsClockModuleTimerEnabled(timer)) {
			DisableCore(timer, baseAddr, TIMER1_TCLR, TIMER1_TSICR, TIMER1_TWPS);
			ResetCore(baseAddr, TIMER1_TMAR, TIMER1_TLDR, TIMER1_TWER, TIMER1_TISR, TIMER1_TTGR, TIMER1_TCLR, TIMER1_TCRR, TIMER1_TSICR, TIMER1_TWPS);
		} else {
			ClockModuleEnable(timer);
		}

		ConfigurationCore(baseAddr, TIMER1_TLDR, TIMER1_TISR, TIMER1_TTGR, TIMER1_TCLR, TIMER1_TWER, TIMER1_TIER, TIMER1_TSICR, TIMER1_TWPS, TIMER1_TCRR);
	} else {
		//TMAR should be less than 0xFFFF FFFF
		uint32_t matchValue = milliseconds * TIMER_FACTOR;
		if(UINT32_MAX == matchValue || 0 >= matchValue) {
			return -1;
		}

		//timer 2 - 7
		if(IsClockModuleTimerEnabled(timer)) {
			DisableCore(timer, baseAddr, TIMER_TCLR, TIMER_TSICR, TIMER_TWPS);
			ResetCore(baseAddr, TIMER_TMAR, TIMER_TLDR, TIMER_IRQWAKEEN, TIMER_IRQSTATUS, TIMER_TTGR, TIMER_TCLR, TIMER_TCRR, TIMER_TSICR, TIMER_TWPS);
		} else {
			ClockModuleEnable(timer);
		}

		ConfigurationCore(baseAddr, TIMER_TLDR, TIMER_IRQSTATUS, TIMER_TTGR, TIMER_TCLR, TIMER_IRQWAKEEN, TIMER_IRQENABLE_SET, TIMER_TSICR, TIMER_TWPS, TIMER_TCRR);

		//defines the match value (e.g. interrupt is raised, if this value is reached)
		reg32wor(baseAddr, TIMER_TMAR, matchValue);
		WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TMAR, baseAddr)
	}

	//configure interrupt routine
	uint32_t irqCode = GetTimerInterruptCode(timer);
	IntRegister(irqCode, routine);
	IntHandlerEnable(irqCode);
	IntResetRegister(irqCode, GetResetTimerFunc(timer));

	return 0;
}

void ConfigurationCore(uint32_t baseAddr, uint32_t tldr, uint32_t tisr, uint32_t ttgr, uint32_t tclr, uint32_t twer, uint32_t tier, uint32_t tsicr, uint32_t twps, uint32_t tcrr) {

	//enable posted mode for checking pending writes
	EnablePostedMode(baseAddr, tsicr);

	//defines where the timer should start to count (e.g. after a auto reload)
	reg32wor(baseAddr, tldr, 0x00);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TLDR, baseAddr)

	//Writing in the TTGR register, TCRR will be loaded from TLDR and prescaler counter will be cleared.
	//Reload will be done regardless of the AR field value of TCLR register.
	reg32wor(baseAddr, ttgr, TRIGGER_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TTGR, baseAddr)

	//set compare enabled and auto reload
	reg32wor(baseAddr, tclr, TCLR_CE + TCLR_AR);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TCLR, baseAddr)

	SetIrqWakeenMode(baseAddr, IrqWakeen_MAT_WUP_ENA, twer);
	SetIrqMode(baseAddr, IrqMode_MATCH, tier);

	//clear pending interrupts
	ResetTimerIrqStatusCore(baseAddr, tisr, tsicr, tcrr, twps, ttgr);
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
		reg32wor(baseAddr, irqWakeenRegister, IRQWAKEEN_MAT_WUP_ENA + IRQWAKEEN_OVF_WUP_ENA + IRQWAKEEN_TCAR_WUP_ENA);
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
		reg32wor(baseAddr, irqRegister, IRQENABLE_OVF_EN_FLAG + IRQENABLE_MAT_EN_FLAG + IRQENABLE_TCAR_EN_FLAG);
		break;
	case IqrMode_OFF:
		reg32w(baseAddr, irqRegister, 0);
		break;
	default:
		break;
	}
}

ResetTimerFunc GetResetTimerFunc(Timer timer) {
	switch (timer) {
	case Timer_TIMER1MS:
		return ResetTimer1IrqStatus;
	case Timer_TIMER2:
		return ResetTimer2IrqStatus;
	case Timer_TIMER3:
		return ResetTimer3IrqStatus;
	case Timer_TIMER4:
		return ResetTimer4IrqStatus;
	case Timer_TIMER5:
		return ResetTimer5IrqStatus;
	case Timer_TIMER6:
		return ResetTimer6IrqStatus;
	case Timer_TIMER7:
		return ResetTimer7IrqStatus;
	default:
		return NULL;
	}
}

uint32_t GetTimerBaseAddr(Timer timer) {
	switch (timer) {
	case Timer_TIMER1MS:
		return SOC_DMTIMER_1_REGS;
	case Timer_TIMER2:
		return SOC_DMTIMER_2_REGS;
	case Timer_TIMER3:
		return SOC_DMTIMER_3_REGS;
	case Timer_TIMER4:
		return SOC_DMTIMER_4_REGS;
	case Timer_TIMER5:
		return SOC_DMTIMER_5_REGS;
	case Timer_TIMER6:
		return SOC_DMTIMER_6_REGS;
	case Timer_TIMER7:
		return SOC_DMTIMER_7_REGS;
	default:
		return UINT32_MAX;
	}
}

uint32_t GetTimerInterruptCode(Timer timer) {
	switch (timer) {
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

void ResetTimer2IrqStatus() {
	ResetTimerIrqStatusCore(SOC_DMTIMER_2_REGS, TIMER_IRQSTATUS, TIMER_TSICR, TIMER_TCRR, TIMER_TWPS, TIMER_TTGR);
}

void ResetTimer3IrqStatus() {
	ResetTimerIrqStatusCore(SOC_DMTIMER_3_REGS, TIMER_IRQSTATUS, TIMER_TSICR, TIMER_TCRR, TIMER_TWPS, TIMER_TTGR);
}

void ResetTimer4IrqStatus() {
	ResetTimerIrqStatusCore(SOC_DMTIMER_4_REGS, TIMER_IRQSTATUS, TIMER_TSICR, TIMER_TCRR, TIMER_TWPS, TIMER_TTGR);
}

void ResetTimer5IrqStatus() {
	ResetTimerIrqStatusCore(SOC_DMTIMER_5_REGS, TIMER_IRQSTATUS, TIMER_TSICR, TIMER_TCRR, TIMER_TWPS, TIMER_TTGR);
}

void ResetTimer6IrqStatus() {
	ResetTimerIrqStatusCore(SOC_DMTIMER_6_REGS, TIMER_IRQSTATUS, TIMER_TSICR, TIMER_TCRR, TIMER_TWPS, TIMER_TTGR);
}

void ResetTimer7IrqStatus() {
	ResetTimerIrqStatusCore(SOC_DMTIMER_7_REGS, TIMER_IRQSTATUS, TIMER_TSICR, TIMER_TCRR, TIMER_TWPS, TIMER_TTGR);
}

void ResetTimerIrqStatusCore(uint32_t baseAddr, uint32_t tisr, uint32_t tsicr, uint32_t tcrr, uint32_t twps, uint32_t ttgr) {
	//clear all pending interrupt flags
	reg32w(baseAddr, tisr, 1);
	wait((reg32r(baseAddr, tisr)) != 0);

	//Writing in the TTGR register, TCRR will be loaded from TLDR and prescaler counter will be cleared.
	//Reload will be done regardless of the AR field value of TCLR register.
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TTGR, baseAddr)
	reg32wor(baseAddr, ttgr, TRIGGER_VALUE);
	WaitForWrite(tsicr, twps, TWPS_W_PEND_TTGR, baseAddr)
}

uint32_t IsClockModuleTimerEnabled(Timer timer) {
	switch (timer) {
	case Timer_TIMER1MS: //no clock configuration
		return CM_WKUP_TIMER1_CLKCTRL_MODULEMODE_ENABLE == (reg32r(SOC_CM_WKUP_REGS, CM_WKUP_TIMER1_CLKCTRL) & CM_WKUP_TIMER1_CLKCTRL_MODULEMODE);
	case Timer_TIMER2:
		return CM_PER_TIMER2_CLKCTRL_MODULEMODE_ENABLE == (reg32r(SOC_CM_PER_REGS, CM_PER_TIMER2_CLKCTRL) & CM_PER_TIMER2_CLKCTRL_MODULEMODE);
	case Timer_TIMER3:
		return CM_PER_TIMER3_CLKCTRL_MODULEMODE_ENABLE == (reg32r(SOC_CM_PER_REGS, CM_PER_TIMER3_CLKCTRL) & CM_PER_TIMER3_CLKCTRL_MODULEMODE);
	case Timer_TIMER4:
		return CM_PER_TIMER4_CLKCTRL_MODULEMODE_ENABLE == (reg32r(SOC_CM_PER_REGS, CM_PER_TIMER4_CLKCTRL) & CM_PER_TIMER4_CLKCTRL_MODULEMODE);
	case Timer_TIMER5:
		return CM_PER_TIMER5_CLKCTRL_MODULEMODE_ENABLE == (reg32r(SOC_CM_PER_REGS, CM_PER_TIMER5_CLKCTRL) & CM_PER_TIMER5_CLKCTRL_MODULEMODE);
	case Timer_TIMER6:
		return CM_PER_TIMER6_CLKCTRL_MODULEMODE_ENABLE == (reg32r(SOC_CM_PER_REGS, CM_PER_TIMER6_CLKCTRL) & CM_PER_TIMER6_CLKCTRL_MODULEMODE);
	case Timer_TIMER7:
		return CM_PER_TIMER7_CLKCTRL_MODULEMODE_ENABLE == (reg32r(SOC_CM_PER_REGS, CM_PER_TIMER7_CLKCTRL) & CM_PER_TIMER7_CLKCTRL_MODULEMODE);
	default:
		return 0;
	}
}

void ClockModuleEnable(Timer timer) {
	switch (timer) {
	case Timer_TIMER1MS:
	    reg32an(SOC_CM_DPLL_REGS, CM_DPLL_CLKSEL_TIMER1MS_CLK, CM_DPLL_CLKSEL_TIMER1MS_CLK_CLKSEL);
	    wait((reg32r(SOC_CM_DPLL_REGS, CM_DPLL_CLKSEL_TIMER1MS_CLK) & CM_DPLL_CLKSEL_TIMER1MS_CLK_CLKSEL) != 0x0);

		reg32w(SOC_CM_DPLL_REGS, CM_DPLL_CLKSEL_TIMER1MS_CLK, CM_DPLL_CLKSEL_TIMER1MS_CLK_CLKSEL_SEL1);

	    wait((reg32r(SOC_CM_DPLL_REGS, CM_DPLL_CLKSEL_TIMER1MS_CLK) & CM_DPLL_CLKSEL_TIMER1MS_CLK_CLKSEL) != CM_DPLL_CLKSEL_TIMER1MS_CLK_CLKSEL_SEL1);

	    reg32m(SOC_CM_WKUP_REGS, CM_WKUP_TIMER1_CLKCTRL, CM_WKUP_TIMER1_CLKCTRL_MODULEMODE_ENABLE);

	    wait((reg32r(SOC_CM_WKUP_REGS, CM_WKUP_TIMER1_CLKCTRL) & CM_WKUP_TIMER1_CLKCTRL_MODULEMODE) != CM_WKUP_TIMER1_CLKCTRL_MODULEMODE_ENABLE);

	    wait((reg32r(SOC_CM_WKUP_REGS, CM_WKUP_TIMER1_CLKCTRL) & CM_WKUP_TIMER1_CLKCTRL_IDLEST) != CM_WKUP_TIMER1_CLKCTRL_IDLEST_FUNC);
		break;
	case Timer_TIMER2:
		ClockModuleEnableCore(CM_DPLL_CLKSEL_TIMER2_CLK, CM_DPLL_CLKSEL_TIMER2_CLK_CLKSEL, CM_DPLL_CLKSEL_TIMER2_CLK_CLKSEL_SEL3, CM_PER_TIMER2_CLKCTRL,
								CM_PER_TIMER2_CLKCTRL_MODULEMODE_ENABLE, CM_PER_TIMER2_CLKCTRL_MODULEMODE, CM_PER_TIMER2_CLKCTRL_IDLEST, CM_PER_TIMER2_CLKCTRL_IDLEST_FUNC, CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER2_GCLK);
		break;
	case Timer_TIMER3:
		ClockModuleEnableCore(CM_DPLL_CLKSEL_TIMER3_CLK, CM_DPLL_CLKSEL_TIMER3_CLK_CLKSEL, CM_DPLL_CLKSEL_TIMER3_CLK_CLKSEL_SEL3, CM_PER_TIMER3_CLKCTRL,
								CM_PER_TIMER3_CLKCTRL_MODULEMODE_ENABLE, CM_PER_TIMER3_CLKCTRL_MODULEMODE, CM_PER_TIMER3_CLKCTRL_IDLEST, CM_PER_TIMER3_CLKCTRL_IDLEST_FUNC, CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER3_GCLK);
		break;
	case Timer_TIMER4:
		ClockModuleEnableCore(CM_DPLL_CLKSEL_TIMER4_CLK, CM_DPLL_CLKSEL_TIMER4_CLK_CLKSEL, CM_DPLL_CLKSEL_TIMER4_CLK_CLKSEL_SEL3, CM_PER_TIMER4_CLKCTRL,
								CM_PER_TIMER4_CLKCTRL_MODULEMODE_ENABLE, CM_PER_TIMER4_CLKCTRL_MODULEMODE, CM_PER_TIMER4_CLKCTRL_IDLEST, CM_PER_TIMER4_CLKCTRL_IDLEST_FUNC, CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER4_GCLK);
		break;
	case Timer_TIMER5:
		ClockModuleEnableCore(CM_DPLL_CLKSEL_TIMER5_CLK, CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL, CM_DPLL_CLKSEL_TIMER5_CLK_CLKSEL_SEL3, CM_PER_TIMER5_CLKCTRL,
								CM_PER_TIMER5_CLKCTRL_MODULEMODE_ENABLE, CM_PER_TIMER5_CLKCTRL_MODULEMODE, CM_PER_TIMER5_CLKCTRL_IDLEST, CM_PER_TIMER5_CLKCTRL_IDLEST_FUNC, CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER5_GCLK);
		break;
	case Timer_TIMER6:
		ClockModuleEnableCore(CM_DPLL_CLKSEL_TIMER6_CLK, CM_DPLL_CLKSEL_TIMER6_CLK_CLKSEL, CM_DPLL_CLKSEL_TIMER6_CLK_CLKSEL_SEL3, CM_PER_TIMER6_CLKCTRL,
								CM_PER_TIMER6_CLKCTRL_MODULEMODE_ENABLE, CM_PER_TIMER6_CLKCTRL_MODULEMODE, CM_PER_TIMER6_CLKCTRL_IDLEST, CM_PER_TIMER6_CLKCTRL_IDLEST_FUNC, CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER6_GCLK);
		break;
	case Timer_TIMER7:
		ClockModuleEnableCore(CM_DPLL_CLKSEL_TIMER7_CLK, CM_DPLL_CLKSEL_TIMER7_CLK_CLKSEL, CM_DPLL_CLKSEL_TIMER7_CLK_CLKSEL_CLK_M_OSC, CM_PER_TIMER7_CLKCTRL,
								CM_PER_TIMER7_CLKCTRL_MODULEMODE_ENABLE, CM_PER_TIMER7_CLKCTRL_MODULEMODE, CM_PER_TIMER7_CLKCTRL_IDLEST, CM_PER_TIMER7_CLKCTRL_IDLEST_FUNC, CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_TIMER7_GCLK);
		break;
	}
}

void ClockModuleEnableCore(uint32_t dpll_clksel_clk, uint32_t dpll_clksel_clk_clksel, uint32_t dpll_clksel_clk_clksel_sel, uint32_t per_clkctrl,
		uint32_t per_clkctrl_mode, uint32_t per_ctlctrl_modulemode, uint32_t per_clkctrl_idlest, uint32_t per_clkctrl_idelst_func, uint32_t per_clkactivity_gclk) {

	reg32w(SOC_CM_PER_REGS, CM_PER_L3S_CLKSTCTRL, CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	wait((reg32r(SOC_CM_PER_REGS, CM_PER_L3S_CLKSTCTRL) & CM_PER_L3S_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3S_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

	reg32w(SOC_CM_PER_REGS, CM_PER_L3_CLKSTCTRL, CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	wait((reg32r(SOC_CM_PER_REGS, CM_PER_L3_CLKSTCTRL) & CM_PER_L3_CLKSTCTRL_CLKTRCTRL) != CM_PER_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

	reg32w(SOC_CM_PER_REGS, CM_PER_L3_INSTR_CLKCTRL, CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE);
	wait((reg32r(SOC_CM_PER_REGS, CM_PER_L3_INSTR_CLKCTRL) & CM_PER_L3_INSTR_CLKCTRL_MODULEMODE) != CM_PER_L3_INSTR_CLKCTRL_MODULEMODE_ENABLE);

	reg32w(SOC_CM_PER_REGS, CM_PER_L3_CLKCTRL, CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE);
	wait((reg32r(SOC_CM_PER_REGS, CM_PER_L3_CLKCTRL) & CM_PER_L3_CLKCTRL_MODULEMODE) != CM_PER_L3_CLKCTRL_MODULEMODE_ENABLE);

	reg32w(SOC_CM_PER_REGS, CM_PER_OCPWP_L3_CLKSTCTRL, CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	wait((reg32r(SOC_CM_PER_REGS, CM_PER_OCPWP_L3_CLKSTCTRL) & CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL) != CM_PER_OCPWP_L3_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

	reg32w(SOC_CM_PER_REGS, CM_PER_L4LS_CLKSTCTRL, CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP);
	wait((reg32r(SOC_CM_PER_REGS, CM_PER_L4LS_CLKSTCTRL) & CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL) != CM_PER_L4LS_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

	reg32w(SOC_CM_PER_REGS, CM_PER_L4LS_CLKCTRL, CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE);
	wait((reg32r(SOC_CM_PER_REGS, CM_PER_L4LS_CLKCTRL) & CM_PER_L4LS_CLKCTRL_MODULEMODE) != CM_PER_L4LS_CLKCTRL_MODULEMODE_ENABLE);

	reg32an(SOC_CM_DPLL_REGS, dpll_clksel_clk, dpll_clksel_clk_clksel);
	reg32m(SOC_CM_DPLL_REGS, dpll_clksel_clk, dpll_clksel_clk_clksel_sel);
	wait((reg32r(SOC_CM_DPLL_REGS, dpll_clksel_clk) & dpll_clksel_clk_clksel) != dpll_clksel_clk_clksel_sel);

	reg32m(SOC_CM_PER_REGS, per_clkctrl, per_clkctrl_mode);
	wait((reg32r(SOC_CM_PER_REGS, per_clkctrl) & per_ctlctrl_modulemode) != per_clkctrl_mode);
	wait((reg32r(SOC_CM_PER_REGS, per_clkctrl) & per_clkctrl_idlest) != per_clkctrl_idelst_func);
	wait(!(reg32r(SOC_CM_PER_REGS, CM_PER_L3S_CLKSTCTRL) & CM_PER_L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK));
	wait(!(reg32r(SOC_CM_PER_REGS, CM_PER_L3_CLKSTCTRL) & CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK));
	wait(!(reg32r(SOC_CM_PER_REGS, CM_PER_OCPWP_L3_CLKSTCTRL) & (CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK | CM_PER_OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK)));
	wait(!(reg32r(SOC_CM_PER_REGS, CM_PER_L4LS_CLKSTCTRL) & (CM_PER_L4LS_CLKSTCTRL_CLKACTIVITY_L4LS_GCLK | per_clkactivity_gclk)));
}

/**
 * \brief   This function configures timer 7 to be used as delay timer.
 *
 * \param   None
 *
 * \return  None.
 *
 */
void TimerDelaySetup() {
	//timer 7
	if(IsClockModuleTimerEnabled(Timer_TIMER7)) {
		DisableCore(Timer_TIMER7, SOC_DMTIMER_7_REGS, TIMER_TCLR, TIMER_TSICR, TIMER_TWPS);
		ResetCore(SOC_DMTIMER_7_REGS, TIMER_TMAR, TIMER_TLDR, TIMER_IRQWAKEEN, TIMER_IRQSTATUS, TIMER_TTGR, TIMER_TCLR, TIMER_TCRR, TIMER_TSICR, TIMER_TWPS);
	} else {
		ClockModuleEnable(Timer_TIMER7);
	}

	//enable posted mode for checking pending writes
	EnablePostedMode(SOC_DMTIMER_7_REGS, TIMER_TSICR);

	//defines where the timer should start to count (e.g. after a auto reload)
	reg32wor(SOC_DMTIMER_7_REGS, TIMER_TLDR, 0x00);
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TLDR, SOC_DMTIMER_7_REGS)

	//Writing in the TTGR register, TCRR will be loaded from TLDR and prescaler counter will be cleared.
	//Reload will be done regardless of the AR field value of TCLR register.
	reg32wor(SOC_DMTIMER_7_REGS, TIMER_TTGR, RESET_VALUE);
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TTGR, SOC_DMTIMER_7_REGS)

	//set one shot and no compare enabled
	//set compare enabled and auto reload
	reg32wor(SOC_DMTIMER_7_REGS, TIMER_TCLR, TCLR_CE);
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCLR, SOC_DMTIMER_7_REGS)

    /* Set the counter value */
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCRR, SOC_DMTIMER_7_REGS)
    reg32w(SOC_DMTIMER_7_REGS, TIMER_TCRR, RESET_VALUE);
#if DELAY_USE_INTERRUPTS
	SetIrqWakeenMode(SOC_DMTIMER_7_REGS, IrqWakeen_ALL, TIMER_IRQWAKEEN);
	SetIrqMode(SOC_DMTIMER_7_REGS, IrqMode_ALL, TIMER_IRQENABLE_SET);

	//clear pending interrupts
	ResetTimer7IrqStatus();

    /* Registering DelayTimerIsr */
    IntRegister(SYS_INT_TINT7, DelayTimerIsr);
    /* Set the priority */
    IntPrioritySet(SYS_INT_TINT7, 0, AINTC_HOSTINT_ROUTE_IRQ);
	IntHandlerEnable(SYS_INT_TINT7);
	IntResetRegister(SYS_INT_TINT7, ResetTimer7IrqStatus);
#endif
}

/**
 * \brief   This function generates a delay of specified milli-seconds.
 *
 * \param   milliSec     This is the number of milli-seconds of delay.
 *
 * \return  None.
 *
 * \Note   This function should not be called when StartTimer, StopTimer and
 *         IsTimerElapsed functionality is in use.
 */
void TimerDelayDelay(uint32_t milliSec) {
#if DELAY_USE_INTERRUPTS
    uint32_t countVal = TIMER_OVERFLOW - (milliSec * TIMER_1MS_COUNT);
    SetDelayTimerCounterValue(countVal);

    flagIsr = FALSE;

    EnableDelayTimerInterrupts();

    EnableDelayTimer();

    while(FALSE == flagIsr);
    DisableDelayTimerInterrupts();

    ShutdownDelayTimer();

    SetDelayTimerCounterValue(RESET_VALUE);
#else
    while(milliSec != 0)
    {
    	SetDelayTimerCounterValue(RESET_VALUE);
    	EnableDelayTimer();
        while(GetDelayTimerCounterValue() < TIMER_1MS_COUNT);
        DisableDelayTimer();
        milliSec--;
    }
#endif
}

/**
 * \brief   This function starts the timer for millisec timeout.
 *
 * \param   milliSec     This is the number of milli-seconds of delay.
 *
 * \return  None.
 */
void TimerDelayStart(uint32_t milliSec) {
#if DELAY_USE_INTERRUPTS
	uint32_t matchValue = milliSec * TIMER_FACTOR;

    SetDelayTimerCounterValue(RESET_VALUE);
    SetDelayTimerMatchValue(matchValue);

    flagIsr = FALSE;

    EnableDelayTimerInterrupts();
#else
    // Set the counter value
    SetDelayTimerCounterValue(RESET_VALUE);
    flagIsr = milliSec;
#endif

	EnableDelayTimer();
}

/**
 * \brief   This function starts the timer for millisec timeout.
 *
 * \param   None.
 *
 * \return  None.
 *
 * \NOTE    delay functionality cannot be used till StopTimer is called.
 */
void TimerDelayStop() {
	ShutdownDelayTimer();
#if DELAY_USE_INTERRUPTS
	DisableDelayTimerInterrupts();
#endif
}

/**
 * \brief   This function checks whether timer is expired for set milli secs
 *
 * \param   None.
 *
 * \return  None.
 *
 * \NOTE  	delay functionality cannot be used till SysStopTimer is called.
 */
uint32_t TimerDelayIsElapsed() {
#if DELAY_USE_INTERRUPTS

    return flagIsr;

#else
    if(GetDelayTimerCounterValue() < (flagIsr * TIMER_1MS_COUNT))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
#endif
}

static void DelayTimerIsr()
{
	ShutdownDelayTimer();

	ResetTimer7IrqStatus();

	TimerDisable(Timer_TIMER7);
    flagIsr = TRUE;
}

void ShutdownDelayTimer() {
	//shut down timer
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCLR, SOC_DMTIMER_7_REGS)
	reg32an(SOC_DMTIMER_7_REGS, TIMER_TCLR, TCLR_ST);
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCLR, SOC_DMTIMER_7_REGS)
}

void EnableDelayTimerInterrupts() {
    // Enable the interrupts
//    reg32w(SOC_DMTIMER_7_REGS, TIMER_IRQENABLE_SET, IRQENABLE_OVF_EN_FLAG & (IRQENABLE_TCAR_EN_FLAG | IRQENABLE_OVF_EN_FLAG | IRQENABLE_MAT_EN_FLAG));
	SetIrqMode(SOC_DMTIMER_7_REGS, IrqMode_ALL, TIMER_IRQENABLE_SET);
}

void DisableDelayTimerInterrupts() {
	// Disable the interrupts
//    reg32w(SOC_DMTIMER_7_REGS, TIMER_IRQENABLE_CLR, IRQENABLE_OVF_EN_FLAG & (IRQENABLE_TCAR_EN_FLAG | IRQENABLE_OVF_EN_FLAG | IRQENABLE_MAT_EN_FLAG));
	SetIrqMode(SOC_DMTIMER_7_REGS, IqrMode_OFF, TIMER_IRQENABLE_SET);
}

void SetDelayTimerCounterValue(uint32_t value) {
    // Set the counter value
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCRR, SOC_DMTIMER_7_REGS)
    reg32w(SOC_DMTIMER_7_REGS, TIMER_TCRR, value);
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCRR, SOC_DMTIMER_7_REGS)
}

void SetDelayTimerMatchValue(uint32_t value) {
	//defines the match value (e.g. interrupt is raised, if this value is reached)
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TMAR, SOC_DMTIMER_7_REGS)
	reg32wor(SOC_DMTIMER_7_REGS, TIMER_TMAR, value);
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TMAR, SOC_DMTIMER_7_REGS)
}

void EnableDelayTimer() {
    /* Wait for previous write to complete */
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCLR, SOC_DMTIMER_7_REGS)

	//turn on timer
	reg32wor(SOC_DMTIMER_7_REGS, TIMER_TCLR, TCLR_ST);

    /* Wait for previous write to complete */
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCLR, SOC_DMTIMER_7_REGS)
}

void DisableDelayTimer() {
    /* Wait for previous write to complete */
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCLR, SOC_DMTIMER_7_REGS)

    /* Stop the timer */
	reg32an(SOC_DMTIMER_7_REGS, TIMER_TCLR, TCLR_ST);

    /* Wait for previous write to complete */
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCLR, SOC_DMTIMER_7_REGS);
}

uint32_t GetDelayTimerCounterValue() {
	// Wait for previous write to complete
	WaitForWrite(TIMER_TSICR, TIMER_TWPS, TWPS_W_PEND_TCRR, SOC_DMTIMER_7_REGS)

	// Read the counter value from TCRR
	return reg32r(SOC_DMTIMER_7_REGS, TIMER_TCRR) ;
}
