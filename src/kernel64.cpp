#include "cli/CLI.h"
uint8_t system_stack[65536];
extern uint8_t initstack[];


void enable_fpu() {
    uint64_t cr0, cr4;

    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); 
    cr0 |= (1 << 1); 
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));

    
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 9);  
    cr4 |= (1 << 10); 
    asm volatile ("mov %0, %%cr4" :: "r"(cr4));

    
    asm volatile ("fninit");
}


void setup_memory() {
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
void init_all() {

    klog("MythOS: Boot sequence initiated");
    init_display();
    terminal.init();
    
    klog("Terminal initialized. Screen is active.");

    init_tss();
    init_gdt();
    init_idt();
    init_exceptions();

    klog("CPU Exceptions are now catching errors.");

    
    terminal.change_color(terminal_color(vga_palette[current_palette][0], vga_palette[current_palette][15]));
    terminal.write_welcome();

    setup_memory();

    
    info_bar_window = terminal;
    info_bar_window.y_offset=0;
    draw_info_bar();


    init_spurious();
    init_keyboard();
    init_timer(1000);
    enable_fpu();
    init_mouse();

    asm volatile ("sti");

    init_scheduler();
    
    klog("[%s] Kernel fully initialized. Interrupts enabled.", __func__);
}




void test() {
    
}

extern "C" void main() { 

    __asm__ __volatile__ ("mov %0, %%rsp" : : "r"(system_stack + 65536));
    init_all();
    
    

    

    
    kprintf("MythOS>");
    input.setStart();
    
    //thread_create(info_bar_thread);
    
    while (true) {


        current_tick = ticks;
        cli_main();    
        previous_tick = current_tick;

        



    }

    //__asm__ __volatile__ ("hlt");

}