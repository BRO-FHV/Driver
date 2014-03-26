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

void InitLedRegister(void) {
	GPIO1ModuleClkConfig();

	/* Enabling the GPIO module. */
	GPIOModuleEnable(LED_REGISTER);

	/* Resetting the GPIO module. */
	GPIOModuleReset(LED_REGISTER);


}

void InitLed0(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED0_PIN, GPIO_DIR_OUTPUT);
}

void InitLed1(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED1_PIN, GPIO_DIR_OUTPUT);
}

void InitLed2(void){

	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED2_PIN, GPIO_DIR_OUTPUT);
}

void InitLed3(void) {
	/* Setting the GPIO pin as an output pin. */
	GPIODirModeSet(LED_REGISTER, LED3_PIN, GPIO_DIR_OUTPUT);
}

void OnLed0(void) {
	GPIOPinWrite(LED_REGISTER, LED0_PIN, GPIO_PIN_HIGH);



}

void OffLed0(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED0_PIN, GPIO_PIN_LOW);
}

void OnLed1(void) {
	GPIOPinWrite(LED_REGISTER, LED1_PIN, GPIO_PIN_HIGH);



}

void OffLed1(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED1_PIN, GPIO_PIN_LOW);
}


void OnLed2(void) {
	GPIOPinWrite(LED_REGISTER, LED2_PIN, GPIO_PIN_HIGH);



}

void OffLed2(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED2_PIN, GPIO_PIN_LOW);
}

void OnLed3(void) {
	GPIOPinWrite(LED_REGISTER, LED3_PIN, GPIO_PIN_HIGH);



}

void OffLed3(void){

	/* Driving a logic LOW on the GPIO pin. */
		GPIOPinWrite(LED_REGISTER, LED3_PIN, GPIO_PIN_LOW);
}


