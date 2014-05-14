/*
 * Driver: dr_broadcast.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 14.05.2014
 * Description: 
 * TODO
 */

#include "lwip/udp.h"
#include "dr_broadcast.h"
#include <string.h>
#include <stdio.h>

#define PORT 2000

void udp_echo_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	if (p != NULL) {
		udp_sendto(pcb, p, IP_ADDR_BROADCAST, PORT);
		pbuf_free(p);
	}
}

void broadcast_start() {
	struct udp_pcb *pcb;
	char msg[] = "testing";
	struct pbuf *p;

	pcb = udp_new();
	udp_bind(pcb, IP_ADDR_ANY, PORT);
	udp_recv(pcb, udp_echo_recv, NULL);

	while (1) {
		p = pbuf_alloc(PBUF_TRANSPORT, sizeof(msg), PBUF_RAM);
		memcpy(p->payload, msg, sizeof(msg));
		udp_sendto(pcb, p, IP_ADDR_BROADCAST, PORT);
		pbuf_free(p); //De-allocate packet buffer
		printf("message send!\n");

		volatile int i = 0;
		for (i = 0; i < 10000; i++) {
		}
	}
}
