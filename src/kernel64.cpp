#include "cli/cli.h"
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

    init_memory();

    
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
    
    
    init_vfs();




    init_cmd_manager();

    klog("[%s] Kernel fully initialized. Interrupts enabled.", __func__);
}




void test() {
    
    uint64_t a=10000000000000;
    kprintf("test a = %ld\n", a);
    uint1024_t b;
    b = 1000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000_u1024;
    kprintf("test b = %llllld\n", &b);
   
   

}


extern "C" void main() { 

    __asm__ __volatile__ ("mov %0, %%rsp" : : "r"(system_stack + 65536));
    init_all();
    
    

    //test();

    
    vfs_print_prompt();
    input.setStart();

    
    //thread_create(info_bar_thread);
    
    while (true) {


        current_tick = ticks;
        cli_main();    
        previous_tick = current_tick;

        



    }

    //__asm__ __volatile__ ("hlt");

}