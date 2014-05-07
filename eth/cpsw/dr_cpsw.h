/*
 * Driver: dr_cpsw.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 11.04.2014
 * Description:
 *
 */

/**
 *  \file   cpsw.h
 *
 *  \brief  CPSW APIs and macros.
 *
 *   This file contains the driver API prototypes and macro definitions.
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

#ifndef DR_CPSW_H
#define DR_CPSW_H

#include "eth/cpsw/hw_cpsw_ale.h"
#include "eth/cpsw/hw_cpsw_cpdma.h"
#include "eth/cpsw/hw_cpsw_port.h"
#include "eth/cpsw/hw_cpsw_sl.h"
#include "eth/cpsw/hw_cpsw_ss.h"
#include "eth/cpsw/hw_cpsw_wr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/*
** Macros which can be used as 'mode' to pass to the API CPSWSlTransferModeSet
*/
#define CPSW_SLIVER_NON_GIG_FULL_DUPLEX        CPSW_SL_MACCONTROL_FULLDUPLEX
#define CPSW_SLIVER_NON_GIG_HALF_DUPLEX        (0x00u)
#define CPSW_SLIVER_GIG_FULL_DUPLEX            CPSW_SL_MACCONTROL_GIG
#define CPSW_SLIVER_INBAND                     CPSW_SL_MACCONTROL_EXT_EN

/*
** Macros which can be used as 'statFlag' to the API CPSWSlMACStatusGet
*/
#define CPSW_SLIVER_STATE                      CPSW_SL_MACSTATUS_IDLE
#define CPSW_SLIVER_EXT_GIG_INPUT_BIT          CPSW_SL_MACSTATUS_EXT_GIG
#define CPSW_SLIVER_EXT_FULL_DUPLEX_BIT        CPSW_SL_MACSTATUS_EXT_FULLDUPLEX
#define CPSW_SLIVER_RX_FLOWCTRL                CPSW_SL_MACSTATUS_RX_FLOW_ACT
#define CPSW_SLIVER_TX_FLOWCTRL                CPSW_SL_MACSTATUS_TX_FLOW_ACT

/*
** Macros returned by API CPSWSlMACStatusGet
*/
#define CPSW_SLIVER_STATE_IDLE                 CPSW_SL_MACSTATUS_IDLE
#define CPSW_SLIVER_EXT_GIG_INPUT_HIGH         CPSW_SL_MACSTATUS_EXT_GIG
#define CPSW_SLIVER_EXT_FULL_DUPLEX_HIGH       CPSW_SL_MACSTATUS_EXT_FULLDUPLEX
#define CPSW_SLIVER_RX_FLOWCTRL_ACTIVE         CPSW_SL_MACSTATUS_RX_FLOW_ACT
#define CPSW_SLIVER_TX_FLOWCTRL_ACTIVE         CPSW_SL_MACSTATUS_TX_FLOW_ACT

/*
** Macros which can be passed asi 'intFlag' to the API CPSWWrCoreIntEnable
** , CPSWWrCoreIntDisable and CPSWWrCoreIntStatusGet
*/
#define CPSW_CORE_INT_RX_THRESH                (0x00u)
#define CPSW_CORE_INT_RX_PULSE                 (0x04u)
#define CPSW_CORE_INT_TX_PULSE                 (0x08u)
#define CPSW_CORE_INT_MISC                     (0x0Cu)

/*
** Macros which can be passed as 'statFlag' to the API CPSWWrRGMIIStatusGet
*/
#define CPSW_RGMII2_DUPLEX                     CPSW_WR_RGMII_CTL_RGMII2_FULLDUPLEX
#define CPSW_RGMII2_SPEED                      CPSW_WR_RGMII_CTL_RGMII2_SPEED
#define CPSW_RGMII2_LINK_STAT                  CPSW_WR_RGMII_CTL_RGMII2_LINK
#define CPSW_RGMII1_DUPLEX                     CPSW_WR_RGMII_CTL_RGMII1_FULLDUPLEX
#define CPSW_RGMII1_SPEED                      CPSW_WR_RGMII_CTL_RGMII1_SPEED
#define CPSW_RGMII1_LINK_STAT                  CPSW_WR_RGMII_CTL_RGMII1_LINK

