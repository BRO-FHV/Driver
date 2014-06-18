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

udp_connection_t* BroTcpGetConnection(uint32_t port);

void BroUdpInput(eth_header_t* ethHeader, ip_header_t* ipHeader, udp_header_t* udp_header, uint8_t data[], uint32_t dataLen) {

	udp_connection_t* conn = BroTcpGetConnection(ConvertBigToLittleEndian(udp_header->destPort));

	if (NULL != conn) {
		//copy package data to avoid data loss
		conn->package.data = (uint8_t*) malloc(dataLen);
		memcpy(conn->package.data, data, dataLen);
		conn->package.len = dataLen;
		//copy sender ip
		memcpy(conn->package.sender, ipHeader->srcIp, IP_ADDR_LENGTH);

		printf("received data (%d): %s\n", conn->port, data);
	} else {
		printf("received data at Port %d, but no connection is associated with this port.\n", conn->port);
	}
}

udp_connection_t* BroTcpGetConnection(uint32_t port) {
	uint8_t i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connections[i].port == port) {
			return &connections[i];
		}
	}

	return NULL;
}

void BroUdpInit(uint32_t port) {
	if (currentIndex < 10) {
		connections[currentIndex].pcb = udp_new();
		connections[currentIndex].port = port;
		connections[currentIndex].package.data = NULL;
		connections[currentIndex].package.len = 0;

		++currentIndex;
	}
}

upd_package_t* BroUdpGetData(uint32_t port) {
	udp_connection_t* conn = BroTcpGetConnection(port);

	return NULL != conn ? &conn->package : NULL;
}

void BroUdpSendData(uint8_t receiver[], uint32_t port, uint8_t* data, uint32_t dataLen) {
	udp_connection_t* conn = BroTcpGetConnection(port);

	if (NULL != conn) {
		udp_bind(conn->pcb, IP_ADDR_ANY, port);

		struct pbuf *p;
		p = pbuf_alloc(PBUF_TRANSPORT, dataLen, PBUF_RAM);

		ipAddr.addr = IpToInt(receiver);

		memcpy(p->payload, data, dataLen);
		udp_sendto(conn->pcb, p, &ipAddr, port);
		pbuf_free(p); //De-allocate packet buffer
		printf("message send to Port %d\n", port);
	} else {
		printf("sending data to Port %d, but no connection is associated with this port.\n", port);
	}
}

tBoolean BroUdpHasData(uint32_t port){
	udp_connection_t* conn = BroTcpGetConnection(port);

	return NULL != conn && conn->package.len > 0 ? TRUE : FALSE;
}
