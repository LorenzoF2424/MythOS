#ifndef MALLOC_H
#define MALLOC_H

#include <stdint.h>
#include <stddef.h>
#include "./terminal_driver/kprintf.h"
 






void pmm_init();

uintptr_t placement_address; 

void* kmalloc_bump(size_t size);









#endif