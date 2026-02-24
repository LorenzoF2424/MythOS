#include "string.h"
#include "kprintf.h"
#include "IDT.h"

uint8_t system_stack[65536];
extern uint8_t initstack[];
/*void test_tastiera() {
    // Leggiamo un byte dalla porta della tastiera (0x60)
    uint8_t scancode = inb(0x60);
    
    // Se lo scancode è diverso da 0, qualcuno ha premuto o rilasciato un tasto!
    if (scancode != 0) {
        kprintf("Scancode hardware ricevuto: %x\n", scancode);
    }
}*/

void check_stack() {
    uint64_t rsp;
    asm volatile("mov %%rsp, %0" : "=r"(rsp));
    kprintf("RSP = %lx/%ld\n", rsp, rsp);
}



extern "C" void main() { // KERNELLLLLLLLLLLLLLLLLLLLLLL

    __asm__ __volatile__ ("mov %0, %%rsp" : : "r"(system_stack + 65536));
    //enable_full_stack();
    init_display();
    init_keyboard();
    //play_sound(880);
    terminal_write_welcome_message();
    
    check_stack();
    kprintf("hello_world!\n");
    kprintf("adfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
    kprintf("adfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
    kprintf("adfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
    kprintf("adfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\n");
    /*terminal_clear();
    for (int i=1;i<1000;i++) {
        kprintf("%d\n",i);
        if (terminal_data.cursor_row>=MAX_ROWS) break;
    }*/
    

    const char *a = "AAACAAAAAAAAAAAAAAAAA\0";
    const char *b = "AAAAAABAAAAAAAAAAAAAA\0";

    if (strcmp(a,b)==0)
        kprintf("same strings!\n");
    if (strcmp(a,b)>0)
        kprintf("a>b!\n");
    if (strcmp(a,b)<0)
        kprintf("a<b!\n");

   
    terminal_set_input_limit();

    while (1) {


         
      
    

    }
}