/*
 * Driver: dr_eth.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 04.04.2014
 * Description: 
 *
 */

#include "dr_eth.h"
#include "lwip/lwiplib.h"
#include "lwip/lwipopts.h"

void CPSWCore0RxIsr(void);
void CPSWCore0TxIsr(void);
void AintcCPSWIntrSetUp(void);
void IpAddrDisplay(unsigned int ipAddr);
void MMUConfigAndEnable(void);

void ConfigureCore(LWIP_IF* lwipIfPort, uint32_t ip);

/**
 * \brief   Configure the ethernet port with a ip.
 *
 * \param   ip		For Example, for IP Address 192.168.247.1, use the corresponding hex value 0xC0A8F701.
 **/
bool EthConfigureWithIP(uint32_t ip) {
	uint32_t ipAddr;
	LWIP_IF lwipIfPort;

//    MMUConfigAndEnable();
//
//	#ifdef LWIP_CACHE_ENABLED
//	    CacheEnable(CACHE_ALL);
//	#endif

	CPSWPinMuxSetup();
	CPSWClkEnable();

	/* Chip configuration RGMII selection */
	EVMPortRGMIIModeSelect();

	AintcCPSWIntrSetUp();
	DelayTimerSetup();

	ConfigureCore(&lwipIfPort, ip);
	ipAddr = lwIPInit(&lwipIfPort);

	/* Initialize the sample httpd server. */
	echo_init();

	return ipAddr;
}

/**
 * \brief   Configure the ethernet port and activate DHCP mode.
 *
 * \return	The specific enum value that indicates the final state of the configuration.
 **/
EthDHCPState EthConfigureWithDHCP() {
	ConfigurationCore()
	ipAddr = lwIPInit(&lwipIfPort1);

	return ipAddr;
}

void ConfigureCore(LWIP_IF* lwipIfPort, uint32_t ip) {
	if (ip) {
		lwipIfPort->ipMode = IPADDR_USE_STATIC;
	} else {
		lwipIfPort->ipMode = IPADDR_USE_DHCP;
	}

	lwipIfPort->instNum = 0;
	lwipIfPort->slvPortNum = 1;
	lwipIfPort->ipAddr = ip;
	lwipIfPort->netMask = 0;
	lwipIfPort->gwAddr = 0;
}

/*
 ** Interrupt Handler for Core 0 Receive interrupt
 */
void CPSWCore0RxIsr(void) {
	lwIPRxIntHandler(0);
}

/*
 ** Interrupt Handler for Core 0 Transmit interrupt
 */
void CPSWCore0TxIsr(void) {
	lwIPTxIntHandler(0);
}

/*
 ** Displays the IP addrss on the Console
 */
void IpAddrDisplay(unsigned int ipAddr) {
	ConsoleUtilsPrintf("%d.%d.%d.%d", (ipAddr & 0xFF), ((ipAddr >> 8) & 0xFF),
			((ipAddr >> 16) & 0xFF), ((ipAddr >> 24) & 0xFF));
}

/*
 ** Set up the ARM Interrupt Controller for generating timer interrupt
 */
void AintcCPSWIntrSetUp(void) {
	/* Enable IRQ for ARM (in CPSR)*/
	IntMasterIRQEnable();

	IntAINTCInit();

	/* Register the Receive ISR for Core 0 */
	IntRegister(SYS_INT_3PGSWRXINT0, CPSWCore0RxIsr);

	/* Register the Transmit ISR for Core 0 */
	IntRegister(SYS_INT_3PGSWTXINT0, CPSWCore0TxIsr);

	/* Set the priority */
	IntPrioritySet(SYS_INT_3PGSWTXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);
	IntPrioritySet(SYS_INT_3PGSWRXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);

	/* Enable the system interrupt */
	IntSystemEnable(SYS_INT_3PGSWTXINT0);
	IntSystemEnable(SYS_INT_3PGSWRXINT0);
}
