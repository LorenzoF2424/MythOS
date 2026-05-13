#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>

//  Page Table Entries Flags (PTE)
#define PAGE_PRESENT    (1ULL << 0)
#define PAGE_RW         (1ULL << 1) // Read/Write
#define PAGE_USER       (1ULL << 2) // User mode (ring 3) access
#define PAGE_NX         (1ULL << 63) // No Execute

#define PTE_ADDR_MASK   0x000FFFFFFFFFF000

void init_vmm();

bool vmm_map_page(uint64_t* pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags);

void vmm_switch_pml4(uint64_t* pml4);

uint64_t* vmm_get_kernel_pml4();

void vmm_map_range(uint64_t* pml4, uint64_t vaddr_start, uint64_t paddr_start, uint64_t size, uint64_t flags);

void vmm_copy_higher_half(uint64_t* new_pml4);

#endif