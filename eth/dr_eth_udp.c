/*
 * Driver: dr_eth_udp.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 14.06.2014
 * Description: 
 * Processing UDP packets
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "lwip/broipinput.h"
#include "lwip/udp.h"
#include "dr_eth_udp.h"
#include "basic.h"

#define MAX_CONNECTIONS		10

ip_addr_t ipAddr;
udp_connection_t connections[MAX_CONNECTIONS];
uint8_t currentIndex;

udp_connection_t* get_udp_connection(uint32_t port);

void bro_udp_input(eth_header* ethHeader, ip_header* ipHeader, udp_header* udp_header, uint8_t data[], uint32_t dataLen) {

	udp_connection_t* conn = get_udp_connection(convertBigToLittleEndian(udp_header->destPort));

	if (NULL != conn) {
		//copy package data to avoid data loss
		conn->package.data = (uint8_t*) malloc(dataLen);
		memcpy(conn->package.data, data, dataLen);
		conn->package.len = dataLen;
		//copy sender ip
		memcpy(conn->package.sender, ipHeader->srcIp, IP_ADDR_LENGTH);

		printf("received data (%d): %s\n", conn->port, data);
	} else {
		printf("connection not found - haha\n");
	}
}

udp_connection_t* get_udp_connection(uint32_t port) {
	uint8_t i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connections[i].port == port) {
			return &connections[i];
		}
	}

	return NULL;
}

void bro_udp_init(uint32_t port) {
	if (currentIndex < 10) {
		connections[currentIndex].pcb = udp_new();
		connections[currentIndex].port = port;
		connections[currentIndex].package.data = NULL;
		connections[currentIndex].package.len = 0;

		++currentIndex;
	}
}

upd_package_t* bro_udp_get_data(uint32_t port) {
	udp_connection_t* conn = get_udp_connection(port);

	return NULL != conn ? &conn->package : NULL;
}

void bro_udp_send_data(uint8_t receiver[], uint32_t port, uint8_t* data, uint32_t dataLen) {
	udp_connection_t* conn = get_udp_connection(port);

	if (NULL != conn) {
		udp_bind(conn->pcb, IP_ADDR_ANY, port);

		struct pbuf *p;
		p = pbuf_alloc(PBUF_TRANSPORT, dataLen, PBUF_RAM);

		ipAddr.addr = ipToInt(receiver);

		memcpy(p->payload, data, dataLen);
		udp_sendto(conn->pcb, p, &ipAddr, port);
		pbuf_free(p); //De-allocate packet buffer
		printf("message send\n");
	} else {
		printf("connection not found - haha\n");
	}
}

tBoolean bro_udp_has_data(uint32_t port){
	udp_connection_t* conn = get_udp_connection(port);

	return NULL != conn && conn->package.len > 0 ? TRUE : FALSE;
}
