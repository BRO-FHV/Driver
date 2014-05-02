/*
 * mem.h
 *
 *  Created on: 12.06.2013
 *      Author: Daniel
 */

#ifndef MEM_GENERIC_H_
#define MEM_GENERIC_H_

#include <basic.h>

typedef uint8_t memorytype_t;

typedef struct {
    bool_t occupied;
    bool_t reserved;
} mem_pagetable_lookup_t;

typedef struct {
    uint32_t globalStartAddress;
    uint32_t globalSize;
    uint32_t userStartAddress;
    uint32_t userSize;
    uint32_t pageTableTotalCount;
    uint32_t pageTableAllocatedCount;
    mem_pagetable_lookup_t* pageTableLookup;
} mem_memory_t;

typedef struct {
    uint32_t startAddress;
    uint32_t size;
} mem_devicememory_t;



mem_memory_t* mem_get(memorytype_t t);
void mem_init(void);

void mem_reserve_page(memorytype_t type, uint32_t pageNumber);
void mem_reserve_pages(memorytype_t type, uint32_t firstPageNumber, uint32_t pageCount);

void mem_free_page(memorytype_t type, uint32_t pageNumber);
void mem_free_pages(memorytype_t type, uint32_t firstPageNumber, uint32_t pageCount);

void* mem_get_page_address(memorytype_t type, uint32_t pageNumber);
uint32_t mem_get_page_number(memorytype_t* type, uint32_t address);

void* mem_find_free(uint32_t pageCount, bool_t align, bool_t reserve);
void* mem_find_free_in(memorytype_t type, uint32_t pageCount, bool_t align, bool_t reserve);

#endif /* MEM_H_ */
