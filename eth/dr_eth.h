/*
 * Driver: dr_eth.h
 * Part of BRO Project, 2014 <<https://github.com/BRO-FHV>>
 *
 * Created on: 04.04.2014
 * Description: 
 * Interface that gains access to the ethernet controller.
 */

#ifndef DR_ETH_H_
#define DR_ETH_H_

#include <inttypes.h>

uint32_t EthConfigureWithIP(uint32_t ip, uint32_t netMask, uint32_t gateway);

#endif /* DR_ETH_H_ */
