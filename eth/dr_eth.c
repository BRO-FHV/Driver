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

typedef struct ethIf
{
    unsigned int instNum;   /* Instance Number */
    unsigned int slvPortNum; /* CPSW Slave Port Number */
    unsigned int ipAddr; /* IP Address */
    unsigned int netMask; /* Net Mask */
    unsigned int gwAddr; /* Gate Way Address */
    unsigned char macArray[LEN_MAC_ADDRESS]; /* MAC Address to be used*/
} ETH_IF;

/**
 * \brief   Configure the ethernet port with a ip.
 *
 * \param   ip			ip address as hex value
 * 			netMask		netmask as hex value
 * 			gateway		standard gateway address as hex value
 *
 * \return	If setting the IP was successful the IP is returned, 0 otherwise.
 *
 **/
uint32_t EthConfigureWithIP(uint32_t ip, uint32_t netMask, uint32_t gateway) {
	ETH_IF ethPort;

	CPSWPinMuxSetup();
	CPSWClkEnable();

	CPSWEVMPortMIIModeSelect();

	// Get the MAC address
	CPSWEVMMACAddrGet(0, ethPort.macArray);

	//Configure Interrupt handler
	InterruptSetup();

	ethPort.instNum = 0;
	ethPort.slvPortNum = 1;
	ethPort.ipAddr = ip;
	ethPort.netMask = netMask;
	ethPort.gwAddr = ethPort;

	uint32_t ipAddr = (uint32_t)lwIPInit(&ethPort);

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
	//asm("	CPS		0x1F");

	lwIPRxIntHandler(0);
}

/*
 ** Interrupt Handler for transmit interrupt
 */
void CPSWCore0TxIsr() {
	lwIPTxIntHandler(0);
}