/* The values, one of which will be returned by CPSWWrRGMIIStatusGet */
#define CPSW_RGMII2_DUPLEX_FULL                CPSW_WR_RGMII_CTL_RGMII2_FULLDUPLEX
#define CPSW_RGMII2_DUPLEX_HALF                (0x00u)
#define CPSW_RGMII2_SPEED_10M                  (0x00u << CPSW_WR_RGMII_CTL_RGMII2_SPEED_SHIFT)
#define CPSW_RGMII2_SPEED_100M                 (0x01u << CPSW_WR_RGMII_CTL_RGMII2_SPEED_SHIFT)
#define CPSW_RGMII2_SPEED_1000M                (0x02u << CPSW_WR_RGMII_CTL_RGMII2_SPEED_SHIFT)
#define CPSW_RGMII2_LINK_UP                    CPSW_WR_RGMII_CTL_RGMII2_LINK
#define CPSW_RGMII2_LINK_DOWN                  (0x00u)
#define CPSW_RGMII1_DUPLEX_FULL                CPSW_WR_RGMII_CTL_RGMII1_FULLDUPLEX
#define CPSW_RGMII1_DUPLEX_HALF                (0x00u)
#define CPSW_RGMII1_SPEED_10M                  (0x00u << CPSW_WR_RGMII_CTL_RGMII1_SPEED_SHIFT)
#define CPSW_RGMII1_SPEED_100M                 (0x01u << CPSW_WR_RGMII_CTL_RGMII1_SPEED_SHIFT)
#define CPSW_RGMII1_SPEED_1000M                (0x02u << CPSW_WR_RGMII_CTL_RGMII1_SPEED_SHIFT)
#define CPSW_RGMII1_LINK_UP                    CPSW_WR_RGMII_CTL_RGMII1_LINK
#define CPSW_RGMII1_LINK_DOWN                  (0x00u)

/*
** Macros which can be passed as 'pacFlag' to the API CPSWWrIntPacingEnable
** CPSWWrIntPacingDisable
*/
#define CPSW_INT_PACING_C0_RX_PULSE            (0x01 << CPSW_WR_INT_CONTROL_INT_PACE_EN_SHIFT)
#define CPSW_INT_PACING_C0_TX_PULSE            (0x02 << CPSW_WR_INT_CONTROL_INT_PACE_EN_SHIFT)
#define CPSW_INT_PACING_C1_RX_PULSE            (0x04 << CPSW_WR_INT_CONTROL_INT_PACE_EN_SHIFT)
#define CPSW_INT_PACING_C1_TX_PULSE            (0x08 << CPSW_WR_INT_CONTROL_INT_PACE_EN_SHIFT)
#define CPSW_INT_PACING_C2_RX_PULSE            (0x10 << CPSW_WR_INT_CONTROL_INT_PACE_EN_SHIFT)
#define CPSW_INT_PACING_C2_TX_PULSE            (0x20 << CPSW_WR_INT_CONTROL_INT_PACE_EN_SHIFT)

/*
** Macros which can be passed as 'portState' to CPSWALEPortStateSet
*/
#define CPSW_ALE_PORT_STATE_FWD                (0x03u)
#define CPSW_ALE_PORT_STATE_LEARN              (0x02u)
#define CPSW_ALE_PORT_STATE_BLOCKED            (0x01u)
#define CPSW_ALE_PORT_STATE_DISABLED           (0x00u)

/*
** Macros which can be passed as 'eoiFlag' to CPSWCPDMAEndOfIntVectorWrite
*/
#define CPSW_EOI_TX_PULSE                      (0x02u)
#define CPSW_EOI_RX_PULSE                      (0x01u)
#define CPSW_EOI_RX_THRESH_PULSE               (0x00u)
#define CPSW_EOI_MISC_PULSE                    (0x03u)

/*
** Macro which can be passed as 'statFlag' to CPSWCPDMAStatusGet
** The same value can be used to compare against the idle status
*/
#define CPDMA_STAT_IDLE                        (CPSW_CPDMA_DMASTATUS_IDLE)

/*
** Macro which can be passed as 'statFlag' to CPSWCPDMAStatusGet
*/
#define CPDMA_STAT_TX_HOST_ERR_CODE            (CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE)

/* The return values for the above 'statFlag' */
#define CPDMA_STAT_TX_NO_ERR                   (0x00u << CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_TX_SOP_ERR                  (0x01u << CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_TX_OWN_ERR                  (0x02u << CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_TX_ZERO_DESC                (0x03u << CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_TX_ZERO_BUF_PTR             (0x04u << CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_TX_ZERO_BUF_LEN             (0x05u << CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_TX_PKT_LEN_ERR              (0x06u << CPSW_CPDMA_DMASTATUS_TX_HOST_ERR_CODE_SHIFT)

