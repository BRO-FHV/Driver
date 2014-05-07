/*
 * Driver: dr_cpsw.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 11.04.2014
 * Description:
 *
 */

/**
 *  \file   cpsw.c
 *
 *  \brief  CPSW device abstraction layer APIs.
 *
 *   This file contains the device abstraction layer APIs for CPSW RGMII.
 */

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
*/
/*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#include "soc_AM335x.h"
#include "hw_control_AM335x.h"
#include "hw_cm_per.h"

#define CPSW_MII_SEL_MODE                     (0x00u)
#define CPSW_MDIO_SEL_MODE                    (0x00u)

#include "basic.h"
#include "dr_cpsw.h"

/*******************************************************************************
*                       INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define CPSW_CORE_OFFSET                   (0x10u)
#define CPSW_MAX_HEADER_DESC               (0x08u)
#define CPDMA_P0_DEF_TX_MAP                (0x76543210u)
#define ALE_ENTRY_WORDS                    (0x03u)
#define CPDMA_ERR_CHANNEL_POS              (0xFFu)
#define CPSW_PORT_DUAL_MAC_MODE            (0x01u <<                        \
                                            CPSW_PORT_P0_TX_IN_CTL_TX_IN_SEL_SHIFT)
#define CPSW_PORT_RATE_LIM_MODE            (0x02u <<                        \
                                            CPSW_PORT_P0_TX_IN_CTL_TX_IN_SEL_SHIFT)

/*******************************************************************************
*                        API FUNCTION DEFINITIONS
*******************************************************************************/
/**
 * \brief   Resets the CPSW Subsystem.
 *
 * \param   baseAddr    Base address of the CPSW Subsystem
 *
 * \return  None
 **/
void CPSWSSReset(uint32_t baseAddr)
{
    /* Reset the CPSW */
    reg32w(baseAddr, CPSW_SS_SOFT_RESET, CPSW_SS_SOFT_RESET_SOFT_RESET);

    wait(reg32r(baseAddr, CPSW_SS_SOFT_RESET) & CPSW_SS_SOFT_RESET_SOFT_RESET);
}

/**
 * \brief   Force the CPGMAC_SL into gigabit mode if the input GMII_MTCLK has
 *          been stopped by the PHY
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 *
 * \return  None
 *
 **/
void CPSWSlGigModeForceEnable(uint32_t baseAddr)
{
	reg32m(baseAddr, CPSW_SL_MACCONTROL, CPSW_SL_MACCONTROL_GIG_FORCE);
}

/**
 * \brief   Enables the fullduplex and gigabit mode to be selected
 *          from the FULLDUPLEX_IN and GIG_IN input signals and not from the
 *          fullduplex and gig bits in this register. The FULLDUPLEX_MODE bit
 *          reflects the actual fullduplex mode selected
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 *
 * \return  None
 *
 **/
void CPSWSlControlExtEnable(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_SL_MACCONTROL, CPSW_SL_MACCONTROL_EXT_EN);
}


/**
 * \brief   Disables the CPGMAC_SL gigabit mode if the input GMII_MTCLK has
 *          been stopped by the PHY
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 *
 * \return  None
 *
 **/
void CPSWSlGigModeForceDisable(uint32_t baseAddr)
{
	reg32an(baseAddr, CPSW_SL_MACCONTROL, CPSW_SL_MACCONTROL_GIG_FORCE);
}

/**
 * \brief   Sets the Transfer mode, 10/100 or gigabit mode  and the duplex
 *          mode  for the sliver.
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 * \param   mode        The transfer mode
 *    'mode' can take one of the below values. \n
 *         CPSW_SLIVER_NON_GIG_HALF_DUPLEX - 10/100 Mbps mode, half duplex. \n
 *         CPSW_SLIVER_NON_GIG_FULL_DUPLEX - 10/100 Mbps mode, full duplex. \n
 *         CPSW_SLIVER_GIG_FULL_DUPLEX - 1000 Mbps mode, full duplex. \n
 *         Note: for 10Mbps mode, CPSW_SLIVER_INBAND has to be configured. \n
 *
 * \return  None
 *
 **/
void CPSWSlTransferModeSet(uint32_t baseAddr, uint32_t mode)
{
	reg32an(baseAddr, CPSW_SL_MACCONTROL, CPSW_SL_MACCONTROL_GIG | CPSW_SL_MACCONTROL_FULLDUPLEX);

	reg32m(baseAddr, CPSW_SL_MACCONTROL, mode);
}

/**
 * \brief   Returns the MAC Status. The value will be the contents of the MAC
 *          status register.
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 * \param   statFlag    Status flags to be read.
 *            statFlag can take an OR combination of the below values. \n
 *              CPSW_SLIVER_STATE - The Sliver state. \n
 *              CPSW_SLIVER_EXT_GIG_INPUT_BIT - The EXT_GIG input bit mask.\n
 *              CPSW_SLIVER_EXT_FULL_DUPLEX_BIT - The EXT_FULLDUPLEX input
 *                                                  bit mask. \n
 *              CPSW_SLIVER_RX_FLOWCTRL - The receive flow control active. \n
 *              CPSW_SLIVER_TX_FLOWCTRL - The transmit flow control.
 *
 *
 * \return  MAC Status
 *            The MAC status register value returned can be compared against
 *            the below tokens. \n
 *              CPSW_SLIVER_STATE_IDLE - The Sliver is in idle state. \n
 *              CPSW_SLIVER_EXT_GIG_INPUT_HIGH - The EXT_GIG input
 *                                                 bit is in HIGH state.\n
 *              CPSW_SLIVER_EXT_FULL_DUPLEX_HIGH - The EXT_FULLDUPLEX input
 *                                                   bit is in HIGH state. \n
 *              CPSW_SLIVER_RX_FLOWCTRL_ACTIVE - The receive flow control is
 *                                                 active. \n
 *              CPSW_SLIVER_TX_FLOWCTRL_ACTIVE - The pause time period is
 *                                                 observed for a received
 *                                                 pause frame
 *
 **/
uint32_t CPSWSlMACStatusGet(uint32_t baseAddr, uint32_t statFlag)
{
    /* Return the required status only */
    return (reg32r(baseAddr, CPSW_SL_MACSTATUS) & statFlag);
}

/**
 * \brief   Resets the CPSW Sliver Logic.
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 *
 * \return  None
 *
 **/
void CPSWSlReset(uint32_t baseAddr)
{
    /* Reset the sliver logic */
    reg32w(baseAddr, CPSW_SL_SOFT_RESET, CPSW_SL_SOFT_RESET_SOFT_RESET);

    /* Wait till the reset completes */
    wait(CPSW_SL_SOFT_RESET_SOFT_RESET == ((reg32r(baseAddr, CPSW_SL_SOFT_RESET)) & CPSW_SL_SOFT_RESET_SOFT_RESET));
}


/**
 * \brief   Sets the maximum length for received frame.
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 * \param   rxMaxLen    Maximum length for a received frame
 *     The default value for 'rxMaxLen' is 1518. The maximum value
 *     which can be set is 16383.
 *
 * \return  None
 *
 **/
void CPSWSlRxMaxLenSet(uint32_t baseAddr, uint32_t rxMaxLen)
{
    /* Set the desired maximum length */
    reg32w(baseAddr, CPSW_SL_RX_MAXLEN, rxMaxLen);
}

/**
 * \brief   Enables GMII for the sliver.
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 *
 * \return  None
 **/
void CPSWSlGMIIEnable(uint32_t baseAddr)
{
	reg32m(baseAddr, CPSW_SL_MACCONTROL, CPSW_SL_MACCONTROL_GMII_EN);
}

