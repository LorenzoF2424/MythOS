#include "idt/IDT.h"


idt_entry_t idt[256];
idt_ptr_t idt_ptr;


volatile uint8_t kbd_buffer[KBD_BUFFER_SIZE];
volatile uint16_t kbd_head = 0; 
volatile uint16_t kbd_tail = 0; 

void remap_pic() {
    

    // wake up PICs and set new offsets
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    // Set new offsets: PIC1 -> 0x20 (32), PIC2 -> 0x28 (40)
    outb(PIC1_DATA, 0x20); 
    outb(PIC2_DATA, 0x28); 


    // Tell PIC1 that there is a slave PIC at IRQ2 (0000 0100)
    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    // Set PICs to 8086 mode
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

    // set masks: unmask only IRQ0 (timer) and IRQ1 (keyboard)
    outb(PIC1_DATA, 0b1111'1000); 
    outb(PIC2_DATA, 0b1110'1111);
}


void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_mid  = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].base_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    
    idt[num].sel = sel;
    
    // Se è un Double Fault, usa l'IST 1 (indice 1 nella TSS)
    if (num == 8) {
        idt[num].ist = 1; 
    } else {
        idt[num].ist = 0;
    }

    idt[num].flags = flags; 
    idt[num].always0 = 0;
}

void init_idt() {
    idt_ptr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint64_t)&idt;

    for (int i = 0; i < 256; i++) {
        uint8_t *ptr = (uint8_t*)&idt[i];
        for (size_t j = 0; j < sizeof(idt_entry_t); j++) ptr[j] = 0;
    }

    
   

    idt_flush((uint64_t)&idt_ptr);

    remap_pic();
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



