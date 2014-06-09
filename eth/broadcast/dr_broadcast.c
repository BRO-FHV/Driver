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
#include "timer/dr_timer.h"

#define PORT 		2000
#define DELAY 		5000
#define USE_TIMER	Timer_TIMER4

static char msg[] = "testing";
static struct udp_pcb *pcb;


void udp_echo_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	if (p != NULL) {
		udp_sendto(pcb, p, IP_ADDR_BROADCAST, PORT);
		pbuf_free(p);
	}
}

void sendBroadcastMsg() {
	struct pbuf *p;
	p = pbuf_alloc(PBUF_TRANSPORT, sizeof(msg), PBUF_RAM);

	memcpy(p->payload, msg, sizeof(msg));
	udp_sendto(pcb, p, IP_ADDR_BROADCAST, PORT);
	pbuf_free(p); //De-allocate packet buffer
	printf("broadcast message send...\n");
}

void BroadcastStart() {
	pcb = udp_new();
	udp_bind(pcb, IP_ADDR_ANY, PORT);
	udp_recv(pcb, udp_echo_recv, NULL);

	TimerConfiguration(USE_TIMER, DELAY, sendBroadcastMsg);
	TimerEnable(USE_TIMER);
}