/**
 * \brief   Enables RGMII for the sliver.
 *
 * \param   baseAddr    Base address of the CPSW Sliver Module registers.
 *
 * \return  None
 **/
void CPSWSlRGMIIEnable(uint32_t baseAddr)
{
	reg32m(baseAddr, CPSW_SL_MACCONTROL, (CPSW_SL_MACCONTROL_GMII_EN | CPSW_SL_MACCONTROL_IFCTL_A | CPSW_SL_MACCONTROL_IFCTL_B));
}

/**
 * \brief   Resets the CPSW Wrapper module.
 *
 * \param   baseAddr    Base address of the CPSW Wrapper Module
 *
 * \return  None
 **/
void CPSWWrReset(uint32_t baseAddr)
{
    /* Reset the CPSW Wrapper */
    reg32w(baseAddr, CPSW_WR_SOFT_RESET, CPSW_WR_SOFT_RESET_SOFT_RESET);

    wait(reg32r(baseAddr, CPSW_WR_SOFT_RESET) & CPSW_WR_SOFT_RESET_SOFT_RESET);
}

/**
 * \brief   Resets the Control Register of CPSW Wrapper module
 *
 * \param   baseAddr    Base address of the CPSW Wrapper Module
 *
 * \return  None
 **/
void CPSWWrControlRegReset(uint32_t baseAddr)
{
    /* Reset the CPSW Wrapper control Register */
    reg32w(baseAddr, CPSW_WR_CONTROL, CPSW_WR_CONTROL_MMR_RESET);
}

/**
 * \brief   Enables an interrupt for the specified core.
 *
 * \param   baseAddr    Base address of the CPSW Wrapper Module
 * \param   core        Core number
 * \param   channel     Channel number
 * \param   intFlag     Interrupt to be enabled
 *    'intFlag' can take one of the below values. \n
 *          CPSW_CORE_INT_RX_THRESH - RX threshold interrupt \n
 *          CPSW_CORE_INT_RX_PULSE - RX pulse interrupt \n
 *          CPSW_CORE_INT_TX_PULSE - TX pulse interrupt \n
 *          CPSW_CORE_INT_MISC - Miscellaneous interrupt
 *
 * \return  None
 **/
void CPSWWrCoreIntEnable(uint32_t baseAddr, uint32_t core, uint32_t channel, uint32_t intFlag)
{
    reg32m(baseAddr, CPSW_WR_C_RX_THRESH_EN(core) + intFlag, (1 << channel));
}

/**
 * \brief   Disables an interrupt for the specified core.
 *
 * \param   baseAddr    Base address of thei CPSW Wrapper Module
 * \param   core        Core number
 * \param   channel     Channel number
 * \param   intFlag     Interrupt to be disabled
 *    'intFlag' can take one of the below values. \n
 *          CPSW_CORE_INT_RX_THRESH - RX threshold interrupt \n
 *          CPSW_CORE_INT_RX_PULSE - RX pulse interrupt \n
 *          CPSW_CORE_INT_TX_PULSE - TX pulse interrupt \n
 *          CPSW_CORE_INT_MISC - Miscellaneous interrupt
 *
 * \return  None
 **/
void CPSWWrCoreIntDisable(uint32_t baseAddr, uint32_t core, uint32_t channel, uint32_t intFlag)
{
	reg32an(baseAddr, CPSW_WR_C_RX_THRESH_EN(core) + intFlag, (1 << channel));
}

/**
 * \brief   Returns the interrupt status of the core for the specified
 *          channel
 *
 * \param   baseAddr    Base address of thei CPSW Wrapper Module
 * \param   core        Core number
 * \param   channel     Channel number
 * \param   intFlag     Interrupt status to be read
 *    'intFlag' can take one of the below values. \n
 *          CPSW_CORE_INT_RX_THRESH - RX threshold interrupt \n
 *          CPSW_CORE_INT_RX_PULSE - RX pulse interrupt \n
 *          CPSW_CORE_INT_TX_PULSE - TX pulse interrupt \n
 *          CPSW_CORE_INT_MISC - Miscellaneous interrupt
 *
 * \return  same as intFlag if the status is set
 *          '0' if the status is cleared
 **/
uint32_t CPSWWrCoreIntStatusGet(uint32_t baseAddr, uint32_t core, uint32_t channel, uint32_t intFlag)
{
    return (reg32r(baseAddr, CPSW_WR_C_RX_THRESH_STAT(core) + intFlag) & (1 << channel));
}

/**
 * \brief   Returns the RGMII status requested.
 *
 * \param   baseAddr    Base address of the CPSW Wrapper Module
 * \param   statFlag    Status to be checked
 *    'statFlag' can take a combination of the below values. \n
 *        CPSW_RGMII2_DUPLEX - Duplex of RGMII2 \n
 *        CPSW_RGMII2_SPEED - Speed of RGMII2 \n
 *        CPSW_RGMII2_LINK_STAT - Link Status of RGMII2 \n
 *        CPSW_RGMII1_DUPLEX - Duplex of RGMII1 \n
 *        CPSW_RGMII1_SPEED - Speed of RGMII1 \n
 *        CPSW_RGMII1_LINK_STAT - Link Status of RGMII1 \n
 *
 *    The returned value can be compared agains the below values \n
 *        CPSW_RGMII2_DUPLEX_FULL - RGMII2 full duplex \n
 *        CPSW_RGMII2_DUPLEX_HALF - RGMII2 half duplex \n
 *        CPSW_RGMII2_SPEED_10M - Speed is 10 Mbps \n
 *        CPSW_RGMII2_SPEED_100M - Speed is 100 Mbps \n
 *        CPSW_RGMII2_SPEED_1000M - Speed is 1000 Mbps \n
 *        CPSW_RGMII2_LINK_UP - RGMII2 link is up\n
 *        CPSW_RGMII2_LINK_DOWN - RGMII2 link is down \n
 *        CPSW_RGMII1_DUPLEX_FULL - RGMII1 full duplex \n
 *        CPSW_RGMII1_DUPLEX_HALF - RGMII1 half duplex \n
 *        CPSW_RGMII1_SPEED_10M - Speed is 10 Mbps \n
 *        CPSW_RGMII1_SPEED_100M - Speed is 100 Mbps \n
 *        CPSW_RGMII1_SPEED_1000M - Speed is 1000 Mbps \n
 *        CPSW_RGMII1_LINK_UP - RGMII1 link is up\n
 *        CPSW_RGMII1_LINK_DOWN - RGMII1 link is down \n
 *
 * \return  Status of RGMII. Return value can be compared agains the same
 *          statFlag passed.
 **/
uint32_t CPSWWrRGMIIStatusGet(uint32_t baseAddr, uint32_t statFlag)
{
    return (reg32r(baseAddr, CPSW_WR_RGMII_CTL) & statFlag);
}

/**
 * \brief   Initializes the ALE. The ALE logic is reset and the ALE table
 *          entries are cleared.
 *
 * \param   baseAddr    Base address of the ALE module
 *
 * \return  None
 **/
void CPSWALEInit(uint32_t baseAddr)
{
    reg32w(baseAddr, CPSW_ALE_CONTROL, (CPSW_ALE_CONTROL_CLEAR_TABLE | CPSW_ALE_CONTROL_ENABLE_ALE));
}

/**
 * \brief   Age Out Address Table. The Untouched ageable ALE table
 *          entries are cleared.
 *
 * \param   baseAddr    Base address of the ALE module
 *
 * \return  None
 **/
void CPSWALEAgeOut(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_AGE_OUT_NOW);

    wait(CPSW_ALE_CONTROL_AGE_OUT_NOW & (reg32r(baseAddr, CPSW_ALE_CONTROL)));
}

