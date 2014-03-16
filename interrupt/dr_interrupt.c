/*
 * Driver: interrupt.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * TODO
 */

#include <basic.h>
#include <hw_interrupt.h>
#include "dr_interrupt.h"

/**
 * \brief Enables system interrupt
 *
 * \param intrNum		number of interrupt
 */
void IntSystemEnable(unsigned int intrNum) {
    __asm(" dsb");

    reg32w(SOC_AINTC_REGS, INTC_MIR_CLEAR(intrNum >> REG_IDX_SHIFT),
		(0x01 << (intrNum & REG_BIT_MASK)));
}


void HandleInterrupt(unsigned int num)
{
	funcPtr_Array[num]();
}


void SetInterrupt(unsigned int num, funcPtr fP)
{
	funcPtr_Array[num] = fP;
}
