#ifndef KHEAP_H
#define KHEAP_H

#include <stdint.h>
#include <stddef.h>

size_t heap_get_allocated();

void init_kheap();


void* malloc(size_t size);


void free(void* ptr);

#endif