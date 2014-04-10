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
#include "../interrupt/dr_interrupt.h"


//#include "soc_AM335x.h"
//#include "hw_control_AM335x.h"
//#include "hw_types.h"
//#include "beaglebone.h"
//#include "hw_cm_per.h"

#define CPSW_MII_SEL_MODE		(0x00u)
#define CPSW_MDIO_SEL_MODE		(0x00u)
#define LEN_MAC_ADDR			(0x06u)
#define OFFSET_MAC_ADDR			(0x30u)

void CPSWCore0RxIsr();
void CPSWCore0TxIsr();
void AintcCPSWIntrSetUp();
void IpAddrDisplay(uint32_t ipAddr);
void MMUConfigAndEnable();

void ConfigureCore(LWIP_IF* lwipIfPort, uint32_t ip);
void CPSWPinMuxSetup();
void CPSWClkEnable();
void EVMPortMIIModeSelect();
void EVMPortRGMIIModeSelect();
void EVMMACAddrGet(uint32_t addrIdx, uint8_t *macAddr);

void CPSWCore0TxIsr();
void CPSWCore0RxIsr();

/**
 * \brief   Configure the ethernet port with a ip.
 *
 * \param   ip		For Example IP Address 192.168.0.7 use the corresponding hex value 0xC0A80007
 **/
uint32_t EthConfigureWithIP(uint32_t ip) {
	uint32_t ipAddr;
	LWIP_IF lwipIfPort;

	ConfigureCore(&lwipIfPort, ip);
	ipAddr = lwIPInit(&lwipIfPort);

	return ipAddr;
}

/**
 * \brief   Configure the ethernet port and activate DHCP mode.
 *
 * \return	The specific enum value that indicates the final state of the configuration.
 **/
uint32_t EthConfigureWithDHCP() {
	uint32_t ipAddr;
	LWIP_IF lwipIfPort;

	ConfigureCore(&lwipIfPort);
	ipAddr = lwIPInit(&lwipIfPort);

	return ipAddr;
}

void ConfigureCore(LWIP_IF* lwipIfPort, uint32_t ip = 0) {
	CPSWPinMuxSetup();
	CPSWClkEnable();

	//Chip configuration MII selection
	EVMPortMIIModeSelect();
	//Chip configuration RGMII selection
	//EVMPortRGMIIModeSelect();

	// Get the MAC address
	EVMMACAddrGet(0, lwipIfPort->macArray);

	DelayTimerSetup();

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
 ** Set up the interrupt Controller for generating timer interrupt
 */
void AintcCPSWIntrSetUp() {
	/* Register the Receive ISR for Core 0 */
	IntRegister(SYS_INT_3PGSWRXINT0, CPSWCore0RxIsr);

	/* Register the Transmit ISR for Core 0 */
	IntRegister(SYS_INT_3PGSWTXINT0, CPSWCore0TxIsr);

	/* Set the priority */
	IntPrioritySet(SYS_INT_3PGSWTXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);
	IntPrioritySet(SYS_INT_3PGSWRXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);

	//TODO CHECK IF NEEDED
	/* Enable the system interrupt */
	//IntSystemEnable(SYS_INT_3PGSWTXINT0);
	//IntSystemEnable(SYS_INT_3PGSWRXINT0);
}

/**
 * \brief   This function selects the CPSW pins for use in MII mode.
 *
 * \param   None
 *
 * \return  None.
 *
 */
void CPSWPinMuxSetup()
{
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RX_ER, CONTROL_CONF_MII1_RXERR_CONF_MII1_RXERR_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXEN, CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXDV, CONTROL_CONF_MII1_RXDV_CONF_MII1_RXDV_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD3, CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD2, CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD1, CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD0, CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXCLK, CONTROL_CONF_MII1_TXCLK_CONF_MII1_TXCLK_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXCLK, CONTROL_CONF_MII1_RXCLK_CONF_MII1_RXCLK_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD3, CONTROL_CONF_MII1_RXD3_CONF_MII1_RXD3_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD2, CONTROL_CONF_MII1_RXD2_CONF_MII1_RXD2_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD1, CONTROL_CONF_MII1_RXD1_CONF_MII1_RXD1_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD0, CONTROL_CONF_MII1_RXD0_CONF_MII1_RXD0_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_COL, CONTROL_CONF_MII1_COL_CONF_MII1_COL_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_CRS, CONTROL_CONF_MII1_CRS_CONF_MII1_CRS_RXACTIVE | CPSW_MII_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MDIO_DATA, CONTROL_CONF_MDIO_DATA_CONF_MDIO_DATA_RXACTIVE | CONTROL_CONF_MDIO_DATA_CONF_MDIO_DATA_PUTYPESEL | CPSW_MDIO_SEL_MODE);
	  reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MDIO_CLK, CONTROL_CONF_MDIO_CLK_CONF_MDIO_CLK_PUTYPESEL | CPSW_MDIO_SEL_MODE);
}

/**
 * \brief   Enables CPSW clocks (enable registers in PRCM)
 *
 * \param   None
 *
 * \return  None.
 */
void CPSWClkEnable()
{
	reg32w(SOC_PRCM_REGS, CM_PER_CPGMAC0_CLKCTRL, CM_PER_CPGMAC0_CLKCTRL_MODULEMODE_ENABLE);

    wait(0 != (HWREG(SOC_PRCM_REGS + CM_PER_CPGMAC0_CLKCTRL) & CM_PER_CPGMAC0_CLKCTRL_IDLEST));

    reg32w(SOC_PRCM_REGS, CM_PER_CPSW_CLKSTCTRL, CM_PER_CPSW_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

    wait(0 == (HWREG(SOC_PRCM_REGS + CM_PER_CPSW_CLKSTCTRL) & CM_PER_CPSW_CLKSTCTRL_CLKACTIVITY_CPSW_125MHZ_GCLK));
}

/**
 * \brief   This function sets the MII mode for both ports
 *
 * \param   None
 *
 * \return  None.
 */
void EVMPortMIIModeSelect()
{
    /* Select MII, Internal Delay mode */
    reg32w(SOC_CONTROL_REGS, CONTROL_GMII_SEL, 0x00);
}

/**
 * \brief   This function sets the RGMII mode for both ports
 *
 * \param   None
 *
 * \return  None.
 */
void EVMPortRGMIIModeSelect()
{
    /* Select RGMII, Internal Delay mode */
	reg32w(SOC_CONTROL_REGS, CONTROL_GMII_SEL, 0x0A);
}

/**
 * \brief   This function returns the MAC address for the EVM
 *
 * \param   addrIdx    the MAC address index.
 * \param   macAddr    the Pointer where the MAC address shall be stored; 'addrIdx' can be either 0 or 1
 *
 * \return  None.
 */
void EVMMACAddrGet(uint32_t addrIdx, uint8_t *macAddr)
{
    macAddr[0] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_LO(addrIdx)) >> 8) & 0xFF;
    macAddr[1] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_LO(addrIdx))) & 0xFF;
    macAddr[2] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx)) >> 24) & 0xFF;
    macAddr[3] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx)) >> 16) & 0xFF;
    macAddr[4] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx)) >> 8) & 0xFF;
    macAddr[5] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx))) & 0xFF;
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
