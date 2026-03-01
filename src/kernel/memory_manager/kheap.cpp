#include "kheap.h"
#include "pmm.h"
#include "../terminal_driver/kprintf.h"
#define NUM_CACHES 8


struct SlabHeader {
    uint32_t block_size;
};

struct HeapFreeNode {
    HeapFreeNode* next;
};

static uint32_t cache_sizes[NUM_CACHES] = {16, 32, 64, 128, 256, 512, 1024, 2048};
static HeapFreeNode* cache_free_lists[NUM_CACHES];



void init_kheap() {
    for (int i = 0; i < NUM_CACHES; i++) {
        cache_free_lists[i] = nullptr;
    }
    kprintf("Heap: Slab Allocator initialized.\n");
}


void* malloc(size_t size) {
    if (size == 0) return nullptr;

    int cache_idx = -1;
    for (int i = 0; i < NUM_CACHES; i++) {
        if (size <= cache_sizes[i]) {
            cache_idx = i;
            break;
        }
    }


    if (cache_idx == -1) {
        kprintf("Heap Error: malloc request too large (%d bytes)\n", size);
        return nullptr;
    }

    uint32_t block_size = cache_sizes[cache_idx];

    if (cache_free_lists[cache_idx] == nullptr) {
        void* new_page = pmm_alloc_blocks(0); 
        if (!new_page) {
            kprintf("Heap Error: Out of memory (PMM returned null)\n");
            return nullptr;
        }

        SlabHeader* header = (SlabHeader*)new_page;
        header->block_size = block_size;

        uint64_t start_addr = (uint64_t)new_page + sizeof(SlabHeader);
        uint64_t end_addr = (uint64_t)new_page + PAGE_SIZE;

        for (uint64_t curr = start_addr; curr + block_size <= end_addr; curr += block_size) {
            HeapFreeNode* node = (HeapFreeNode*)curr;
            node->next = cache_free_lists[cache_idx];
            cache_free_lists[cache_idx] = node;
        }
        
        kprintf("Heap: Allocated new page for %d-byte cache.\n", block_size);
    }

    HeapFreeNode* allocated_node = cache_free_lists[cache_idx];
    cache_free_lists[cache_idx] = allocated_node->next;

    return (void*)allocated_node;
}


void free(void* ptr) {
    if (!ptr) return;

    uint64_t page_start = (uint64_t)ptr & ~0xFFF;
    SlabHeader* header = (SlabHeader*)page_start;

    uint32_t block_size = header->block_size;

    int cache_idx = -1;
    for (int i = 0; i < NUM_CACHES; i++) {
        if (block_size == cache_sizes[i]) {
            cache_idx = i;
            break;
        }
    }

    if (cache_idx != -1) {
        HeapFreeNode* node = (HeapFreeNode*)ptr;
        node->next = cache_free_lists[cache_idx];
        cache_free_lists[cache_idx] = node;
    } else {
        kprintf("Heap Error: Attempted to free an invalid or corrupted pointer.\n");
    }
}