/**
 * \brief   Sets the Broadcast Packet Rate Limit
 *
 * \param   baseAddr      Base Address of the ALE module
 * \param   bplVal        The Broadcast Packet Rate Limit Value
 *
 * \return  None
 *
 **/
void CPSWALEBroadcastRateLimitSet(uint32_t baseAddr, uint32_t portNum, uint32_t bplVal)
{
    reg32an(baseAddr, CPSW_ALE_PORTCTL(portNum), CPSW_ALE_PORTCTL0_BCAST_LIMIT);
    reg32m(baseAddr, CPSW_ALE_PORTCTL(portNum), (bplVal << CPSW_ALE_PORTCTL0_BCAST_LIMIT_SHIFT));
}

/**
 * \brief   Sets the Multicast Packet Rate Limit
 *
 * \param   baseAddr      Base Address of the ALE module
 * \param   mplVal        The Multicast Packet Rate Limit Value
 *
 * \return  None
 *
 **/
void CPSWALEMulticastRateLimitSet(uint32_t baseAddr, uint32_t portNum, uint32_t mplVal)
{
    reg32an(baseAddr, CPSW_ALE_PORTCTL(portNum), CPSW_ALE_PORTCTL0_MCAST_LIMIT);
    reg32m(baseAddr, CPSW_ALE_PORTCTL(portNum), (mplVal << CPSW_ALE_PORTCTL0_MCAST_LIMIT_SHIFT));
}

/**
 * \brief   VLAN ID Ingress Check
 *
 * \param   baseAddr      Base Address of the ALE module
 *
 * \return  None
 *
 **/
void CPSWALEVIDIngressCheckSet(uint32_t baseAddr, uint32_t portNum)
{
    reg32m(baseAddr, CPSW_ALE_PORTCTL(portNum), CPSW_ALE_PORTCTL0_MCAST_LIMIT);
}

/**
 * \brief   Sets the port state in the ALE for a given port
 *
 * \param   baseAddr    Base address of the ALE module
 * \param   portNum     The port number
 * \param   portState   The port state to be set
 *    'portState' can take one of the below values \n
 *        CPSW_ALE_PORT_STATE_FWD - ALE state is Forward \n
 *        CPSW_ALE_PORT_STATE_LEARN - ALE state is Learn \n
 *        CPSW_ALE_PORT_STATE_BLOCKED - ALE state is Blocked \n
 *        CPSW_ALE_PORT_STATE_DISABLED - ALE state is Disabled
 *
 * \return  None
 **/
void CPSWALEPortStateSet(uint32_t baseAddr, uint32_t portNum, uint32_t portState)
{
    reg32an(baseAddr, CPSW_ALE_PORTCTL(portNum), CPSW_ALE_PORTCTL0_PORT_STATE);

    reg32m(baseAddr, CPSW_ALE_PORTCTL(portNum), portState);
}

/**
 * \brief   Sets VLAN Aware mode for ALE to Flood if VLAN not found
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALEVLANAwareSet(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ALE_VLAN_AWARE);
}

/**
 * \brief   Clears VLAN Aware mode for ALE to drop packets
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALEVLANAwareClear(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ALE_VLAN_AWARE);
}

/**
 * \brief   Configure Rate Limit to TX Mode
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALERateLimitTXMode(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_RATE_LIMIT_TX);
}

/**
 * \brief   Configure Rate Limit to RX Mode
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALERateLimitRXMode(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_RATE_LIMIT_TX);
}

/**
 * \brief   Enable Rate Limit for ALE
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALERateLimitEnable(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ENABLE_RATE_LIMIT);
}

/**
 * \brief   Disable Rate Limit for ALE
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALERateLimitDisable(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ENABLE_RATE_LIMIT);
}

/**
 * \brief   Enable MAC Authorization Mode for ALE
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALEAUTHModeSet(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ENABLE_AUTH_MODE);
}

/**
 * \brief   Disable MAC Authorization Mode for ALE
 *
 * \param   baseAddr    Base address of the ALE Module
 *
 * \return  None
 **/
void CPSWALEAUTHModeClear(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ENABLE_AUTH_MODE);
}

/**
 * \brief   Sets an ALE table entry
 *
 * \param   baseAddr    Base address of the ALE Module
 * \param   aleTblIdx   The Index of the table entry
 * \param   aleEntryPtr The address of the entry to be set
 *
 * \return  None
 **/
void CPSWALETableEntrySet(uint32_t baseAddr, uint32_t aleTblIdx, uint32_t *aleEntryPtr)
{
    uint32_t cnt;

    for (cnt = 0; cnt < ALE_ENTRY_WORDS; cnt++)
    {
        reg32w(baseAddr, CPSW_ALE_TBLW(cnt), *(aleEntryPtr + cnt));
    }

    reg32w(baseAddr,  CPSW_ALE_TBLCTL, aleTblIdx | CPSW_ALE_TBLCTL_WRITE_RDZ);
}

/**
 * \brief   Returns an ALE table entry
 *
 * \param   baseAddr    Base address of the ALE Module
 * \param   aleTblIdx   The Index of the table entry
 * \param   aleEntryPtr The address where the ALE entry to be written
 *
 * \return  None
 **/
void CPSWALETableEntryGet(uint32_t baseAddr, uint32_t aleTblIdx, uint32_t *aleEntryPtr)
{
    uint32_t cnt;

    reg32w(baseAddr, CPSW_ALE_TBLCTL, aleTblIdx);

    for (cnt = 0; cnt < ALE_ENTRY_WORDS; cnt++)
    {
        *(aleEntryPtr + cnt) = reg32r(baseAddr, CPSW_ALE_TBLW(cnt));
    }
}

/**
 * \brief   Returns the prescale value for ALE. The input clock is divided
 *          by this value to use in the broadcast/multicast rate limiters.
 *
 * \param   baseAddr      Base Address of the ALE module
 *
 * \return  Prescale value
 *
 **/
uint32_t CPSWALEPrescaleGet(uint32_t baseAddr)
{
    return (reg32r(baseAddr, CPSW_ALE_PRESCALE) & CPSW_ALE_PRESCALE_ALE_PRESCALE);
}

/**
 * \brief   Sets the prescale value for ALE. The input clock is divided
 *          by this value to use in the broadcast/multicast rate limiters.
 *
 * \param   baseAddr      Base Address of the ALE module
 * \param   psVal         The prescale value
 *
 * \return  None
 *
 **/
void CPSWALEPrescaleSet(uint32_t baseAddr, uint32_t psVal)
{
    reg32m(baseAddr, CPSW_ALE_PRESCALE, psVal & CPSW_ALE_PRESCALE_ALE_PRESCALE);
}

/**
 * \brief   Sets the Unknown VLAN Force Untagged Egress
 *
 * \param   baseAddr      Base Address of the ALE module
 * \param   ueVal         The Unknown VLAN Fornce Untagged Egress value
 *
 * \return  None
 *
 **/
void CPSWALEUnknownUntaggedEgressSet(uint32_t baseAddr, uint32_t ueVal)
{
    reg32an(baseAddr, CPSW_ALE_UNKNOWN_VLAN, CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_FORCE_UNTA);
    reg32m(baseAddr, CPSW_ALE_UNKNOWN_VLAN, (ueVal << CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_FORCE_UNTA_SHIFT));
}

/**
 * \brief   Sets the Unknown VLAN Registered Multicast Flood Mask
 *
 * \param   baseAddr      Base Address of the ALE module
 * \param   rfmVal        Unknown VLAN Registered Multicast Flood Mask Value
 *
 * \return  None
 *
 **/
