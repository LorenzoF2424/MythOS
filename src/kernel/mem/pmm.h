#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>
#include "../terminal_driver/kprintf.h"
#include "../terminal_driver/bootboot.h" 

#define PAGE_SIZE 4096
#define MAX_ORDER 8     //2^x for the highest order

struct FreeBlock {
    FreeBlock* next;
};

void init_pmm();

void* pmm_alloc_blocks(uint8_t order);

void pmm_free_blocks(void* ptr, uint8_t order);

size_t pmm_get_allocated();

#endif