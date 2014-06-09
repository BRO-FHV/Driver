/*
 * Driver: dr_console.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * Implementation for debug console
 */

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <interrupt/hw_interrupt.h>
#include "dr_console.h"
#include "../interrupt/dr_interrupt.h"
#include "../uart/dr_uart.h"
#include "../watch/dr_watch.h"

#define BAUD_RATE_115200 (115200)

static char* PrepareMessage(const char* const type, char sender[],
		char message[]);

// buffer for string concat
char buffer[256] = "";

// Seperator LogMessage
const char* const sepColon = ": ";
const char* const sepBlank = " ";
const char* const end = "\r\n";

// Type strings
const char* const logType = "LOG";

uint16_t index = 0;
uint32_t uartBaseAddr;

/**
 * \brief Enables Debug Console
 */
void ConsoleEnable(uint32_t baseAddr) {
	// save base address of used module
	uartBaseAddr = baseAddr;
	// enable uart module
	UartEnable(baseAddr);
	// configure uart module with 115200 baud rate
	UartConfigure(baseAddr, BAUD_RATE_115200);

	// enable uart interrupt
	UartSystemIntEnable();

	// enable all uart interrupts
	UartIntEnable(uartBaseAddr,
			(UART_INT_LINE_STAT | UART_INT_RHR_CTI));
}

/**
 * \brief Send Log Message
 */
void ConsoleLog(char sender[], char message[]) {
	char* logMessage = PrepareMessage(logType, sender, message);

	UartWrite(uartBaseAddr, logMessage, strlen(logMessage));
}

/**
 * \brief Send Log Message with variable amount of arguments
 */
void ConsoleLogf(char sender[], const char *string, ...) {

	va_list arg;
	va_start (arg, string);

	UartWritef(uartBaseAddr,  string, arg);

	va_end(arg);
}

/**
 * \brief prepare internal message
 */
static char* PrepareMessage(const char* const type, char sender[],
		char message[]) {
	buffer[0] = '\0';

	strcat(buffer, WatchCurrentTimeStampString());
	strcat(buffer, sepBlank);
	strcat(buffer, type);
	strcat(buffer, sepBlank);
	strcat(buffer, sender);
	strcat(buffer, sepColon);
	strcat(buffer, message);
	strcat(buffer, end);

	return buffer;
}
