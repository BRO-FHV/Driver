/*
 * Driver: dr_eth.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 04.04.2014
 * Description: 
 *
 */
#include <stdio.h>
#include "dr_eth.h"
#include "cpsw/dr_cpsw.h"
#include "lwip/ports/cpsw/include/lwiplib.h"
#include "../timer/dr_timer.h"
#include "../interrupt/dr_interrupt.h"

uint32_t ConfigureCore(uint32_t ip);
void CPSWCore0RxIsr();
void CPSWCore0TxIsr();

void InterruptSetup();

/**
 * \brief   Configure the ethernet port with a ip.
 *
 * \param   ip		For Example IP Address 192.168.0.7 use the corresponding hex value 0xC0A80007
 *
 * \return	IF setting the IP was successful the IP is returned, 0 otherwise.
 *
 **/
uint32_t EthConfigureWithIP(uint32_t ip) {
	return ConfigureCore(ip);
}

uint32_t ConfigureCore(uint32_t ip) {
	LWIP_IF lwipIfPort1, lwipIfPort2;

	#ifdef LWIP_CACHE_ENABLED
		CacheEnable(CACHE_ALL);
	#endif

	CPSWPinMuxSetup();
	CPSWClkEnable();

	CPSWEVMPortMIIModeSelect();

	// Get the MAC address
	CPSWEVMMACAddrGet(0, lwipIfPort1.macArray);
	CPSWEVMMACAddrGet(1, lwipIfPort2.macArray);

	//Configure Interrupt handler
	InterruptSetup();

	lwipIfPort1.ipMode = IPADDR_USE_STATIC;
	lwipIfPort1.instNum = 0;
	lwipIfPort1.slvPortNum = 1;
	lwipIfPort1.ipAddr = ip;
    lwipIfPort1.netMask = 0;
    lwipIfPort1.gwAddr = 0;

	uint32_t ipAddr = (uint32_t)lwIPInit(&lwipIfPort1);

	if(0 == ipAddr) {
		printf("\n\rUnable to get IP-Address!");


	} else {
		printf("\n\rUsing IP-Addr: %d.%d.%d.%d\n\r", (ipAddr & 0xFF), ((ipAddr >> 8) & 0xFF), ((ipAddr >> 16) & 0xFF), ((ipAddr >> 24) & 0xFF));
	}

	return ipAddr;
}

/*
 ** Set up the interrupt Controller for generating timer interrupt
 */
void InterruptSetup() {
	/* Register the Receive ISR for Core 0 */
	IntRegister(SYS_INT_3PGSWRXINT0, CPSWCore0RxIsr);

	/* Register the Transmit ISR for Core 0 */
	IntRegister(SYS_INT_3PGSWTXINT0, CPSWCore0TxIsr);

	/* Set the priority */
	IntPrioritySet(SYS_INT_3PGSWTXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);
	IntPrioritySet(SYS_INT_3PGSWRXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enable the system interrupt */
    IntHandlerEnable(SYS_INT_3PGSWTXINT0);
    IntHandlerEnable(SYS_INT_3PGSWRXINT0);
}

/*
 ** Interrupt Handler for receive interrupt
 */
void CPSWCore0RxIsr() {
	lwIPRxIntHandler(0);
}

/*
 ** Interrupt Handler for transmit interrupt
 */
void CPSWCore0TxIsr() {
	lwIPTxIntHandler(0);
}
