/*
 * Driver: dr_eth.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 04.04.2014
 * Description: 
 * TODO
 */

#ifndef DR_ETH_H_
#define DR_ETH_H_

#include <inttypes.h>

void EthConfigureWithIP(uint32_t ip);
uint32_t EthConfigureWithDHCP();

#endif /* DR_ETH_H_ */
