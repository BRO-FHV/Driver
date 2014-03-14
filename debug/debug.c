/*
 * Driver: debug.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: Mar 14, 2014
 * Description: 
 * TODO
 */

#include <inttypes.h>
#include "debug.h"
#include "../uart/uart0.h"

#define BAUD_RATE_115200 (115200)

uint16_t index = 0;

void DebugEnable() {
	Uart0Configure(BAUD_RATE_115200);
	Uart0Enable();
}

void DebugLog(uint32_t sender, char message[]) {
	char buf[256];
	snprintf(buf, sizeof buf, "[Sender: %s (%s)]: %s", sender, index, message);
	++index;

	Uart0Write(buf);
}
