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
#include "dr_eth_tcp.h"
#include "basic.h"

#define MAX_CONNECTIONS		10

ip_addr_t ipAddr;
tcp_connection_t connections[MAX_CONNECTIONS];
uint8_t currentIndex;

tcp_connection_t* BroTcpGetConnection(uint32_t port);

void BroTcpInput(eth_header_t* ethHeader, ip_header_t* ipHeader, tcp_header_t* tcpHeader, uint8_t data[], uint32_t dataLen) {

	tcp_connection_t* conn = BroTcpGetConnection(ConvertBigToLittleEndian(tcpHeader->destPort));

	if (NULL != conn) {
		printf("TCP Connection found - impl todo\n");
	} else {
		printf("connection not found - haha\n");
	}
}

tcp_connection_t* BroTcpGetConnection(uint32_t port) {
	uint8_t i;
	for (i = 0; i < MAX_CONNECTIONS; i++) {
		if (connections[i].port == port) {
			return &connections[i];
		}
	}

	return NULL;
}

void BroTcpInit(uint32_t port) {
	if (currentIndex < 10) {
		connections[currentIndex].pcb = tcp_new();
		connections[currentIndex].port = port;
		connections[currentIndex].package.data = NULL;
		connections[currentIndex].package.len = 0;

		++currentIndex;
	}
}

tcp_package_t* BroTcpGetData(uint32_t port) {
	tcp_connection_t* conn = BroTcpGetConnection(port);

	return NULL != conn ? &conn->package : NULL;
}

void BroTcpSendData(uint8_t receiver[], uint32_t port, uint8_t* data, uint32_t dataLen) {
	tcp_connection_t* conn = BroTcpGetConnection(port);

	if (NULL != conn) {
		printf("TCP Connection found - impl todo\n");
	} else {
		printf("connection not found - haha\n");
	}
}

tBoolean BroTcpHasData(uint32_t port){
	tcp_connection_t* conn = BroTcpGetConnection(port);

	return NULL != conn && conn->package.len > 0 ? TRUE : FALSE;
}
