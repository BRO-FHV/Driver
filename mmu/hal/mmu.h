/*
 * mmu.h
 *
 *  Created on: 13.06.2013
 *      Author: Daniel
 */

#ifndef MMU_GENARC_H_
#define MMU_GENARC_H_

#include <stdint.h>

void __mmu_enable(void);
void __mmu_disable(void);

void __mmu_flush_tlb();
void __mmu_set_kernel_table(uint32_t tableAddress);
void __mmu_set_process_table(uint32_t tableAddress);

void __mmu_set_domain_access(uint32_t domainAccess);
uint8_t __mmu_get_domain_access(uint8_t domain);
void __mmu_load_dabt_data(void);

#endif /* MMU_H_ */
