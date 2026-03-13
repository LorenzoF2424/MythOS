#include "cli/CLI.h"
uint8_t system_stack[65536];
extern uint8_t initstack[];


void enable_fpu() {
    uint64_t cr0, cr4;

    // 1. Leggiamo il registro CR0
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); // Resetta il bit EM (Emulation) - dice alla CPU di usare l'hardware vero
    cr0 |= (1 << 1);  // Setta il bit MP (Monitor co-processor)
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));

    // 2. Leggiamo il registro CR4 per attivare le istruzioni SSE
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 9);  // OSFXSR: Attiva il salvataggio dei registri FXSAVE/FXRSTOR
    cr4 |= (1 << 10); // OSXMMEXCPT: Attiva le eccezioni hardware per i float
    asm volatile ("mov %0, %%cr4" :: "r"(cr4));

    // 3. Inizializza lo stato dell'FPU
    asm volatile ("fninit");
}


void setup_memory() {

    
    init_pmm();

    init_vmm();
    uint64_t* kernel_pml4 = vmm_get_kernel_pml4();

    vmm_copy_higher_half(kernel_pml4);

    
    vmm_map_range(kernel_pml4, 0, 0, (get_total_memory_mb() + 16ULL) * 1024ULL * 1024ULL, PAGE_PRESENT | PAGE_RW);

    vmm_switch_pml4(kernel_pml4);

    init_kheap();
   
}

void init_all() {

    init_display();
    init_idt();
    terminal_write_welcome_message();
    init_exceptions();
    setup_memory();
    sysCommandAt("check stack", point(42, 1));
    sysCommandAt("check cpu", point(42, 2));
    sysCommandAt("check cs", point(42, 0));

    
    init_keyboard();
    init_timer(1000);
    enable_fpu();
    init_mouse();

    asm volatile ("sti");
}


void test() {
  
}

extern "C" void main() { 

    __asm__ __volatile__ ("mov %0, %%rsp" : : "r"(system_stack + 65536));
    init_all();
    
    

    test();



    kprintf("MythOS>");
    terminal_set_input_limit();

    
    while (true) {


        current_tick = ticks;
        cli_main();    
        previous_tick = current_tick;

        



        //__asm__ __volatile__ ("hlt");
    }
}