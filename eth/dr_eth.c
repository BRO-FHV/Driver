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

#define LEN_MAC_ADDRESS                    (6)

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
	ethPort.gwAddr = gateway;

	uint32_t ipAddr = (uint32_t)Init(&ethPort);

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

/**
 *
 * \brief Initializes the lwIP TCP/IP stack.
 *
 * \param lwipIf  The interface structure for lwIP
 *
 * \return IP Address.
*/
unsigned int Init(LWIP_IF *lwipIf)
{
    struct ip_addr ip_addr;
    struct ip_addr net_mask;
    struct ip_addr gw_addr;
    unsigned int *ipAddrPtr;
    static unsigned int lwipInitFlag = 0;
    unsigned int ifNum;
    unsigned int temp;

    /* do lwip library init only once */
    if(0 == lwipInitFlag)
    {
        lwip_init();
    }

    /* Setup the network address values. */
    if(lwipIf->ipMode == IPADDR_USE_STATIC)
    {
        ip_addr.addr = htonl(lwipIf->ipAddr);
        net_mask.addr = htonl(lwipIf->netMask);
        gw_addr.addr = htonl(lwipIf->gwAddr);
    }
    else
    {
        ip_addr.addr = 0;
        net_mask.addr = 0;
        gw_addr.addr = 0;
    }

#ifdef CPSW_DUAL_MAC_MODE
    ifNum = (lwipIf->instNum * MAX_SLAVEPORT_PER_INST) + lwipIf->slvPortNum - 1;
#else
    ifNum = lwipIf->instNum;
#endif

    cpswPortIf[ifNum].inst_num = lwipIf->instNum;
    cpswPortIf[ifNum].port_num = lwipIf->slvPortNum;

    /* set MAC hardware address */
    for(temp = 0; temp < LEN_MAC_ADDRESS; temp++)
    {
        cpswPortIf[ifNum].eth_addr[temp] =
                         lwipIf->macArray[(LEN_MAC_ADDRESS - 1) - temp];
    }

    /*
    ** Create, configure and add the Ethernet controller interface with
    ** default settings.  ip_input should be used to send packets directly to
    ** the stack. The lwIP will internaly call the cpswif_init function.
    */
    if(NULL ==
       netif_add(&cpswNetIF[ifNum], &ip_addr, &net_mask, &gw_addr,
                 &cpswPortIf[ifNum], cpswif_init, ip_input))
    {
        LWIP_PRINTF("\n\rUnable to add interface for interface %d", ifNum);
        return 0;
    }

    if(0 == lwipInitFlag)
    {
        netif_set_default(&cpswNetIF[ifNum]);
        lwipInitFlag = 1;
    }

    /* Start DHCP, if enabled. */
#if LWIP_DHCP
    if(lwipIf->ipMode == IPADDR_USE_DHCP)
    {
        lwIPDHCPComplete(ifNum);
    }
#endif

    /* Start AutoIP, if enabled and DHCP is not. */
#if LWIP_AUTOIP
    if(lwipIf->ipMode == IPADDR_USE_AUTOIP)
    {
        autoip_start(&cpswNetIF[ifNum]);
    }
#endif

    if((lwipIf->ipMode == IPADDR_USE_STATIC)
       ||(lwipIf->ipMode == IPADDR_USE_AUTOIP))
    {
       /* Bring the interface up */
       netif_set_up(&cpswNetIF[ifNum]);
    }

    ipAddrPtr = (unsigned int*)&(cpswNetIF[ifNum].ip_addr);

    return (*ipAddrPtr);
}


/*
 * \brief   Checks if the ethernet link is up
 *
 * \param   instNum     The instance number of CPSW module
 * \param   slvPortNum  The Slave Port Number
 *
 * \return  Interface status.
*/
unsigned int lwIPNetIfStatusGet(unsigned int instNum, unsigned int slvPortNum)
{
    unsigned int ifNum;

    ifNum = instNum * MAX_SLAVEPORT_PER_INST + slvPortNum - 1;

    return (cpswif_netif_status(&cpswNetIF[ifNum]));
}

/*
 * \brief   Checks if the ethernet link is up
 *
 * \param   instNum     The instance number of CPSW module
 * \param   slvPortNum  The Slave Port Number
 *
 * \return  The link status.
*/
unsigned int lwIPLinkStatusGet(unsigned int instNum, unsigned int slvPortNum)
{
    return (cpswif_link_status(instNum, slvPortNum));
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
