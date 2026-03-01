#include "CLI.h"
uint8_t system_stack[65536];
extern uint8_t initstack[];




void init_all_except_memory_stuff() {

    init_display();
    init_keyboard();


    terminal_write_welcome_message();
    sysCommandAt("check ram", 42, 0);
    sysCommandAt("check stack", 42, 1);
    sysCommandAt("check cpu", 42, 2);
    sysCommandAt("check cs", 80, 0);
}

void setup_memory() {
    init_pmm();

    init_vmm();
    uint64_t* kernel_pml4 = vmm_get_kernel_pml4();

    vmm_copy_higher_half(kernel_pml4);

   
    vmm_map_range(kernel_pml4, 0, 0, 256 * 1024 * 1024, PAGE_PRESENT | PAGE_RW);

    vmm_switch_pml4(kernel_pml4);

    init_kheap();
}

void test() {
   kprintf("--- Starting Final Memory Integration Test ---\n");

    // 1. Test Dynamic Allocation (KHeap + PMM)
    void* ptr1 = malloc(64);
    void* ptr2 = malloc(64);
    
    kprintf("Test 1 - Allocation: ptr1=%p, ptr2=%p\n", ptr1, ptr2);
    
    if (ptr1 && ptr2 && ptr1 != ptr2) {
        kprintf("Test 1 - SUCCESS: Slab is slicing pages correctly.\n");
    } else {
        kprintf("Test 1 - FAILED: Allocation error!\n");
    }

    // 2. Test Virtual Mapping (VMM)
    // Proviamo a scrivere e leggere in uno dei puntatori
    // Se il VMM non fosse mappato correttamente (Identity mapping), qui avremmo un Page Fault
    uint64_t* check = (uint64_t*)ptr1;
    *check = 0xDEADC0DECAFEBABE;
    
    if (*check == 0xDEADC0DECAFEBABE) {
        kprintf("Test 2 - SUCCESS: Virtual memory writing works at %p\n", ptr1);
    } else {
        kprintf("Test 2 - FAILED: Memory corruption!\n");
    }

    // 3. Test Reuse (Free + Malloc)
    free(ptr1);
    void* ptr3 = malloc(64);
    kprintf("Test 3 - Reuse: freed ptr1=%p, new ptr3=%p\n", ptr1, ptr3);

    if (ptr1 == ptr3) {
        kprintf("Test 3 - SUCCESS: Memory recycled correctly.\n");
    } else {
        kprintf("Test 3 - WARNING: Memory was not recycled, but it might be okay.\n");
    }

    kprintf("--- All Systems Nominal: Kernel is ready! ---\n");
}

extern "C" void main() { 

    __asm__ __volatile__ ("mov %0, %%rsp" : : "r"(system_stack + 65536));
    init_all_except_memory_stuff();
    setup_memory();
 

    test();



    kprintf("MythicOS>");
    terminal_set_input_limit();

    
    while (true) {
        cli_main();    
    
        




    ticks++;}
}