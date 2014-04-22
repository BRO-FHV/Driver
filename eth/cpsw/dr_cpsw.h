/*
 * Driver: dr_cpsw.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 11.04.2014
 * Description:
 *
 */
void CPSWEVMPortMIIModeSelect();
void CPSWEVMMACAddrGet(uint32_t addrIdx, uint8_t *macAddr);
void CPSWPinMuxSetup();
void CPSWClkEnable();
