/*
 * mem.c
 *
 *  Created on: 12.06.2013
 *      Author: Daniel
 */

#include "mem.h"
#include "mmu.h"
#include <stdlib.h>
#include <string.h>
#include <basic.h>

uint32_t intvecs_start;
uint32_t intvecs_size;

uint8_t memory_count;
mem_memory_t* memories;

uint8_t device_memory_count;
mem_devicememory_t* device_memories;

// Two booleans for each entry
mem_memory_t* mem_get(memorytype_t t)
{
    return &memories[t];
}

void mem_init(void)
{
    memorytype_t i;
    __mem_init();

    for(i = 0; i < memory_count; i++)
    {
        memories[i].pageTableTotalCount = memories[i].globalSize  / MMU_MASTER_TABLE_PAGE_SIZE;
        memories[i].pageTableAllocatedCount = 0;
        memories[i].pageTableLookup = malloc(sizeof(mem_pagetable_lookup_t) * memories[i].pageTableTotalCount);
        memset(memories[i].pageTableLookup, 0, sizeof(mem_pagetable_lookup_t) * memories[i].pageTableTotalCount);
    }
}

void mem_reserve_pages(memorytype_t type, uint32_t firstPageNumber, uint32_t pageCount)
{
    int i;
    for (i = 0; i < pageCount; i++)
    {
        mem_reserve_page(type, firstPageNumber + i);
    }
}

void mem_reserve_page(memorytype_t type, uint32_t pageNumber)
{
    mem_memory_t* m = mem_get(type);
    if (pageNumber >= m->pageTableTotalCount)
    {
        return;
    }
    m->pageTableLookup[pageNumber].reserved = TRUE;
    m->pageTableAllocatedCount++;
    return;
}

void mem_free_page(memorytype_t type, uint32_t pageNumber)
{
    mem_memory_t* m = mem_get(type);
    if (pageNumber >= m->pageTableTotalCount)
    {
        return;
    }
    m->pageTableLookup[pageNumber].reserved = FALSE;
    m->pageTableAllocatedCount--;
    // reset the memory of the page
    memset(mem_get_page_address(type, pageNumber), 0, MMU_MASTER_TABLE_PAGE_SIZE);
}

void mem_free_pages(memorytype_t type, uint32_t firstPageNumber, uint32_t pageCount)
{
    int i;
    for (i = 0; i < pageCount; i++)
    {
        mem_free_page(type, firstPageNumber + i);
    }
}

void* mem_get_page_address(memorytype_t type, uint32_t pageNumber)
{
    mem_memory_t* m = mem_get(type);
    if (pageNumber >= m->pageTableTotalCount)
    {
        return NULL;
    }
    return (void*) (m->globalStartAddress + (pageNumber * MMU_MASTER_TABLE_PAGE_SIZE));
}

uint32_t mem_get_page_number(memorytype_t* type, uint32_t address)
{
    memorytype_t i;
    mem_memory_t* m;
    for(i = 0; i < memory_count; i++)
    {
        m = mem_get(i);
        if(address >= m->globalStartAddress && address < (m->globalStartAddress + m->globalSize))
        {
            *type = i;
            return (address - m->globalStartAddress) / MMU_MASTER_TABLE_PAGE_SIZE;
        }
    }
    return 0;
}

void* mem_find_free_in(memorytype_t type, uint32_t pageCount, bool_t align, bool_t reserve)
{
    //
    // find pageCount unoccupied pages in a row
    void* result = NULL;
    mem_memory_t* m = mem_get(type);
    uint32_t pagesFound = 0;
    uint32_t i;

    for (i = 0; i < m->pageTableTotalCount; i++)
    {
        if (!m->pageTableLookup[i].reserved && ((!align) || (pagesFound > 0) || ((i % pageCount) == 0)))
        {
            pagesFound++;
            if (pagesFound == pageCount)
            {
                result = mem_get_page_address(type, (i - pageCount) + 1);
                if (reserve)
                {
                    mem_reserve_pages(type, (i - pageCount) + 1, pageCount);
                }
                break;
            }
        }
        else
        {
            pagesFound = 0;
        }
    }
    return result;
}

void* mem_find_free(uint32_t pageCount, bool_t align, bool_t reserve)
{
    void* address;
    memorytype_t i;
    for(i = 0; i < memory_count; i++)
    {
        address = mem_find_free_in(i, pageCount, align, reserve);
        if(address != NULL)
        {
            return address;
        }
    }
    return NULL;
}