void CPSWALEUnknownRegFloodMaskSet(uint32_t baseAddr, uint32_t rfmVal)
{
    reg32an(baseAddr, CPSW_ALE_UNKNOWN_VLAN, CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_MCAST_FLO);
    reg32m(baseAddr, CPSW_ALE_UNKNOWN_VLAN, rfmVal << CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_MCAST_FLO_SHIFT);
}

/**
 * \brief   Sets the Unknown VLAN UnRegistered Multicast Flood Mask
 *
 * \param   baseAddr      Base Address of the ALE module
 * \param   ufmVal        Unknown VLAN UnRegistered Multicast Flood Mask Value
 *
 * \return  None
 *
 **/
void CPSWALEUnknownUnRegFloodMaskSet(uint32_t baseAddr, uint32_t ufmVal)
{
    reg32an(baseAddr, CPSW_ALE_UNKNOWN_VLAN, CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_REG_MCAST);
    reg32m(baseAddr, CPSW_ALE_UNKNOWN_VLAN, (ufmVal << CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_REG_MCAST_SHIFT));
}

/**
 * \brief   Sets the Unknown VLAN Member List
 *
 * \param   baseAddr      Base Address of the ALE module
 * \param   mlVal        Unknown VLAN UnRegistered Multicast Flood Mask Value
 *
 * \return  None
 *
 **/
void CPSWALEUnknownMemberListSet(uint32_t baseAddr, uint32_t mlVal)
{
    reg32an(baseAddr, CPSW_ALE_UNKNOWN_VLAN, CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_VLAN_MEM);
    reg32m(baseAddr, CPSW_ALE_UNKNOWN_VLAN, (mlVal << CPSW_ALE_UNKNOWN_VLAN_UNKNOWN_VLAN_MEM_SHIFT));
}

/**
 * \brief   Enables the bypassing of the ALE logic
 *
 * \param   baseAddr      Base Address of the ALE module
 *
 * \return  None
 *
 **/
void CPSWALEBypassEnable(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ALE_BYPASS);
}

/**
 * \brief   Disables the bypassing of the ALE logic
 *
 * \param   baseAddr      Base Address of the ALE module
 *
 * \return  None
 *
 **/
void CPSWALEBypassDisable(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_ALE_CONTROL, CPSW_ALE_CONTROL_ALE_BYPASS);
}

/**
 * \brief   Enables the receive flow control for CPSW for a given port
 *
 * \param   baseAddr      Base Address of the CPSW subsystem
 * \param   portNum       The port number
 *
 * \return  None
 *
 **/
void CPSWRxFlowControlEnable(uint32_t baseAddr, uint32_t portNum)
{
    reg32m(baseAddr, CPSW_SS_FLOW_CONTROL, (1 << portNum));
}

/**
 * \brief   Disables the receive flow control for CPSW for a given port
 *
 * \param   baseAddr      Base Address of the CPSW subsystem
 * \param   portNum       The port number
 *
 * \return  None
 *
 **/
void CPSWRxFlowControlDisable(uint32_t baseAddr, uint32_t portNum)
{
    reg32an(baseAddr, CPSW_SS_FLOW_CONTROL, (1 << portNum));
}

/**
 * \brief   Enables the software idle mode, causing the switch fabric to stop
 *          forward packets at the next start of packet.
 *
 * \param   baseAddr      Base Address of the CPSW subsystem
 *
 * \return  None
 *
 **/
void CPSWSoftwareIdleEnable(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_SS_SOFT_IDLE, CPSW_SS_SOFT_IDLE_SOFT_IDLE);
}

/**
 * \brief   Disables the software idle mode, causing the switch fabric to
 *          forward packets at the next start of packet.
 *
 * \param   baseAddr      Base Address of the CPSW subsystem
 *
 * \return  None
 *
 **/
void CPSWSoftwareIdleDisable(uint32_t baseAddr, uint32_t portNum)
{
    reg32an(baseAddr, CPSW_SS_SOFT_IDLE, CPSW_SS_SOFT_IDLE_SOFT_IDLE);
}

/**
 * \brief   Enables the CPSW statistics for the given port
 *
 * \param   baseAddr      Base Address of the CPSW subsystem
 * \param   portNum       The port number
 *
 * \return  None
 *
 **/
void CPSWStatisticsEnable(uint32_t baseAddr)
{
    reg32w(baseAddr, CPSW_SS_STAT_PORT_EN, (CPSW_SS_STAT_PORT_EN_P0_STAT_EN
                                             | CPSW_SS_STAT_PORT_EN_P1_STAT_EN
                                             | CPSW_SS_STAT_PORT_EN_P2_STAT_EN));
}

/**
 * \brief   Enables the VLAN aware mode for CPSW
 *
 * \param   baseAddr      Base Address of the CPSW subsystem
 *
 * \return  None
 *
 **/
void CPSWVLANAwareEnable(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_SS_CONTROL, CPSW_SS_CONTROL_VLAN_AWARE);
}

/**
 * \brief   Disables the VLAN aware mode for CPSW
 *
 * \param   baseAddr      Base Address of the CPSW subsystem
 *
 * \return  None
 *
 **/
void CPSWVLANAwareDisable(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_SS_CONTROL, CPSW_SS_CONTROL_VLAN_AWARE);
}

/**
 * \brief   Sets the ethernet address at the CPSW port
 *
 * \param   baseAddr      Base address of the CPSW Port Module registers
 * \param   ethAddr       Start address of the 6 byte ethernet address
 *
 * \return  None
 *
 **/
void CPSWPortSrcAddrSet(uint32_t baseAddr, uint8_t *ethAddr)
{

    reg32w(baseAddr, CPSW_PORT_SA_HI, ethAddr[0]
                   | (ethAddr[1] << CPSW_PORT_P1_SA_HI_MACSRCADDR_39_32_SHIFT)
                   | (ethAddr[2] << CPSW_PORT_P1_SA_HI_MACSRCADDR_31_24_SHIFT)
                   | (ethAddr[3] << CPSW_PORT_P1_SA_HI_MACSRCADDR_23_16_SHIFT));
    reg32w(baseAddr, CPSW_PORT_SA_LO, ethAddr[4] | (ethAddr[5] << CPSW_PORT_P1_SA_LO_MACSRCADDR_7_0_SHIFT));
}

/**
 * \brief   Sets Dual Mac Mode for CPSW Port0
 *
 * \param   baseAddr      Base address of the CPSW Host Port Module registers
 *
 * \return  None
 *
 **/
void CPSWHostPortDualMacModeSet(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_PORT_TX_IN_CTL, CPSW_PORT_P0_TX_IN_CTL_TX_IN_SEL);
    reg32m(baseAddr, CPSW_PORT_TX_IN_CTL, CPSW_PORT_P0_TX_IN_CTL_TX_IN_DUAL_MAC);
}

/**
 * \brief   Configures Port VLAN
 *
 * \param   baseAddr      Base address of the CPSW Port Module registers
 * \param   vlanId        VLAN ID to be set
 * \param   cfiBit        CFI value to be set
 * \param   vlanPri       Port VLAN priority
 *            'vlanId' can take a value from 0 to 0xFFF \n
 *            'cfiBit' can be either 0 or 1. \n
 *            'vlanPri' can be any value between and including 0 and 7.
 *
 * \return  None
 *
 **/
void CPSWPortVLANConfig(uint32_t baseAddr, uint32_t vlanId, uint32_t cfiBit, uint32_t vlanPri)
{
    reg32w(baseAddr, CPSW_PORT_PORT_VLAN, vlanId
                           | (cfiBit << CPSW_PORT_P2_PORT_VLAN_PORT_CFI_SHIFT)
                           | (vlanPri << CPSW_PORT_P2_PORT_VLAN_PORT_PRI_SHIFT));
}

