#ifndef MALLOC_H
#define MALLOC_H

#include <stdint.h>
#include <stddef.h>
#include "kprintf.h" 






void pmm_init();

uintptr_t placement_address = 0xFFFFFFFFC0200000; 

void* kmalloc_bump(size_t size) {
    void* tmp = (void*)placement_address;
    placement_address += size;
    return tmp;
}









#endif