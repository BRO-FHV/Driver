/*
 * Driver: dr_led.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 26.03.2014
 * Description:
 * TODO
 */

/** ============================================================================
 *   \file  dr_led.c
 *
 *   \brief This Files contain methods to enable and disable Leds
 *
 *  ============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include "dr_led.h"
#include <soc_AM335x.h>

/**
 * \brief  Methode um das LED GPIO einzuschalten
 *
 *
 *
 */
void InitLedRegister(void) {
	GPIO1ModuleClkConfig();

	/* Enabling the GPIO module. */
	GPIOModuleEnable(LED_REGISTER);

	/* Resetting the GPIO module. */
	GPIOModuleReset(LED_REGISTER);


}

/**
 * \brief  LED0 enable
 *
 *
 */
void InitLed0(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED0_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED1 enable
 *
 *
 */
void InitLed1(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED1_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED2 enable
 *
 *
 */
void InitLed2(void){

	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED2_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED3 enable
 *
 *
 */
void InitLed3(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED3_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED0 on
 *
 *
 */
void OnLed0(void) {
	GPIOPinWrite(LED_REGISTER, LED0_PIN, GPIO_PIN_HIGH);



}

/**
 * \brief  LED1 off
 *
 *
 */
void OffLed0(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED0_PIN, GPIO_PIN_LOW);
}

/**
 * \brief  LED1 on
 *
 *
 */
void OnLed1(void) {
	GPIOPinWrite(LED_REGISTER, LED1_PIN, GPIO_PIN_HIGH);



}

/**
 * \brief  LED2 off
 *
 *
 */
void OffLed1(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED1_PIN, GPIO_PIN_LOW);
}


void OnLed2(void) {
	GPIOPinWrite(LED_REGISTER, LED2_PIN, GPIO_PIN_HIGH);



}
/**
 * \brief  LED2 off
 *
 *
 */
void OffLed2(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED2_PIN, GPIO_PIN_LOW);
}

/**
 * \brief  LED3 on
 *
 *
 */
void OnLed3(void) {
	GPIOPinWrite(LED_REGISTER, LED3_PIN, GPIO_PIN_HIGH);



}
/**
 * \brief  LED3 off
 *
 *
 */
void OffLed3(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED3_PIN, GPIO_PIN_LOW);
}