/**
 * \brief   Returns the requested CPSW Statistics
 *
 * \param   baseAddr      Base address of the CPSW Status Module registers.
 * \param   statReg       Statistics Register to be read
 *
 * \return  The requested statistics
 *
 **/
uint32_t CPSWStatisticsGet(uint32_t baseAddr, uint32_t statReg)
{
    return (reg32r(baseAddr, statReg));
}

/**
 * \brief   Resets the CPDMA
 *
 * \param   baseAddr      Base address of the CPDMA Module registers.
 *
 * \return  None
 *
 **/
void CPSWCPDMAReset(uint32_t baseAddr)
{
    uint32_t cnt;

    /* Reset the CPDMA */
    reg32w(baseAddr, CPSW_CPDMA_CPDMA_SOFT_RESET, CPSW_CPDMA_CPDMA_SOFT_RESET_SOFT_RESET);

    /* Wait till the reset completes */
    wait(reg32r(baseAddr, CPSW_CPDMA_CPDMA_SOFT_RESET) & CPSW_CPDMA_CPDMA_SOFT_RESET_SOFT_RESET);

    /* Initialize all the header descriptor pointer registers */
    for(cnt =  0; cnt< CPSW_MAX_HEADER_DESC; cnt++)
    {
        reg32w(baseAddr, CPSW_CPDMA_TX_HDP(cnt), 0);
        reg32w(baseAddr, CPSW_CPDMA_RX_HDP(cnt), 0);
        reg32w(baseAddr, CPSW_CPDMA_TX_CP(cnt), 0);
        reg32w(baseAddr, CPSW_CPDMA_RX_CP(cnt), 0);
    }
}

/**
 * \brief   Enables the TXPULSE Interrupt Generation.
 *
 * \param   baseAddr      Base address of the CPDMA Module registers.
 * \param   channel       Channel number for which interrupt to be enabled
 *
 * \return  None
 *
 **/
void CPSWCPDMATxIntEnable(uint32_t baseAddr, uint32_t channel)
{
    reg32m(baseAddr, CPSW_CPDMA_TX_INTMASK_SET, (1 << channel));
}

/**
 * \brief   Enables the RXPULSE Interrupt Generation.
 *
 * \param   baseAddr      Base address of the CPDMA Module registers.
 * \param   channel       Channel number for which interrupt to be enabled
 *
 * \return  None
 *
 **/
void CPSWCPDMARxIntEnable(uint32_t baseAddr, uint32_t channel)
{
    reg32m(baseAddr, CPSW_CPDMA_RX_INTMASK_SET, (1 << channel));
}

/**
 * \brief   Disables the TXPULSE Interrupt Generation.
 *
 * \param   baseAddr      Base address of the CPDMA Module registers.
 * \param   channel       Channel number for which interrupt to be disabled
 *
 * \return  None
 *
 **/
void CPSWCPDMATxIntDisable(uint32_t baseAddr, uint32_t channel)
{
    reg32m(baseAddr, CPSW_CPDMA_TX_INTMASK_CLEAR, (1 << channel));

}

/**
 * \brief   Disables the RXPULSE Interrupt Generation.
 *
 * \param   baseAddr      Base address of the CPDMA Module registers.
 * \param   channel       Channel number for which interrupt to be disabled
 *
 * \return  None
 *
 **/
void CPSWCPDMARxIntDisable(uint32_t baseAddr, uint32_t channel)
{
    reg32m(baseAddr, CPSW_CPDMA_RX_INTMASK_CLEAR, (1 << channel));

}

/**
 * \brief   API to enable the transmit in the TX Control Register.
 *          After the transmit is enabled, any write to TXHDP of
 *          a channel will start transmission
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 *
 * \return  None
 *
 **/
void CPSWCPDMATxEnable(uint32_t baseAddr)
{
    reg32w(baseAddr, CPSW_CPDMA_TX_CONTROL, CPSW_CPDMA_TX_CONTROL_TX_EN);
}

/**
 * \brief   API to enable the receive in the RX Control Register.
 *          After the receive is enabled, and write to RXHDP of
 *          a channel, the data can be received in the destination
 *          specified by the corresponding RX buffer descriptor.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 *
 * \return  None
 *
 **/
void CPSWCPDMARxEnable(uint32_t baseAddr)
{
    reg32w(baseAddr, CPSW_CPDMA_RX_CONTROL, CPSW_CPDMA_RX_CONTROL_RX_EN);
}

/**
 * \brief   API to write the TX HDP register. If transmit is enabled,
 *          write to the TX HDP will immediately start transmission.
 *          The data will be taken from the buffer pointer of the TX buffer
 *          descriptor written to the TX HDP
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 * \param   descHdr       Address of the TX buffer descriptor
 * \param   channel       Channel Number
 *
 * \return  None
 *
 **/
void CPSWCPDMATxHdrDescPtrWrite(uint32_t baseAddr, uint32_t descHdr, uint32_t channel)
{
    reg32w(baseAddr, CPSW_CPDMA_TX_HDP(channel), descHdr);
}

/**
 * \brief   API to write the RX HDP register. If receive is enabled,
 *          write to the RX HDP will enable data reception to point to
 *          the corresponding RX buffer descriptor's buffer pointer.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 * \param   descHdr       Address of the RX buffer descriptor
 * \param   channel       Channel Number
 *
 * \return  None
 *
 **/
void CPSWCPDMARxHdrDescPtrWrite(uint32_t baseAddr, uint32_t descHdr, uint32_t channel)
{
    reg32w(baseAddr, CPSW_CPDMA_RX_HDP(channel), descHdr);
}

/**
 * \brief   Writes the DMA End Of Interrupt Vector.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 * \param   eoiFlag       Type of interrupt to acknowledge to the CPDMA
 *            'eoiFlag' can take the following values \n
 *                CPSW_EOI_TX_PULSE - TX Pulse Interrupt \n
 *                CPSW_EOI_RX_PULSE - RX Pulse Interrupt \n
 *                CPSW_EOI_RX_THRESH_PULSE - RX Pulse Threshold Interrupt \n
 *                CPSW_EOI_MISC_PULSE - Misc Interrupt \n
 *
 * \return  None
 *
 **/
void CPSWCPDMAEndOfIntVectorWrite(uint32_t baseAddr, uint32_t eoiFlag)
{
    /* Acknowledge the CPDMA */
    reg32w(baseAddr, CPSW_CPDMA_CPDMA_EOI_VECTOR, eoiFlag);
}

/**
 * \brief   Writes the the TX Completion Pointer for a specific channel
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 * \param   channel       Channel Number.
 * \param   comPtr        Completion Pointer Value to be written
 *
 * \return  None
 *
 **/
void CPSWCPDMATxCPWrite(uint32_t baseAddr, uint32_t channel, uint32_t comPtr)
{
    reg32w(baseAddr, CPSW_CPDMA_TX_CP(channel), comPtr);
}

/**
 * \brief   Writes the the RX Completion Pointer for a specific channel
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 * \param   channel       Channel Number.
 * \param   comPtr        Completion Pointer Value to be written
 *
 * \return  None
 *
 **/
void CPSWCPDMARxCPWrite(uint32_t baseAddr, uint32_t channel, uint32_t comPtr)
{
    reg32w(baseAddr, CPSW_CPDMA_RX_CP(channel), comPtr);
}

/**
 * \brief   Set the free buffers for a specific channel
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 * \param   channel       Channel Number.
 * \param   nBuf          Number of free buffers
 *
 * \return  None
 *
 **/
