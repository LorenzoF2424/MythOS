#include "gnu_utils/cpp_support.h"

void* operator new(size_t size) {
    return malloc(size); 
}

void* operator new[](size_t size) {
    return malloc(size);
}

void operator delete(void* p) {
    free(p); 
}

void operator delete[](void* p) {
    free(p);
}


void operator delete(void* p, size_t size) {
    free(p);
}