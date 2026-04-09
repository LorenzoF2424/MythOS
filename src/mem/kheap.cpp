#include "mem/kheap.h"
#include "mem/pmm.h"
#include "term/kprintf.h"
#define NUM_CACHES 8


struct SlabHeader {
    uint32_t block_size;
};

struct HeapFreeNode {
    HeapFreeNode* next;
};

static uint32_t cache_sizes[NUM_CACHES] = {16, 32, 64, 128, 256, 512, 1024, 2048};
static HeapFreeNode* cache_free_lists[NUM_CACHES];
size_t heap_allocated_bytes=0;
Spinlock heap_lock;

size_t heap_get_allocated() {
    return heap_allocated_bytes;
}

void init_kheap() {
    for (int i = 0; i < NUM_CACHES; i++) {
        cache_free_lists[i] = nullptr;
    }
    kprintf("Heap: Slab Allocator initialized.\n");
}


void* malloc(size_t size) {

    spinlock_acquire(&heap_lock);
    if (size == 0) {
        spinlock_release(&heap_lock);
        return nullptr;
    }

    int cache_idx = -1;
    for (int i = 0; i < NUM_CACHES; i++) {
        if (size <= cache_sizes[i]) {
            cache_idx = i;
            break;
        }
    }


    if (cache_idx == -1) {
        kprintf("Heap Error: malloc request too large (%d bytes)\n", size);
        spinlock_release(&heap_lock);
        return nullptr;
    }

    uint32_t block_size = cache_sizes[cache_idx];

    if (cache_free_lists[cache_idx] == nullptr) {
        void* new_page = pmm_alloc_blocks(0); 
        if (!new_page) {
            kprintf("Heap Error: Out of memory (PMM returned null)\n");
            spinlock_release(&heap_lock);
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
        
        //kprintf("Heap: Allocated new page for %d-byte cache.\n", block_size);
    }

    HeapFreeNode* allocated_node = cache_free_lists[cache_idx];
    cache_free_lists[cache_idx] = allocated_node->next;


    heap_allocated_bytes += block_size;
    spinlock_release(&heap_lock);
    return (void*)allocated_node;
}


void free(void* ptr) {

    spinlock_acquire(&heap_lock);
    if (!ptr) {
        spinlock_release(&heap_lock);
        return;
    }

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
        heap_allocated_bytes -= block_size;
    } else {
        kprintf("Heap Error: Attempted to free an invalid or corrupted pointer.\n");
    }

    spinlock_release(&heap_lock);
}


void init_memory() {
    init_pmm();
    init_vmm();

    uint64_t* kernel_pml4 = vmm_get_kernel_pml4();
    if (!kernel_pml4) return; // Early return: Critical failure

    vmm_copy_higher_half(kernel_pml4);

    uint64_t total_mb = get_total_memory_mb();
    uint64_t ram_to_map = (total_mb + 16ULL) * 1024ULL * 1024ULL;
    vmm_map_range(kernel_pml4, 0, 0, ram_to_map, PAGE_PRESENT | PAGE_RW);
    vmm_map_page(kernel_pml4, 0x0, 0x0, 0); 

    vmm_switch_pml4(kernel_pml4);
    init_kheap();

    // Just one clean log for the entire memory subsystem
    klog("[%s] Memory ready: %ld MB mapped, PML4 at %p, Heap active", __func__, total_mb, kernel_pml4);
}