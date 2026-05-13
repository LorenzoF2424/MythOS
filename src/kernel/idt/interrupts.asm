[BITS 64]

global idt_flush
global outb
global inb



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




