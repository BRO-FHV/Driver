/*
 * Driver: dr_led.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 26.03.2014
 * Description:
 * TODO
 */

/** ============================================================================
 *   \file  dr_led.h
 *
 *   \brief This Files contain information about Led Constants
 *
 *  ============================================================================
 */
#include <stdio.h>
#include <stdint.h>
#include "../gpio/dr_gpio.h"
#include <soc_AM335x.h>


#ifndef DR_LED_H_
#define DR_LED_H_

/*****************************************************************************
**                INTERNAL MACRO DEFINITIONS
*****************************************************************************/
#define LED_REGISTER          (SOC_GPIO_1_REGS)
#define LED0_PIN		(21)
#define LED1_PIN        (22)
#define LED2_PIN        (23)
#define LED3_PIN        (24)


/*****************************************************************************
**                LED Method Definition
*****************************************************************************/
void LedInitRegister(void);

void LedInit0(void);
void LedOn0(void);
void LedOff0(void);

void LedInit1(void);
void LedOn1(void);
void LedOff1(void);

void LedInit2(void);
void LedOn2(void);
void LedOff2(void);

void LedInit3(void);
void LedOn3(void);
void LedOff3(void);

#endif /* DR_LED_H_ */
