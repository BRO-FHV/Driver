/*
 * mmu.c
 *
 *  Created on: 13.06.2013
 *      Author: Daniel
 */

#include "mmu.h"
#include "../mmu.h"


uint8_t __mmu_get_domain_access(uint8_t domain)
{
    switch(domain)
    {
        case 0:
            return 0x03;
        default:
            return 0x01;
    }
}
