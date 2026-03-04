#include "vmm.h"
#include "pmm.h"
#include "../terminal_driver/kprintf.h"

uint64_t* kernel_pml4 = nullptr;

void zero_page(void* page) {
    uint64_t* p = (uint64_t*)page;
    for (int i = 0; i < 512; i++) {
        p[i] = 0;
    }
}

void init_vmm() {
    kernel_pml4 = (uint64_t*)pmm_alloc_blocks(0);
    
    if (!kernel_pml4) {
        kprintf("PANIC: Unable to allocate Kernel PML4!\n");
        return;
    }
    
    zero_page(kernel_pml4);
    
    kprintf("VMM Initialized (PML4: %p)\n", (uint64_t)kernel_pml4);
}

uint64_t* vmm_get_kernel_pml4() {
    return kernel_pml4;
}


bool vmm_map_page(uint64_t* pml4, uint64_t vaddr, uint64_t paddr, uint64_t flags) {

    uint64_t pml4_idx = (vaddr >> 39) & 0x1FF;
    uint64_t pdpt_idx = (vaddr >> 30) & 0x1FF;
    uint64_t pd_idx   = (vaddr >> 21) & 0x1FF;
    uint64_t pt_idx   = (vaddr >> 12) & 0x1FF;


    if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
        uint64_t* new_pdpt = (uint64_t*)pmm_alloc_blocks(0);
        if (!new_pdpt) return false;
        zero_page(new_pdpt);
        pml4[pml4_idx] = (uint64_t)new_pdpt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }
    uint64_t* pdpt = (uint64_t*)(pml4[pml4_idx] & PTE_ADDR_MASK);



    if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
        uint64_t* new_pd = (uint64_t*)pmm_alloc_blocks(0);
        if (!new_pd) return false;
        zero_page(new_pd);
        pdpt[pdpt_idx] = (uint64_t)new_pd | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }
    uint64_t* pd = (uint64_t*)(pdpt[pdpt_idx] & PTE_ADDR_MASK);



    if (!(pd[pd_idx] & PAGE_PRESENT)) {
        uint64_t* new_pt = (uint64_t*)pmm_alloc_blocks(0);
        if (!new_pt) return false;
        zero_page(new_pt);
        pd[pd_idx] = (uint64_t)new_pt | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }
    uint64_t* pt = (uint64_t*)(pd[pd_idx] & PTE_ADDR_MASK);



    pt[pt_idx] = (paddr & PTE_ADDR_MASK) | flags;

    asm volatile("invlpg (%0)" ::"r" (vaddr) : "memory");

    return true;
}

void vmm_switch_pml4(uint64_t* pml4) {
    asm volatile("mov %0, %%cr3" :: "r" ((uint64_t)pml4));
}


void vmm_map_range(uint64_t* pml4, uint64_t vaddr_start, uint64_t paddr_start, uint64_t size, uint64_t flags) {
    uint64_t start_v = vaddr_start & ~(PAGE_SIZE - 1);
    uint64_t start_p = paddr_start & ~(PAGE_SIZE - 1);
    uint64_t end_v = (vaddr_start + size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uint64_t i = 0; start_v + i < end_v; i += PAGE_SIZE) {
        vmm_map_page(pml4, start_v + i, start_p + i, flags);
    }
}



void vmm_copy_higher_half(uint64_t* new_pml4) {
    uint64_t current_cr3;

    asm volatile("mov %%cr3, %0" : "=r"(current_cr3));
    
    uint64_t* old_pml4 = (uint64_t*)(current_cr3 & PTE_ADDR_MASK);
    
    for (int i = 256; i < 512; i++) {
        new_pml4[i] = old_pml4[i];
    }
}