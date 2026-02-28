#include "malloc.h"




void* kmalloc_bump(size_t size) {
    void* tmp = (void*)placement_address;
    placement_address += size;
    return tmp;
}