void CPSWCPDMANumFreeBufSet(uint32_t baseAddr, uint32_t channel, uint32_t nBuf)
{
    reg32w(baseAddr, CPSW_CPDMA_RX_FREEBUFFER(channel), nBuf);
}

/**
 * \brief   Returns the CPDMA Status.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers.
 * \param   statFlag      The status flags to be read
 *     'statFlag' can take one of the following values \n
 *            CPDMA_STAT_IDLE - to check if CPDMA is idle. \n
 *            CPDMA_STAT_TX_HOST_ERR_CODE - TX host error code. \n
 *            CPDMA_STAT_TX_HOST_ERR_CHAN - TX host error channel. \n
 *            CPDMA_STAT_RX_HOST_ERR_CODE - RX host error code. \n
 *            CPDMA_STAT_RX_HOST_ERR_CHAN - RX host error channel. \n
 *
 * \return  the DMA status for the status flag passed.
 *          The return values for CPDMA_STAT_IDLE are, \n
 *            CPDMA_STAT_IDLE - CPDMA is in idle state \n
 *            CPDMA_STAT_NOT_IDLE - CPDMA is not in idle state \n
 *
 *          The return values for CPDMA_STAT_TX_HOST_ERR_CODE are, \n
 *            CPDMA_STAT_TX_NO_ERR - No error \n
 *            CPDMA_STAT_TX_SOP_ERR - SOP error \n
 *            CPDMA_STAT_TX_OWN_ERR - Ownership bit not
 *                                                  set in SOP buffer \n
 *            CPDMA_STAT_TX_ZERO_DESC - Zero Next Buffer
 *                                       Descriptor Pointer Without EOP \n
 *            CPDMA_STAT_TX_ZERO_BUF_PTR - Zero Buffer Pointer \n
 *            CPDMA_STAT_TX_ZERO_BUF_LEN - Zero Buffer Length \n
 *            CPDMA_STAT_TX_PKT_LEN_ERR - Packet Length Error \n
 *
 *          The return values for CPDMA_STAT_RX_HOST_ERR_CODE are, \n
 *            CPDMA_STAT_RXi_NO_ERR - No error \n
 *            CPDMA_STAT_RX_OWN_NOT_SET - Ownership bit not set in
                                          input buffer \n
 *            CPDMA_STAT_RX_ZERO_BUF_PTR - Zero Buffer Pointer\n
 *            CPDMA_STAT_RX_ZERO_BUF_LEN - Zero Buffer Length on
 *                                       non-SOP descriptor \n
 *            CPDMA_STAT_RX_SOP_BUF_LEN_ERR - SOP buffer length not
 *                                       greater than offset\n
 *
 **/
uint32_t CPSWCPDMAStatusGet(uint32_t baseAddr, uint32_t statFlag)
{
    return (((reg32r(baseAddr, CPSW_CPDMA_DMASTATUS)) & statFlag) >> (statFlag & CPDMA_ERR_CHANNEL_POS));
}

/**
 * \brief   Configures the CPDMA module by writing the configuration value
 *          to the DMA control register.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 * \param   cfg           CPDMA configuration written to control register
 *     'cfg' shall be CPDMA_CFG(tx_rlim, rx_cef, cmd_idle,
 *                              rx_offlen_blk, rx_own, tx_ptype). \n
 *        The parameter 'tx_rlim' to CPDMA_CFG can take one of the below
 *        values, showing which all channels are rate-limited. \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7 \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7_TO_6 \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7_TO_5 \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7_TO_4 \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7_TO_3 \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7_TO_2 \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7_TO_1 \n
 *            CPDMA_CFG_TX_RATE_LIM_CH_7_TO_0 \n
 *        The parameter 'rx_cef' to CPDMA_CFG can take one of the below
 *        values \n
 *            CPDMA_CFG_COPY_ERR_FRAMES - To copy error frames to memory \n
 *            CPDMA_CFG_NO_COPY_ERR_FRAMES - Not to copy error frames \n
 *        The parameter 'cmd_idle' to CPDMA_CFG can take one of the below
 *        values \n
 *            CPDMA_CFG_IDLE_COMMAND - Idle commanded \n
 *            CPDMA_CFG_IDLE_COMMAND_NONE - Idle not commanded \n
 *        The parameter 'rx_offlen_blk' to CPDMA_CFG can take one of the below
 *        values \n
 *            CPDMA_CFG_BLOCK_RX_OFF_LEN_WRITE - Block the DMA writes to the
 *                                               offset/length field during
 *                                               packet processing. \n
 *            CPDMA_CFG_NOT_BLOCK_RX_OFF_LEN_WRITE - Do not Block the DMA writes
 *                                              to the offset/length field during
 *                                              packet processing. \n
 *        The parameter 'rx_own' to CPDMA_CFG can take one of the below
 *        values \n
 *            CPDMA_CFG_RX_OWN_1 - The CPDMA writes 1 to the ownership bit at
 *                                 the end of packet processing. \n
 *            CPDMA_CFG_RX_OWN_0 - The CPDMA writes 0 to the ownership bit at
 *                                 the end of packet processing. \n
 *        The parameter 'tx_ptype' to CPDMA_CFG can take one of the below
 *        values \n
 *            CPDMA_CFG_TX_PRI_ROUND_ROBIN - The next channel for transmit is
 *                                           chosen round-robin. \n
 *            CPDMA_CFG_TX_PRI_FIXED - The next channel for transmit is
 *                                     chosen priority based, channel 7 with the
 *                                     highest priority \n
 *
 * \return  None
 *
 **/
void CPSWCPDMAConfig(uint32_t baseAddr, uint32_t cfg)
{
    reg32w(baseAddr, CPSW_CPDMA_DMACONTROL, cfg);
}

/**
 * \brief   Enable the command idle mode for CPDMA. When this API is called, the
 *          CPSW stops all the reception and transmission. However, if receiving
 *          the current frame will be received completely before going to the idle
 *          state. Also, while transmitting, the contents in the fifo will be sent
 *          fully.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 *
 * \return  None
 *
 **/
void CPSWCPDMACmdIdleEnable(uint32_t baseAddr)
{
    reg32m(baseAddr, CPSW_CPDMA_DMACONTROL, CPSW_CPDMA_DMACONTROL_CMD_IDLE);

    /* Wait till the state changes to idle */
    wait((reg32r(baseAddr, CPSW_CPDMA_DMASTATUS) & CPSW_CPDMA_DMASTATUS_IDLE) != CPSW_CPDMA_DMASTATUS_IDLE);
}

/**
 * \brief   Disable the command idle mode for CPDMA.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 *
 * \return  None
 *
 **/
void CPSWCPDMACmdIdleDisable(uint32_t baseAddr)
{
    reg32an(baseAddr, CPSW_CPDMA_DMACONTROL, CPSW_CPDMA_DMACONTROL_CMD_IDLE);
}

/**
 * \brief   Sets the RX buffer offset value. The RX buffer offset will be
 *          written by the port into each frame SOP buffer descriptor
 *          buffer_offset field. The frame data will begin after the
 *          rx_buffer_offset value of bytes. This value will be used for
 *          all the channels .
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 * \param   bufOff        Buffer offset value
 *
 * \return  None
 *
 **/
void CPSWCPDMARxBufOffsetSet(uint32_t baseAddr, uint32_t bufOff)
{
    reg32w(baseAddr, CPSW_CPDMA_RX_BUFFER_OFFSET, bufOff);
}

