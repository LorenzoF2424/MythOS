#include "pmm.h"


extern BOOTBOOT bootboot;

FreeBlock* free_lists[MAX_ORDER + 1];
uint64_t total_free_memory = 0;

inline uint64_t get_size(uint8_t order) {
    return (uint64_t)PAGE_SIZE << order;
}

void pmm_push_block(void* ptr, uint8_t order) {
    FreeBlock* block = (FreeBlock*)ptr;
    block->next = free_lists[order];
    free_lists[order] = block;
}

void* pmm_pop_block(uint8_t order) {
    if (!free_lists[order]) return nullptr;
    FreeBlock* block = free_lists[order];
    free_lists[order] = block->next;
    return block;
}

void init_pmm() {
    
    for (int i = 0; i <= MAX_ORDER; i++) {
        free_lists[i] = nullptr;
    }

    uint32_t num_entries = (bootboot.size - 128) / 16;
    MMapEnt *mmap = &bootboot.mmap;

    for (uint32_t i = 0; i < num_entries; i++) {
        uint8_t type = mmap[i].size & 0xF;
        if (type != 1) continue;

        uint64_t base = mmap[i].ptr;
        uint64_t size = mmap[i].size & 0xFFFFFFFFFFFFFFF0;

        uint64_t start = (base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
        uint64_t end = (base + size) & ~(PAGE_SIZE - 1);

        while (start < end) {
            uint8_t order = MAX_ORDER;
            
            while (order > 0) {
                uint64_t block_size = get_size(order);
                if (start + block_size <= end && (start % block_size) == 0) {
                    break;
                }
                order--;
            }
            
            pmm_push_block((void*)start, order);
            total_free_memory += get_size(order);
            start += get_size(order);
        }
    }
    
    kprintf("Buddy System ready. Free memory: %d MB\n", total_free_memory / (1024 * 1024));
}

void* pmm_alloc_blocks(uint8_t order) {
    if (order > MAX_ORDER) return nullptr;

    for (uint8_t i = order; i <= MAX_ORDER; i++) {
        if (free_lists[i] != nullptr) {
            void* block = pmm_pop_block(i);

            while (i > order) {
                i--;
                uint64_t buddy = (uint64_t)block + get_size(i);
                pmm_push_block((void*)buddy, i);
            }
            return block;
        }
    }
    
    kprintf("PANIC: Out of physical memory!\n");
    return nullptr;
}

void pmm_free_blocks(void* ptr, uint8_t order) {
    uint64_t addr = (uint64_t)ptr;

    while (order < MAX_ORDER) {
        uint64_t buddy_addr = addr ^ get_size(order);
        FreeBlock* prev = nullptr;
        FreeBlock* curr = free_lists[order];
        bool merged = false;

        while (curr != nullptr) {
            if ((uint64_t)curr == buddy_addr) {
                if (prev) prev->next = curr->next;
                else free_lists[order] = curr->next;

                addr = addr < buddy_addr ? addr : buddy_addr;
                order++;
                merged = true;
                break;
            }
            prev = curr;
            curr = curr->next;
        }

        if (!merged) break; 
    }
    
    pmm_push_block((void*)addr, order);
}