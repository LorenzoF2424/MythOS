[BITS 64]
%include "macros.asm"

global timer_isr
extern timer_handler_c
extern scheduler_switch

timer_isr:
    pushall

    call timer_handler_c    

    mov rdi, rsp            
    call scheduler_switch   
    mov rsp, rax            

    popall
iretq