#include "kernel/mem/kheap.h"
#include "kernel/mem/pmm.h"
#include "drivers/display/kprintf.h"

#define NUM_CACHES 8
#define SLAB_MAGIC 0xDEADC0DE

// Header at the start of each physical page to track block sizes
struct SlabHeader {
    uint32_t block_size;
};

// Security wrapper for user allocations to detect corruption
struct SlabBlock {
    uint32_t magic_start; // Redzone/Canary field
    uint8_t  payload[0];  // Start of user data
};

// Offset needed to reach the payload from the start of a block
#define SLAB_OVERHEAD (sizeof(uint32_t))

// Node structure for the linked list of free blocks
struct HeapFreeNode {
    HeapFreeNode* next;
};

// Predefined cache sizes for the Slab Allocator
static uint32_t cache_sizes[NUM_CACHES] = {16, 32, 64, 128, 256, 512, 1024, 2048};
static HeapFreeNode* cache_free_lists[NUM_CACHES];
size_t heap_allocated_bytes = 0;
Spinlock heap_lock;

size_t heap_get_allocated() {
    return heap_allocated_bytes;
}

// Initializes the free lists for each cache size
void init_kheap() {
    for (int i = 0; i < NUM_CACHES; i++) {
        cache_free_lists[i] = nullptr;
    }
    kprintf("Heap: Slab Allocator initialized with Redzone protection.\n");
}

void* malloc(size_t size) {
    spinlock_acquire(&heap_lock);

    // Early return: invalid request size
    if (size == 0) {
        spinlock_release(&heap_lock);
        return nullptr;
    }

    // Adjust requested size to include the safety overhead
    size_t total_needed = size + SLAB_OVERHEAD;
    int cache_idx = -1;

    // Find the smallest cache that can fit the requested size + overhead
    for (int i = 0; i < NUM_CACHES; i++) {
        if (total_needed <= cache_sizes[i]) {
            cache_idx = i;
            break;
        }
    }

    // Early return: request size exceeds maximum slab size
    if (cache_idx == -1) {
        kprintf("Heap Error: malloc request too large (%d bytes)\n", size);
        spinlock_release(&heap_lock);
        return nullptr;
    }

    uint32_t block_size = cache_sizes[cache_idx];

    // If the cache list is empty, allocate a new page from PMM
    if (cache_free_lists[cache_idx] == nullptr) {
        void* new_page = pmm_alloc_blocks(0); 
        if (!new_page) {
            kprintf("Heap Error: Out of memory (PMM returned null)\n");
            spinlock_release(&heap_lock);
            return nullptr;
        }

        // Initialize the page header
        SlabHeader* header = (SlabHeader*)new_page;
        header->block_size = block_size;

        // Populate the page with free blocks
        uint64_t start_addr = (uint64_t)new_page + sizeof(SlabHeader);
        uint64_t end_addr = (uint64_t)new_page + PAGE_SIZE;

        for (uint64_t curr = start_addr; curr + block_size <= end_addr; curr += block_size) {
            HeapFreeNode* node = (HeapFreeNode*)curr;
            node->next = cache_free_lists[cache_idx];
            cache_free_lists[cache_idx] = node;
        }
    }

    // Pop a block from the free list
    SlabBlock* block = (SlabBlock*)cache_free_lists[cache_idx];
    cache_free_lists[cache_idx] = ((HeapFreeNode*)block)->next;

    // Initialize the canary/magic value for corruption detection
    block->magic_start = SLAB_MAGIC;

    heap_allocated_bytes += block_size;
    spinlock_release(&heap_lock);

    // Return the pointer to the payload (skipping the magic_start field)
    return (void*)(block->payload);
}

void free(void* ptr) {
    // Early return: cannot free a null pointer
    if (!ptr) return;

    spinlock_acquire(&heap_lock);

    // Calculate the actual start of the SlabBlock structure
    SlabBlock* block = (SlabBlock*)((uint64_t)ptr - SLAB_OVERHEAD);

    // Security Check: verify if the canary value is still intact
    if (block->magic_start != SLAB_MAGIC) {
        kprintf("Heap Error: Attempted to free an invalid or corrupted pointer at %p.\n", ptr);
        spinlock_release(&heap_lock);
        return;
    }

    // Determine the page header to identify the correct cache list
    uint64_t page_start = (uint64_t)block & ~0xFFF;
    SlabHeader* header = (SlabHeader*)page_start;
    uint32_t block_size = header->block_size;

    int cache_idx = -1;
    for (int i = 0; i < NUM_CACHES; i++) {
        if (block_size == cache_sizes[i]) {
            cache_idx = i;
            break;
        }
    }

    // If block size is valid, return the block to the free list
    if (cache_idx != -1) {
        // Clear the magic value to prevent double-free or use-after-free
        block->magic_start = 0; 

        HeapFreeNode* node = (HeapFreeNode*)block;
        node->next = cache_free_lists[cache_idx];
        cache_free_lists[cache_idx] = node;
        heap_allocated_bytes -= block_size;
    } else {
        kprintf("Heap Error: Page header corruption detected for pointer %p.\n", ptr);
    }

    spinlock_release(&heap_lock);
}

// ==========================================
// Memory Subsystem Initialization
// ==========================================
void init_memory() {
    init_pmm();
    init_vmm();

    // Retrieve kernel page tables
    uint64_t* kernel_pml4 = vmm_get_kernel_pml4();
    if (!kernel_pml4) return; // Critical failure

    // Ensure higher-half kernel mapping
    vmm_copy_higher_half(kernel_pml4);

    // Map physical memory to the higher half
    uint64_t total_mb = get_total_memory_mb();
    uint64_t ram_to_map = (total_mb + 16ULL) * 1024ULL * 1024ULL;
    vmm_map_range(kernel_pml4, 0, 0, ram_to_map, PAGE_PRESENT | PAGE_RW);
    vmm_map_page(kernel_pml4, 0x0, 0x0, 0); // Null pointer protection if needed

    // Activate the new page tables
    vmm_switch_pml4(kernel_pml4);
    
    // Finally initialize the Heap allocator
    init_kheap();

    // Unified log for the memory subsystem status
    klog("[%s] Memory ready: %ld MB mapped, PML4 at %p, Heap active", __func__, total_mb, kernel_pml4);
}