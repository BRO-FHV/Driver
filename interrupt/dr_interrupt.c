/*
 * Driver: dr_interrupt.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * TODO
 */

#include <stdio.h>
#include <basic.h>
#include <soc_AM335x.h>
#include <hw_interrupt.h>
#include <hw_cpu.h>
#include "dr_interrupt.h"

intHandler intHandlers[NUM_INTERRUPTS];

static void IntDefaultHandler(void);

void IntControllerInit(void) {
	unsigned int intNum;

	// Reset the ARM interrupt controller
	reg32w(SOC_AINTC_REGS, INTC_SYSCONFIG, INTC_SYSCONFIG_SOFTRESET);

	// Wait for the reset to complete
	wait(
			(reg32r(SOC_AINTC_REGS, INTC_SYSSTATUS) & INTC_SYSSTATUS_RESETDONE)
					!= INTC_SYSSTATUS_RESETDONE);

	// Enable any interrupt generation by setting priority threshold
	reg32w(SOC_AINTC_REGS, INTC_THRESHOLD, INTC_THRESHOLD_PRIORITYTHRESHOLD);

	// Register the default handler for all interrupts
	for (intNum = 0; intNum < NUM_INTERRUPTS; intNum++) {
		intHandlers[intNum] = IntDefaultHandler;
	}
}

void IntPrioritySet(unsigned int intrNum, unsigned int priority,
		unsigned int hostIntRoute) {
	reg32w(SOC_AINTC_REGS, INTC_ILR(intrNum),
			((priority << INTC_ILR_PRIORITY_SHIFT) & INTC_ILR_PRIORITY) | hostIntRoute);
}

/**
 * \brief Enables system interrupt
 *
 * \param intrNum		number of interrupt
 */
void IntHandlerEnable(uint32_t intNum) {
	__asm(" dsb");

	// Disable the system interrupt in the corresponding MIR_CLEAR register
	reg32w(SOC_AINTC_REGS, INTC_MIR_CLEAR(intNum >> REG_IDX_SHIFT),
			(0x01 << (intNum & REG_BIT_MASK)));
}

void IntHandlerDisable(uint32_t intNum) {
	__asm(" dsb");

	// Enable the system interrupt in the corresponding MIR_SET register
	reg32w(SOC_AINTC_REGS, INTC_MIR_SET(intNum >> REG_IDX_SHIFT),
			(0x01 << (intNum & REG_BIT_MASK)));
}

void IntRegister(uint32_t intNum, intHandler handler) {
	// Assign ISR
	intHandlers[intNum] = handler;
}

void IntUnRegister(uint32_t intNum) {
	// Assign default ISR
	intHandlers[intNum] = IntDefaultHandler;
}

void IntIRQHandler() {
	uint32_t intNum = IntActiveIrqNumGet();
	printf("interrupt %d\n", intNum);

	// call assigned interrupt handler
	intHandlers[intNum]();

	// reset interrupt pending bit
	reg32m(SOC_AINTC_REGS, INTC_CONTROL, INTC_CONTROL_NEWIRQAGR);
}

/**
 * \brief Reads the active IRQ number
 *
 * \return Active IRQ number.
 *
 **/
uint32_t IntActiveIrqNumGet(void) {
	return (reg32r(SOC_AINTC_REGS, INTC_SIR_IRQ) & INTC_SIR_IRQ_ACTIVEIRQ);
}

/**
 * \brief Default Interrupt Handler.
 *        This is the default interrupt handler for all interrupts. It simply returns
 *        without performing any operation
 */
static void IntDefaultHandler(void) {
// Go Back, nothing to be done
	;
}