/**
 * \brief   Returns the raw transmit interrupt pending status.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 * \param   chanMask      Channel Mask
 *    'chanMask' can be given for one or more channels. \n
 *         0x01- for 0th channel, 0x80 for 7th channel, 0x81 for both 0th
 *         and 7th channel etc. \n
 *
 * \return  Raw receive interrupt status \n
 *          bits for the 'chanMask' will be set if interrupt is pending \n
 *          bits for the 'chanMask' will be clear if interrupt is not
 *                                                              pending \n
 *
 **/
uint32_t CPSWCPDMATxIntStatRawGet(uint32_t baseAddr, uint32_t chanMask)
{
    return (reg32r(baseAddr, CPSW_CPDMA_TX_INTSTAT_RAW) & chanMask);
}

/**
 * \brief   Returns the masked transmit interrupt pending status.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 * \param   chanMask      Channel Mask
 *    'chanMask' can be given for one or more channels. \n
 *         0x01- for 0th channel, 0x80 for 7th channel, 0x81 for both 0th
 *         and 7th channel etc. \n
 *
 * \return  Masked transmit interrupt status \n
 *          bits for the 'chanMask' will be set if interrupt is pending \n
 *          bits for the 'chanMask' will be cleared if interrupt is not
 *                                                              pending \n
 *
 **/
uint32_t CPSWCPDMATxIntStatMaskedGet(uint32_t baseAddr, uint32_t chanMask)
{
    return (reg32r(baseAddr, CPSW_CPDMA_TX_INTSTAT_MASKED) & chanMask);
}

/**
 * \brief   Returns the raw receive interrupt pending status.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 * \param   chanMask      Channel Mask
 * \param   intType       Interrupt type
 *    'chanMask' can be given for one or more channels. \n
 *         0x01- for 0th channel, 0x80 for 7th channel, 0x81 for both 0th
 *         and 7th channel etc. \n
 *    'intType' can take one of the following values. \n
 *         CPDMA_RX_INT_THRESH_PEND - RX threshold interrupt pending \n
 *         CPDMA_RX_INT_PULSE_PEND - RX pulse interrupt pending \n
 *
 * \return  Raw receive interrupt status \n
 *          bits for the 'chanMask' will be set if interrupt is pending \n
 *          bits for the 'chanMask' will be cleared if interrupt is not
 *                                                              pending \n
 *
 **/
uint32_t CPSWCPDMARxIntStatRawGet(uint32_t baseAddr, uint32_t chanMask, uint32_t intType)
{
    return ((reg32r(baseAddr, CPSW_CPDMA_RX_INTSTAT_RAW) >> intType) & chanMask);
}

/**
 * \brief   Returns the masked receive interrupt pending status.
 *
 * \param   baseAddr      Base Address of the CPDMA module registers
 * \param   chanMask      Channel Mask
 * \param   intType       Interrupt type
 *    'chanMask' can be given for one or more channels. \n
 *         0x01- for 0th channel, 0x80 for 7th channel, 0x81 for both 0th
 *         and 7th channel etc. \n
 *    'intType' can take one of the following values. \n
 *         CPDMA_RX_INT_THRESH_PEND - RX threshold interrupt pending \n
 *         CPDMA_RX_INT_PULSE_PEND - RX pulse interrupt pending \n
 *
 * \return  Masked receive interrupt status \n
 *          bits for the 'chanMask' will be set if interrupt is pending \n
 *          bits for the 'chanMask' will be cleared if interrupt is not
 *                                                              pending \n
 *
 **/
uint32_t CPSWCPDMARxIntStatMaskedGet(uint32_t baseAddr, uint32_t chanMask, uint32_t intType)
{
    return ((reg32r(baseAddr, CPSW_CPDMA_RX_INTSTAT_MASKED) >> intType) & chanMask);
}

/**
 * \brief   Saves the CPSW register context. This can be used while going
 *          to power down mode where CPSW power will be cut down.
 *
 * \param   contextPtr   Pointer to the structure where CPSW register context
 *                       need to be saved.
 *
 * \return  None
 *
 **/
void CPSWContextSave(CPSWCONTEXT *contextPtr)
{
    uint32_t idx;
    uint32_t *cppiDest = (uint32_t*)contextPtr->cppiRamBase;

    CPSWCPDMACmdIdleEnable(contextPtr->cpdmaBase);

    /* Restore the CPPI RAM contents */
    for(idx = 0; idx < (CPSW_SIZE_CPPI_RAM / 4); idx++, cppiDest++)
    {
        contextPtr->cppiRam[idx] = *cppiDest;
    }

    contextPtr->aleCtrl = reg32r(contextPtr->aleBase, CPSW_ALE_CONTROL);
    contextPtr->alePortCtl[0] = reg32r(contextPtr->aleBase, CPSW_ALE_PORTCTL(0));
    contextPtr->alePortCtl[1] = reg32r(contextPtr->aleBase, CPSW_ALE_PORTCTL(1));
    contextPtr->alePortCtl[2] = reg32r(contextPtr->aleBase, CPSW_ALE_PORTCTL(2));

    for(idx = 0; idx < CPSW_MAX_NUM_ALE_ENTRY; idx++)
    {
        CPSWALETableEntryGet(contextPtr->aleBase, idx, &(contextPtr->aleEntry[idx * 3]));
    }

    contextPtr->ssStatPortEn = reg32r(contextPtr->ssBase, CPSW_SS_STAT_PORT_EN);
    contextPtr->port1SaHi = reg32r(contextPtr->port1Base, CPSW_PORT_SA_HI);
    contextPtr->port1SaLo = reg32r(contextPtr->port1Base, CPSW_PORT_SA_LO);
    contextPtr->port2SaHi = reg32r(contextPtr->port2Base, CPSW_PORT_SA_HI);
    contextPtr->port2SaLo = reg32r(contextPtr->port2Base, CPSW_PORT_SA_LO);
    contextPtr->port1TxInCtl = reg32r(contextPtr->port1Base, CPSW_PORT_TX_IN_CTL);
    contextPtr->port2TxInCtl = reg32r(contextPtr->port2Base, CPSW_PORT_TX_IN_CTL);
    contextPtr->port1Vlan = reg32r(contextPtr->port1Base, CPSW_PORT_PORT_VLAN);
    contextPtr->port2Vlan = reg32r(contextPtr->port2Base, CPSW_PORT_PORT_VLAN);
    contextPtr->cpdmaRxFB = reg32r(contextPtr->cpdmaBase, CPSW_CPDMA_RX_FREEBUFFER(0));
    contextPtr->cpdmaTxCtl = reg32r(contextPtr->cpdmaBase, CPSW_CPDMA_TX_CONTROL);
    contextPtr->cpdmaRxCtl = reg32r(contextPtr->cpdmaBase, CPSW_CPDMA_RX_CONTROL);
    contextPtr->cpdmaRxHdp = reg32r(contextPtr->cpdmaBase, CPSW_CPDMA_RX_HDP(0));
    contextPtr->txIntMaskSet = reg32r(contextPtr->cpdmaBase, CPSW_CPDMA_TX_INTMASK_SET);
    contextPtr->wrCoreIntTxPulse = reg32r(contextPtr->wrBase, CPSW_WR_C_RX_THRESH_EN(0) + 0x04);
    contextPtr->rxIntMaskSet = reg32r(contextPtr->cpdmaBase, CPSW_CPDMA_RX_INTMASK_SET);
    contextPtr->wrCoreIntRxPulse = reg32r(contextPtr->wrBase, CPSW_WR_C_RX_THRESH_EN(0) + 0x08);
    contextPtr->sl1MacCtl = reg32r(contextPtr->sl1Base, CPSW_SL_MACCONTROL);
    contextPtr->sl2MacCtl = reg32r(contextPtr->sl2Base, CPSW_SL_MACCONTROL);
}

