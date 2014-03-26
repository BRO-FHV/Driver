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

void InitLedRegister(void);

void InitLed0(void);
void OnLed0(void);
void OffLed0(void);

void InitLed1(void);
void OnLed1(void);
void OffLed1(void);

void InitLed2(void);
void OnLed2(void);
void OffLed2(void);

void InitLed3(void);
void OnLed3(void);
void OffLed3(void);






#endif /* DR_LED_H_ */
