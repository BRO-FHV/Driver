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
#include "lwip/udp.h"
#include "lwip/broipinput.h"

typedef struct {
	uint8_t* data;
	uint32_t len;
	uint8_t sender[IP_ADDR_LENGTH];
} upd_package_t;

typedef struct {
	uint32_t port;
	struct udp_pcb *pcb;

	upd_package_t package;
} udp_connection_t;


void bro_udp_init(uint32_t port);
void bro_udp_input(eth_header* ethHeader, ip_header* ipHeader, udp_header* udp_header, uint8_t data[], uint32_t dataLen);
upd_package_t* bro_udp_get_data(uint32_t port);
void bro_udp_send_data(uint8_t receiver[], uint32_t port, uint8_t* data, uint32_t dataLen);
tBoolean bro_udp_has_data(uint32_t port);

#endif /* DR_ETH_UDP_H_ */
