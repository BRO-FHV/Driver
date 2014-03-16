/*
 * Driver: debug.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * Implementation for debug console
 */

#include <inttypes.h>
#include <soc_AM335x.h>
#include <stdlib.h>
#include <string.h>
#include "dr_debug.h"
#include "../uart/dr_uart.h"
#include "../watch/dr_watch.h"

#define BAUD_RATE_115200 (115200)

static char* PrepareMessage(const char* const type, char sender[], char message[]);

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
void DebugEnable(uint32_t baseAddr) {
	uartBaseAddr = baseAddr;
	UartEnable(SOC_UART_0_REGS);
	UartConfigure(SOC_UART_0_REGS, BAUD_RATE_115200);
}

/**
 * \brief Send Log Message
 */
void DebugLog(char sender[], char message[]) {
	char* logMessage = PrepareMessage(logType, sender, message);

	UartWrite(uartBaseAddr, logMessage, strlen(logMessage) - 1);
}

/**
 * \brief prepare insternal message
 */
static char* PrepareMessage(const char* const type, char sender[], char message[]) {
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
