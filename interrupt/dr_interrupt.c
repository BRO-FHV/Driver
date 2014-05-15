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
#include <interrupt/hw_interrupt.h>
#include <cpu/hw_cpu.h>
#include "dr_interrupt.h"

intHandler intIrqHandlers[NUM_INTERRUPTS];
intResetHandler intIrqResetHandlers[NUM_INTERRUPTS];

static void IntDefaultHandler(void);
static void IntDefaultResetHandler(void);

void IntControllerInit(void) {
	uint32_t intNum;

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
		intIrqHandlers[intNum] = IntDefaultHandler;
		intIrqResetHandlers[intNum] = IntDefaultHandler;
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
void IntHandlerEnable(volatile uint32_t intNum) {
	__asm(" dsb");

	// Disable the system interrupt in the corresponding MIR_CLEAR register
	reg32w(SOC_AINTC_REGS, INTC_MIR_CLEAR(intNum >> REG_IDX_SHIFT),
			(0x01 << (intNum & REG_BIT_MASK)));
}

void IntHandlerDisable(volatile uint32_t intNum) {
	__asm(" dsb");

	// Enable the system interrupt in the corresponding MIR_SET register
	reg32w(SOC_AINTC_REGS, INTC_MIR_SET(intNum >> REG_IDX_SHIFT),
			(0x01 << (intNum & REG_BIT_MASK)));
}

void IntRegister(volatile uint32_t intNum, intHandler handler) {
	// Assign ISR
	intIrqHandlers[intNum] = handler;
}

void IntResetRegister(uint32_t intNum, intHandler handler) {
	// Assign ISR reset handler
	intIrqResetHandlers[intNum] = handler;
}

void IntUnRegister(volatile uint32_t intNum) {
	// Assign default ISR
	intIrqResetHandlers[intNum] = IntDefaultHandler;
}

void IntUnResetRegister(uint32_t intNum) {
	// Assign default ISR reset handler
	intIrqHandlers[intNum] = IntDefaultResetHandler;
}

void IntIRQHandler() {
	// active irq number
	uint32_t intNum = IntActiveIrqNumGet();

	// call assigned interrupt handler
	intIrqHandlers[intNum]();

	// call assigned interrupt reset handler
	intIrqResetHandlers[intNum]();

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
 * \brief   Returns the status of the interrupts FIQ and IRQ.
 *
 * \param    None
 *
 * \return   Status of interrupt as in CPSR.
 *
 *  Note: This function call shall be done only in previleged mode of ARM
 **/
uint32_t IntMasterStatusGet(void)
{
    uint32_t stat;

    //TODO REVIEW
    /* IRQ and FIQ in CPSR */
    //asm("    mrs     r0, CPSR and %[result], r0, #0xC0" : [result] "=r" (stat));

    return stat;
}

/**
 * \brief  Enables the processor IRQ only in CPSR. Makes the processor to
 *         respond to IRQs.  This does not affect the set of interrupts
 *         enabled/disabled in the AINTC.
 *
 * \param    None
 *
 * \return   None
 *
 *  Note: This function call shall be done only in previleged mode of ARM
 **/
void IntMasterIRQEnable(void)
{
    /* Enable IRQ in CPSR.*/
    CPUirqe();

}

/**
 * \brief  Disables the processor IRQ only in CPSR.Prevents the processor to
 *         respond to IRQs.  This does not affect the set of interrupts
 *         enabled/disabled in the AINTC.
 *
 * \param    None
 *
 * \return   None
 *
 *  Note: This function call shall be done only in previleged mode of ARM
 **/
void IntMasterIRQDisable(void)
{
    /* Disable IRQ in CPSR.*/
    CPUirqd();
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

/**
 * \brief Default Interrupt Reset Handler.
 */
static void IntDefaultResetHandler(void) {
	// Go Back, nothing to be done
	;
}

void IntSystemEnable(uint32_t intrNum) {
    __asm(" dsb");

    /* Disable the system interrupt in the corresponding MIR_CLEAR register */
    reg32w(SOC_AINTC_REGS, INTC_MIR_CLEAR(intrNum >> REG_IDX_SHIFT), (0x01 << (intrNum & REG_BIT_MASK)));
}
