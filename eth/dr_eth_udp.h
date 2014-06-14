/*
 * Driver: dr_eth_udp.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 14.06.2014
 * Description: 
 * TODO
 */

#ifndef DR_ETH_UDP_H_
#define DR_ETH_UDP_H_

#include <lwip/broipinput.h>

void bro_udp_input(eth_header* ethHeader, ip_header* ipHeader, udp_header* udp_header, uint8_t* data, uint32_t dataLen);

#endif /* DR_ETH_UDP_H_ */
