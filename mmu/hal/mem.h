/*
 * mem.h
 *
 *  Created on: 13.06.2013
 *      Author: Daniel
 */

#ifndef MEM_GENARCH_H_
#define MEM_GENARCH_H_


#include "../mem.h"

extern uint32_t intvecs_start;
extern uint32_t intvecs_size;

extern uint8_t memory_count;
extern mem_memory_t* memories;
extern uint8_t device_memory_count;
extern mem_devicememory_t* device_memories;

void __mem_init(void);

#endif /* MEM_H_ */
