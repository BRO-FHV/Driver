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
#include "lwip/tcp.h"
#include "lwip/broipinput.h"

typedef struct {
	uint8_t* data;
	uint32_t len;
	uint8_t sender[IP_ADDR_LENGTH];
} tcp_package_t;

typedef struct {
	uint32_t port;
	struct tcp_pcb *pcb;

	upd_package_t package;
} tcp_connection_t;


void BroTcpInit(uint32_t port);
void BroTcpInput(eth_header_t* ethHeader, ip_header_t* ipHeader, tcp_header_t* tcpHeader, uint8_t data[], uint32_t dataLen);
tcp_package_t* BroTcpGetData(uint32_t port);
void BroTcpSendData(uint8_t receiver[], uint32_t port, uint8_t* data, uint32_t dataLen);
tBoolean BroTcpHasData(uint32_t port);

#endif /* DR_ETH_UDP_H_ */
