/*
 * Driver: interrupt.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * TODO
 */

#ifndef INTERRUPT_H_
#define INTERRUPT_H_

/**
 * \brief Enables system interrupt
 *
 * \param intrNum		number of interrupt
 */

typedef void (*funcPtr)(void);
funcPtr funcPtr_Array[128];

void IntSystemEnable(unsigned int intrNum);
void HandleInterrupt(unsigned int num);
void SetInterrupt(unsigned int num, funcPtr fP);

#endif /* INTERRUPT_H_ */
