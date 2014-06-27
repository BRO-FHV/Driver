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

#include <inttypes.h>
#include "basic.h"
#include "lwip/broipinput.h"
#include "lwip/udp.h"

typedef struct {
	uint8_t* data;
	uint32_t len;
	uint8_t sender[IP_ADDR_LENGTH];
} udp_package_t;

typedef struct {
	uint16_t port;
	struct udp_pcb *pcb;

	udp_package_t package;
} udp_connection_t;


void BroUdpInit(uint16_t port);
void BroUdpInput(eth_header_t* ethHeader, ip_header_t* ipHeader, udp_header_t* udp_header, uint8_t data[], uint32_t dataLen);
udp_package_t* BroUdpGetData(uint16_t port);
void BroUdpSendData(uint8_t receiver[], uint16_t port, uint8_t* data, uint32_t dataLen);
tBoolean BroUdpHasData(uint16_t port);

#endif /* DR_ETH_UDP_H_ */
