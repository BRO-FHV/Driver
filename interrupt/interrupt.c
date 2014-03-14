/*
 * Driver: interrupt.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * TODO
 */

#include <basic.h>

/**
 * \brief Enables system interrupt
 *
 * \param intrNum		number of interrupt
 */
void IntSystemEnable(unsigned int intrNum) {
	reg32w(SOC_AINTC_REGS, INTC_MIR_CLEAR(intrNum >> REG_IDX_SHIFT),
		(0x01 << (intrNum & REG_BIT_MASK)));
}