/**
 * \brief   Restores the CPSW register context. This can be used while coming
 *          back from power down mode where CPSW power will be cut down.
 *
 * \param   contextPtr   Pointer to the structure where CPSW register context
 *                       need to be restored from.
 *
 * \return  None
 *
 **/
void CPSWContextRestore(CPSWCONTEXT *contextPtr)
{
    uint32_t idx;
    uint32_t *cppiDest = (uint32_t*)contextPtr->cppiRamBase;

    /* Restore the CPPI RAM contents */
    for(idx = 0; idx < (CPSW_SIZE_CPPI_RAM / 4); idx++, cppiDest++)
    {
        *cppiDest = contextPtr->cppiRam[idx] ;
    }

    reg32w(contextPtr->aleBase, CPSW_ALE_CONTROL, contextPtr->aleCtrl);
    reg32w(contextPtr->aleBase, CPSW_ALE_PORTCTL(0), contextPtr->alePortCtl[0]);
    reg32w(contextPtr->aleBase, CPSW_ALE_PORTCTL(1), contextPtr->alePortCtl[1]);
    reg32w(contextPtr->aleBase, CPSW_ALE_PORTCTL(2), contextPtr->alePortCtl[2]);

    for(idx = 0; idx < CPSW_MAX_NUM_ALE_ENTRY; idx++)
    {
        CPSWALETableEntrySet(contextPtr->aleBase, idx, &(contextPtr->aleEntry[idx * 3]));
    }

    reg32w(contextPtr->ssBase, CPSW_SS_STAT_PORT_EN, contextPtr->ssStatPortEn);
    reg32w(contextPtr->port1Base, CPSW_PORT_SA_HI, contextPtr->port1SaHi);
    reg32w(contextPtr->port1Base, CPSW_PORT_SA_LO, contextPtr->port1SaLo);
    reg32w(contextPtr->port2Base, CPSW_PORT_SA_HI, contextPtr->port2SaHi);
    reg32w(contextPtr->port2Base, CPSW_PORT_SA_LO, contextPtr->port2SaLo);
    reg32w(contextPtr->port1Base, CPSW_PORT_TX_IN_CTL, contextPtr->port1TxInCtl);
    reg32w(contextPtr->port2Base, CPSW_PORT_TX_IN_CTL, contextPtr->port2TxInCtl);
    reg32w(contextPtr->port1Base, CPSW_PORT_PORT_VLAN, contextPtr->port1Vlan);
    reg32w(contextPtr->port2Base, CPSW_PORT_PORT_VLAN, contextPtr->port2Vlan);
    reg32w(contextPtr->cpdmaBase, CPSW_CPDMA_RX_FREEBUFFER(0), contextPtr->cpdmaRxFB);
    reg32w(contextPtr->cpdmaBase, CPSW_CPDMA_TX_CONTROL, contextPtr->cpdmaTxCtl);
    reg32w(contextPtr->cpdmaBase, CPSW_CPDMA_RX_CONTROL, contextPtr->cpdmaRxCtl);
    reg32w(contextPtr->cpdmaBase, CPSW_CPDMA_RX_HDP(0), contextPtr->cpdmaRxHdp);
    reg32w(contextPtr->cpdmaBase, CPSW_CPDMA_TX_INTMASK_SET, contextPtr->txIntMaskSet);
    reg32w(contextPtr->wrBase, CPSW_WR_C_RX_THRESH_EN(0) +0x04, contextPtr->wrCoreIntTxPulse);
    reg32w(contextPtr->cpdmaBase, CPSW_CPDMA_RX_INTMASK_SET, contextPtr->rxIntMaskSet);
    reg32w(contextPtr->wrBase, CPSW_WR_C_RX_THRESH_EN(0) + 0x08, contextPtr->wrCoreIntRxPulse);
    reg32w(contextPtr->sl1Base, CPSW_SL_MACCONTROL, contextPtr->sl1MacCtl);
    reg32w(contextPtr->sl2Base, CPSW_SL_MACCONTROL, contextPtr->sl2MacCtl);
}

/**
 * \brief   This function selects the CPSW pins for use in MII mode.
 *
 * \param   None
 *
 * \return  None.
 *
 */
void CPSWPinMuxSetup() {
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXERR,
			CONTROL_CONF_MII1_RXERR_CONF_MII1_RXERR_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXEN, CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXDV,
			CONTROL_CONF_MII1_RXDV_CONF_MII1_RXDV_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD3, CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD2, CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD1, CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXD0, CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_TXCLK,
			CONTROL_CONF_MII1_TXCLK_CONF_MII1_TXCLK_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXCLK,
			CONTROL_CONF_MII1_RXCLK_CONF_MII1_RXCLK_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD3,
			CONTROL_CONF_MII1_RXD3_CONF_MII1_RXD3_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD2,
			CONTROL_CONF_MII1_RXD2_CONF_MII1_RXD2_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD1,
			CONTROL_CONF_MII1_RXD1_CONF_MII1_RXD1_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_RXD0,
			CONTROL_CONF_MII1_RXD0_CONF_MII1_RXD0_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_COL,
			CONTROL_CONF_MII1_COL_CONF_MII1_COL_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MII1_CRS,
			CONTROL_CONF_MII1_CRS_CONF_MII1_CRS_RXACTIVE | CPSW_MII_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MDIO_DATA,
			CONTROL_CONF_MDIO_DATA_CONF_MDIO_DATA_RXACTIVE | CONTROL_CONF_MDIO_DATA_CONF_MDIO_DATA_PUTYPESEL | CPSW_MDIO_SEL_MODE);
	reg32w(SOC_CONTROL_REGS, CONTROL_CONF_MDIO_CLK,
			CONTROL_CONF_MDIO_CLK_CONF_MDIO_CLK_PUTYPESEL | CPSW_MDIO_SEL_MODE);
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

    wait(0 != (reg32r(SOC_PRCM_REGS, CM_PER_CPGMAC0_CLKCTRL) & CM_PER_CPGMAC0_CLKCTRL_IDLEST));

    reg32w(SOC_PRCM_REGS, CM_PER_CPSW_CLKSTCTRL, CM_PER_CPSW_CLKSTCTRL_CLKTRCTRL_SW_WKUP);

    wait(0 == (reg32r(SOC_PRCM_REGS, CM_PER_CPSW_CLKSTCTRL) & CM_PER_CPSW_CLKSTCTRL_CLKACTIVITY_CPSW_125MHZ_GCLK));
}

/**
 * \brief   This function sets the MII mode for both ports
 *
 * \param   None
 *
 * \return  None.
 */
void CPSWEVMPortMIIModeSelect() {
	/* Select MII, Internal Delay mode */
	reg32w(SOC_CONTROL_REGS, CONTROL_GMII_SEL, 0x00);
}

/**
 * \brief   This function returns the MAC address for the EVM
 *
 * \param   addrIdx    the MAC address index.
 * \param   macAddr    the Pointer where the MAC address shall be stored; 'addrIdx' can be either 0 or 1
 *
 * \return  None.
 */
void CPSWEVMMACAddrGet(uint32_t addrIdx, uint8_t *macAddr) {
	macAddr[0] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_LO(addrIdx)) >> 8) & 0xFF;
	macAddr[1] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_LO(addrIdx))) & 0xFF;
	macAddr[2] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx)) >> 24) & 0xFF;
	macAddr[3] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx)) >> 16) & 0xFF;
	macAddr[4] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx)) >> 8) & 0xFF;
	macAddr[5] = (reg32r(SOC_CONTROL_REGS, CONTROL_MAC_ID_HI(addrIdx))) & 0xFF;
}