/*
** Macro which can be passed as 'statFlag' to CPSWCPDMAStatusGet
*/
#define CPDMA_STAT_RX_HOST_ERR_CODE            (CPSW_CPDMA_DMASTATUS_RX_HOST_ERR_CODE)

/* The return values for the above 'statFlag' */
#define CPDMA_STAT_RX_NO_ERR                   (0x00u << CPSW_CPDMA_DMASTATUS_RX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_RX_OWN_NOT_SET              (0x02u << CPSW_CPDMA_DMASTATUS_RX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_RX_ZERO_BUF_PTR             (0x04u << CPSW_CPDMA_DMASTATUS_RX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_RX_ZERO_BUF_LEN             (0x05u << CPSW_CPDMA_DMASTATUS_RX_HOST_ERR_CODE_SHIFT)
#define CPDMA_STAT_RX_SOP_BUF_LEN_ERR          (0x06u << CPSW_CPDMA_DMASTATUS_RX_HOST_ERR_CODE_SHIFT)

/*
** Macros which can be passed as 'statFlag' to CPSWCPDMAStatusGet
*/
#define CPDMA_STAT_TX_HOST_ERR_CHAN            (CPSW_CPDMA_DMASTATUS_TX_ERR_CH | 0x10u)
#define CPDMA_STAT_RX_HOST_ERR_CHAN            (CPSW_CPDMA_DMASTATUS_RX_ERR_CH | 0x08u)

/*
** Macro which can be passed as 'cfg' to the API CPSWCPDMAConfig
** The values for individual fields are also listed below.
*/
#define CPDMA_CFG(tx_rlim, rx_cef, cmd_idle, rx_offlen_blk, rx_own, tx_ptype) \
             (tx_rlim | rx_cef | cmd_idle | rx_offlen_blk | rx_own | tx_ptype)
/* Values for 'tx_rlim' */
#define CPDMA_CFG_TX_RATE_LIM_CH_7             (0x80u << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)
#define CPDMA_CFG_TX_RATE_LIM_CH_7_TO_6        (0xC0u << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)
#define CPDMA_CFG_TX_RATE_LIM_CH_7_TO_5        (0xE0u << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)
#define CPDMA_CFG_TX_RATE_LIM_CH_7_TO_4        (0xF0u << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)
#define CPDMA_CFG_TX_RATE_LIM_CH_7_TO_3        (0xF8u << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)
#define CPDMA_CFG_TX_RATE_LIM_CH_7_TO_2        (0xFCu << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)
#define CPDMA_CFG_TX_RATE_LIM_CH_7_TO_1        (0xFEu << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)
#define CPDMA_CFG_TX_RATE_LIM_CH_7_TO_0        (0xFFu << CPSW_CPDMA_DMACONTROL_TX_RLIM_SHIFT)

/* Values for 'rx_cef' */
#define CPDMA_CFG_COPY_ERR_FRAMES              (CPSW_CPDMA_DMACONTROL_RX_CEF)
#define CPDMA_CFG_NO_COPY_ERR_FRAMES           (0x00u)

/* Values for 'cmd_idle' */
#define CPDMA_CFG_IDLE_COMMAND                 (CPSW_CPDMA_DMACONTROL_CMD_IDLE)
#define CPDMA_CFG_IDLE_COMMAND_NONE            (0x00u)

/* Values for 'rx_offlen_blk' */
#define CPDMA_CFG_BLOCK_RX_OFF_LEN_WRITE       (CPSW_CPDMA_DMACONTROL_RX_OFFLEN_BLOCK)
#define CPDMA_CFG_NOT_BLOCK_RX_OFF_LEN_WRITE   (0x00u)

/* Values for 'rx_own' */
#define CPDMA_CFG_RX_OWN_1                     (CPSW_CPDMA_DMACONTROL_RX_OWNERSHIP)
#define CPDMA_CFG_RX_OWN_0                     (0x00u)

/* Values for 'tx_ptype' */
#define CPDMA_CFG_TX_PRI_ROUND_ROBIN           (CPSW_CPDMA_DMACONTROL_TX_PTYPE)
#define CPDMA_CFG_TX_PRI_FIXED                 (0x00u)

/*
** Macros which can be passed as 'intType' to CPSWCPDMARxIntStatRawGet
** and CPSWCPDMARxIntStatMaskedGet
*/
#define CPDMA_RX_INT_THRESH_PEND               (0x08u)
#define CPDMA_RX_INT_PULSE_PEND                (0x00u)

#define CPSW_MAX_NUM_ALE_ENTRY                 (1024)
#define CPSW_SIZE_CPPI_RAM                     (8192)

