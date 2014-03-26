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
void LedInitRegister(void) {
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
void LedInit0(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED0_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED1 enable
 *
 *
 */
void LedInit1(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED1_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED2 enable
 *
 *
 */
void LedInit2(void){

	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED2_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED3 enable
 *
 *
 */
void LedInit3(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED3_PIN, GPIO_DIR_OUTPUT);
}

/**
 * \brief  LED0 on
 *
 *
 */
void LedOn0(void) {
	GPIOPinWrite(LED_REGISTER, LED0_PIN, GPIO_PIN_HIGH);



}

/**
 * \brief  LED1 off
 *
 *
 */
void LedOff0(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED0_PIN, GPIO_PIN_LOW);
}

/**
 * \brief  LED1 on
 *
 *
 */
void LedOn1(void) {
	GPIOPinWrite(LED_REGISTER, LED1_PIN, GPIO_PIN_HIGH);



}

/**
 * \brief  LED2 off
 *
 *
 */
void LedOff1(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED1_PIN, GPIO_PIN_LOW);
}


void LedOn2(void) {
	GPIOPinWrite(LED_REGISTER, LED2_PIN, GPIO_PIN_HIGH);



}
/**
 * \brief  LED2 off
 *
 *
 */
void LedOff2(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED2_PIN, GPIO_PIN_LOW);
}

/**
 * \brief  LED3 on
 *
 *
 */
void LedOn3(void) {
	GPIOPinWrite(LED_REGISTER, LED3_PIN, GPIO_PIN_HIGH);



}
/**
 * \brief  LED3 off
 *
 *
 */
void LedOff3(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED3_PIN, GPIO_PIN_LOW);
}


