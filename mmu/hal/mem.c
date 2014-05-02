/*
 * mem.c
 *
 *  Created on: 13.06.2013
 *      Author: Daniel
 */

#include "mem.h"
#include <stdlib.h>

extern volatile uint32_t int_ram_user_start;
extern volatile uint32_t ext_ddr_user_start;

void __mem_init(void)
{
    memory_count = 2;
    memories = malloc(memory_count * sizeof(mem_memory_t));

    // int_ram
    memories[0].globalStartAddress = 0x84000000;
    memories[0].globalSize = 0x00100000;
    memories[0].userStartAddress = (uint32_t) 0x85000000;
    memories[0].userSize = 0x00100000;

    // ext_ram
    memories[1].globalStartAddress = 0x80000000;
    memories[1].globalSize = 0x10000000;
    memories[1].userStartAddress = (uint32_t) 0x82000000;
    memories[1].userSize = 0x10000000;

    // interrupt vectors
    intvecs_start = 0x4020FFC0;
    intvecs_size = 0x0000003B;

    // memory mapped IOs
    device_memory_count = 9;
    device_memories = malloc(device_memory_count * sizeof(mem_devicememory_t));

    // l4-core
    device_memories[0].startAddress = 0x48000000;
    device_memories[0].size = 0x01000000;
    // l4-per
    device_memories[1].startAddress = 0x49000000;
    device_memories[1].size = 0x00100000;
    // sgx
    device_memories[2].startAddress = 0x50000000;
    device_memories[2].size = 0x00100000;
    // l4-emu
    device_memories[3].startAddress = 0x54000000;
    device_memories[3].size = 0x00800000;
    // iva 2.2
    device_memories[4].startAddress = 0x5c000000;
    device_memories[4].size = 0x03000000;
    // l3
    device_memories[5].startAddress = 0x68000000;
    device_memories[5].size = 0x01000000;
    // sms
    device_memories[6].startAddress = 0x6c000000;
    device_memories[6].size = 0x01000000;
    // sdrc
    device_memories[7].startAddress = 0x6d000000;
    device_memories[7].size = 0x01000000;
    // gpmc
    device_memories[8].startAddress = 0x6e000000;
    device_memories[8].size = 0x01000000;
}
