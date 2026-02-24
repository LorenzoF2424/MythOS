[BITS 64]

global idt_flush
global outb
global inb

;args order: RDI,RSI
idt_flush:
    lidt [rdi]
ret


outb:
    mov dx, di      
    mov al, sil     
    out dx, al      
ret


inb:
    mov dx, di      
    xor rax, rax    
    in al, dx       
ret

global keyboard_isr
extern keyboard_handler_c  ; La funzione C++ che scriveremo tra poco

keyboard_isr:
    ; 1. Salva tutti i registri che il C++ potrebbe sporcare
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11

    ; 2. Chiama la nostra bellissima funzione in C++
    call keyboard_handler_c

    
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    
iretq