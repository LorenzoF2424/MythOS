#include "IDT.h"


idt_entry_t idt[256];
idt_ptr_t idt_ptr;


volatile uint8_t kbd_buffer[KBD_BUFFER_SIZE];
volatile uint16_t kbd_head = 0; 
volatile uint16_t kbd_tail = 0; 



void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {

    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_mid  = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].base_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    
    idt[num].sel = sel;
    idt[num].ist = 0;
    idt[num].flags = flags; 
 
    idt[num].always0 = 0;
}

void init_idt() {
    idt_ptr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint64_t)&idt;

   
    uint8_t *idt_raw = (uint8_t*)&idt;
    for (size_t i = 0; i < sizeof(struct idt_entry_t) * 256; i++) {
        idt_raw[i] = 0;
    }

    
    idt_flush((uint64_t)&idt_ptr);
}

void remap_pic() {
    
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    
    outb(PIC1_DATA, 0x20); 
    outb(PIC2_DATA, 0x28); 

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

  
    outb(PIC1_DATA, 0xFD); 
    outb(PIC2_DATA, 0xFF);
}




void kbd_push(uint8_t scancode) {
    uint16_t next_head = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (next_head != kbd_tail) { 
        kbd_buffer[kbd_head] = scancode;
        kbd_head = next_head;
    }
}

bool kbd_pop(uint8_t *scancode) {
    if (kbd_head == kbd_tail) {
        return false; 
    }
    *scancode = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
    return true;
}



