// cpp_support.h
#ifndef CPP_SUPPORT_H
#define CPP_SUPPORT_H

#include <stddef.h>
#include "mem/kheap.h"

// --- Global new and delete operators for C++ in the kernel ---
// These override the standard behavior, routing allocations to the kernel's memory manager.

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* p);
void operator delete[](void* p);

// Sized-delete (often required by C++14 and newer)
void operator delete(void* p, size_t size);

// --- Pure Virtual Function Handler ---
// This handles the catastrophic failure where a pure virtual function is called.
// It is declared as C-linkage so the C++ compiler can find it by name.

extern "C" void __cxa_pure_virtual();

#endif // CPP_SUPPORT_H