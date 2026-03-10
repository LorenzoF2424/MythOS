#include "idt/exceptions/handler.h"
#include "idt/IDT.h" 
#include "term/kprintf.h"
#include "idt/keyboard/command.h"

// --- L'ARRAY DEI MESSAGGI DI ERRORE ---
const char* exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};

void* isr_stub_table[32] = {
    (void*)isr0, (void*)isr1, (void*)isr2, (void*)isr3,
    (void*)isr4, (void*)isr5, (void*)isr6, (void*)isr7,
    (void*)isr8, (void*)isr9, (void*)isr10, (void*)isr11,
    (void*)isr12, (void*)isr13, (void*)isr14, (void*)isr15,
    (void*)isr16, (void*)isr17, (void*)isr18, (void*)isr19,
    (void*)isr20, (void*)isr21, (void*)isr22, (void*)isr23,
    (void*)isr24, (void*)isr25, (void*)isr26, (void*)isr27,
    (void*)isr28, (void*)isr29, (void*)isr30, (void*)isr31
};

void init_exceptions() {
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs)); 

    for (int i = 0; i < 32; i++) {
        // 0x8E = Interrupt Gate, cs, Ring 0
        idt_set_gate(i, (uint64_t)isr_stub_table[i], cs, 0x8E); 
    }
}

extern "C" void exception_handler(uint64_t interrupt_number, uint64_t error_code) {
    
    
    //sysCommand("clear");
    sysCommand("color 4F");
    kprintf("\n===================================================\n");
    kprintf("                   KERNEL PANIC                    \n");
    kprintf("===================================================\n");
    kprintf("Exception: %s (%x)\n", exception_messages[interrupt_number], interrupt_number);
    
    kprintf("Error Code: %x\n", error_code);
    kprintf("System Halted.\n");

    while(true) {
        asm volatile("cli; hlt");
    }
}