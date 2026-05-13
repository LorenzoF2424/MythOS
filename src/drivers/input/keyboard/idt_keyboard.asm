[BITS 64]

%include "macros.asm"


global keyboard_isr
extern keyboard_handler_c 

keyboard_isr:
    pushall


    call keyboard_handler_c
    
    popall
iretq