/*
** Structure to save CPSW context
*/
typedef struct cpswContext {
    uint32_t aleBase;
    uint32_t ssBase;
    uint32_t port1Base;
    uint32_t port2Base;
    uint32_t cpdmaBase;
    uint32_t cppiRamBase;
    uint32_t wrBase;
    uint32_t sl1Base;
    uint32_t sl2Base;
    uint32_t aleCtrl;
    uint32_t alePortCtl[3];
    uint32_t aleEntry[CPSW_MAX_NUM_ALE_ENTRY * 3];
    uint32_t ssStatPortEn;
    uint32_t port1SaHi;
    uint32_t port1SaLo;
    uint32_t port2SaHi;
    uint32_t port2SaLo;
    uint32_t port1TxInCtl;
    uint32_t port1Vlan;
    uint32_t port2TxInCtl;
    uint32_t port2Vlan;
    uint32_t cpdmaRxFB;
    uint32_t cpdmaTxCtl;
    uint32_t cpdmaRxCtl;
    uint32_t cpdmaRxHdp;
    uint32_t txIntMaskSet;
    uint32_t rxIntMaskSet;
    uint32_t wrCoreIntTxPulse;
    uint32_t wrCoreIntRxPulse;
    uint32_t sl1MacCtl;
    uint32_t sl2MacCtl;
    uint32_t cppiRam[CPSW_SIZE_CPPI_RAM];
} CPSWCONTEXT;

