#include "CLI.h"
uint8_t system_stack[65536];
extern uint8_t initstack[];




void init_all_except_stack() {

    init_display();
    init_keyboard();


    terminal_write_welcome_message();
    sysCommandAt("check ram", 42, 0);
    sysCommandAt("check stack", 42, 1);
    sysCommandAt("check cpu", 42, 2);
    sysCommandAt("check cs", 80, 0);

}



extern "C" void main() { 

    __asm__ __volatile__ ("mov %0, %%rsp" : : "r"(system_stack + 65536));
    init_all_except_stack();
    
 
    kprintf("MythicOS>");
    terminal_set_input_limit();

    
    while (true) {
        cli_main();    
    
        




    ticks++;}
}