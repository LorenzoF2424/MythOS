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

    vmm_copy_higher_half(kernel_pml4);

   
    uint64_t ram_to_map = (get_total_memory_mb() + 16ULL) * 1024ULL * 1024ULL;
    vmm_map_range(kernel_pml4, 0, 0, ram_to_map, PAGE_PRESENT | PAGE_RW);

   
    vmm_map_page(kernel_pml4, 0x0, 0x0, 0); 

    vmm_switch_pml4(kernel_pml4);

    init_kheap();
}

void init_all() {

    init_display();
    

    init_tss();
    init_gdt();
    init_idt();
    init_exceptions();

    if (draw_info) terminal_data.cursor.y = 4;
    terminal_change_color(terminal_color(vga_palette[current_palette][0], vga_palette[current_palette][15]));
    terminal_write_welcome_message();
    setup_memory();
    //sysCommandAt("check stack", point(42, 1));
    //sysCommandAt("check cpu", point(42, 2));
    //sysCommandAt("check cs", point(42, 0));

    init_spurious();
    init_keyboard();
    init_timer(1000);
    enable_fpu();
    init_mouse();

    asm volatile ("sti");

    init_scheduler();
    

}


void test() {
  for (size_t i=0;i<256;i++) {
    kprintf("%d: %c   ", i, i);
  }
  kprintf("\n");
}

extern "C" void main() { 

    __asm__ __volatile__ ("mov %0, %%rsp" : : "r"(system_stack + 65536));
    init_all();
    
    

    test();

    
    kprintf("MythOS>");
    terminal_set_input_limit();

    thread_create(info_bar_thread);
    
    while (true) {


        current_tick = ticks;
        cli_main();    
        previous_tick = current_tick;

        



    }

    //__asm__ __volatile__ ("hlt");

}