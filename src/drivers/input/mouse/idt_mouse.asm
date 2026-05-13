[bits 64]

%include "macros.asm"  
extern mouse_handler_c
global mouse_isr

mouse_isr:
    pushall

    cld

    call mouse_handler_c


    popall
iretq