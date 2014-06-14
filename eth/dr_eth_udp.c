/*
 * Driver: dr_eth_udp.c
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 14.06.2014
 * Description: 
 * Processing UDP packets
 */

#include <inttypes.h>
#include <lwip/broipinput.h>

#include "dr_eth_udp.h"

void bro_udp_input(eth_header* ethHeader, ip_header* ipHeader, udp_header* udp_header, uint8_t* data, uint32_t dataLen) {

	int x = 1;
}
