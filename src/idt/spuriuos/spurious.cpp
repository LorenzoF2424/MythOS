#include "idt/spurious/spurious.h"
#include "cmos/io.h"

#define PIC1_CMD 0x20
#define PIC2_CMD 0xA0

bool is_spurious_irq7() {
    outb(PIC1_CMD, 0x0B);
    uint8_t isr = inb(PIC1_CMD);
    return !(isr & 0x80);
}


void irq7_handler() {
    if (is_spurious_irq7()) {
        return;
    }
    outb(PIC1_CMD, 0x20);
}

void irq15_handler() {
    outb(PIC2_CMD, 0x0B);
    uint8_t isr = inb(PIC2_CMD);
    
    if (!(isr & 0x80)) {
        outb(PIC1_CMD, 0x20);
        return;
    }

    outb(PIC2_CMD, 0x20);
    outb(PIC1_CMD, 0x20);
}

void init_spurious() {
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    
    idt_set_gate(39, (uint64_t)irq7_handler, cs, 0x8E);
    idt_set_gate(47, (uint64_t)irq15_handler, cs, 0x8E);
}