/*****************************************************************************/
/*
** Prototypes for the APIs
*/
extern void CPSWSSReset(uint32_t baseAddr);
extern void CPSWSlControlExtEnable(uint32_t baseAddr);
extern void CPSWSlGigModeForceEnable(uint32_t baseAddr);
extern void CPSWSlGigModeForceDisable(uint32_t baseAddr);
extern void CPSWSlTransferModeSet(uint32_t baseAddr, uint32_t mode);
extern uint32_t CPSWSlMACStatusGet(uint32_t baseAddr, uint32_t statFlag);
extern void CPSWSlReset(uint32_t baseAddr);
extern void CPSWSlRxMaxLenSet(uint32_t baseAddr, uint32_t rxMaxLen);
extern void CPSWSlGMIIEnable(uint32_t baseAddr);
extern void CPSWSlRGMIIEnable(uint32_t baseAddr);
extern void CPSWWrReset(uint32_t baseAddr);
extern void CPSWWrControlRegReset(uint32_t baseAddr);
extern void CPSWWrCoreIntEnable(uint32_t baseAddr, uint32_t core, uint32_t channel, uint32_t intFlag);
extern void CPSWWrCoreIntDisable(uint32_t baseAddr, uint32_t core, uint32_t channel, uint32_t intFlag);
extern uint32_t CPSWWrCoreIntStatusGet(uint32_t baseAddr, uint32_t core, uint32_t channel, uint32_t intFlag);
extern uint32_t CPSWWrRGMIIStatusGet(uint32_t baseAddr, uint32_t statFlag);
extern void CPSWALEInit(uint32_t baseAddr);
extern void CPSWALEPortStateSet(uint32_t baseAddr, uint32_t portNum, uint32_t portState);
extern void CPSWALETableEntrySet(uint32_t baseAddr, uint32_t aleTblIdx, uint32_t *aleEntryPtr);
extern void CPSWALETableEntryGet(uint32_t baseAddr, uint32_t aleTblIdx, uint32_t *aleEntryPtr);
extern uint32_t CPSWALEPrescaleGet(uint32_t baseAddr);
extern void CPSWALEPrescaleSet(uint32_t baseAddr, uint32_t psVal);
extern void CPSWALEBypassEnable(uint32_t baseAddr);
extern void CPSWALEBypassDisable(uint32_t baseAddr);
extern void CPSWRxFlowControlEnable(uint32_t baseAddr, uint32_t portNum);
extern void CPSWRxFlowControlDisable(uint32_t baseAddr, uint32_t portNum);
extern void CPSWSoftwareIdleEnable(uint32_t baseAddr);
extern void CPSWSoftwareIdleDisable(uint32_t baseAddr, uint32_t portNum);
extern void CPSWStatisticsEnable(uint32_t baseAddr);
extern void CPSWVLANAwareEnable(uint32_t baseAddr);
extern void CPSWVLANAwareDisable(uint32_t baseAddr);
extern void CPSWPortSrcAddrSet(uint32_t baseAddr, uint8_t *ethAddr);
extern uint32_t CPSWStatisticsGet(uint32_t baseAddr, uint32_t statReg);
extern void CPSWCPDMAReset(uint32_t baseAddr);
extern void CPSWCPDMACmdIdleEnable(uint32_t baseAddr);
extern void CPSWCPDMACmdIdleDisable(uint32_t baseAddr);
extern void CPSWCPDMATxIntEnable(uint32_t baseAddr, uint32_t channel);
extern void CPSWCPDMARxIntEnable(uint32_t baseAddr, uint32_t channel);
extern void CPSWCPDMATxIntDisable(uint32_t baseAddr, uint32_t channel);
extern void CPSWCPDMARxIntDisable(uint32_t baseAddr, uint32_t channel);
extern void CPSWCPDMATxEnable(uint32_t baseAddr);
extern void CPSWCPDMARxEnable(uint32_t baseAddr);
extern void CPSWCPDMATxHdrDescPtrWrite(uint32_t baseAddr, uint32_t descHdr, uint32_t channel);
extern void CPSWCPDMARxHdrDescPtrWrite(uint32_t baseAddr, uint32_t descHdr, uint32_t channel);
extern void CPSWCPDMAEndOfIntVectorWrite(uint32_t baseAddr, uint32_t eoiFlag);
extern void CPSWCPDMATxCPWrite(uint32_t baseAddr, uint32_t channel, uint32_t comPtr);
extern void CPSWCPDMARxCPWrite(uint32_t baseAddr, uint32_t channel, uint32_t comPtr);
extern void CPSWCPDMANumFreeBufSet(uint32_t baseAddr, uint32_t channel, uint32_t nBuf);
extern uint32_t CPSWCPDMAStatusGet(uint32_t baseAddr, uint32_t statFlag);
extern void CPSWCPDMAConfig(uint32_t baseAddr, uint32_t cfg);
extern void CPSWCPDMARxBufOffsetSet(uint32_t baseAddr, uint32_t bufOff);
extern uint32_t CPSWCPDMATxIntStatRawGet(uint32_t baseAddr, uint32_t chanMask);
extern uint32_t CPSWCPDMATxIntStatMaskedGet(uint32_t baseAddr, uint32_t chanMask);
extern uint32_t CPSWCPDMARxIntStatRawGet(uint32_t baseAddr, uint32_t chanMask, uint32_t intType);
extern uint32_t CPSWCPDMARxIntStatMaskedGet(uint32_t baseAddr, uint32_t channel, uint32_t intFlag);
extern void CPSWContextSave(CPSWCONTEXT *contextPtr);
extern void CPSWContextRestore(CPSWCONTEXT *contextPtr);
extern void CPSWHostPortDualMacModeSet(uint32_t baseAddr);
extern void CPSWALEVLANAwareSet(uint32_t baseAddr);
extern void CPSWALEVLANAwareClear(uint32_t baseAddr);
extern void CPSWPortVLANConfig(uint32_t baseAddr, uint32_t vlanId, uint32_t cfiBit, uint32_t vlanPri);
extern void CPSWALERateLimitTXMode(uint32_t baseAddr);
extern void CPSWALERateLimitRXMode(uint32_t baseAddr);
extern void CPSWALERateLimitEnable(uint32_t baseAddr);
extern void CPSWALERateLimitDisable(uint32_t baseAddr);
extern void CPSWALEAUTHModeSet(uint32_t baseAddr);
extern void CPSWALEAUTHModeClear(uint32_t baseAddr);
extern void CPSWALEUnknownUntaggedEgressSet(uint32_t baseAddr, uint32_t ueVal);
extern void CPSWALEUnknownRegFloodMaskSet(uint32_t baseAddr, uint32_t rfmVal);
extern void CPSWALEUnknownUnRegFloodMaskSet(uint32_t baseAddr, uint32_t ufmVal);
extern void CPSWALEUnknownMemberListSet(uint32_t baseAddr, uint32_t mlVal);
extern void CPSWALEBroadcastRateLimitSet(uint32_t baseAddr, uint32_t portNum, uint32_t bplVal);
extern void CPSWALEMulticastRateLimitSet(uint32_t baseAddr, uint32_t portNum, uint32_t mplVal);
extern void CPSWALEVIDIngressCheckSet(uint32_t baseAddr, uint32_t portNum);
extern void CPSWALEAgeOut(uint32_t baseAddr);

void CPSWEVMPortMIIModeSelect();
void CPSWEVMMACAddrGet(uint32_t addrIdx, uint8_t *macAddr);
void CPSWPinMuxSetup();
void CPSWClkEnable();

#ifdef __cplusplus
}
#endif

#endif /* DR_CPSW_H */
