#include "idt/exceptions/handler.h"
#include "idt/idt.h" 
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

extern "C" void exception_handler(registers_t* regs) {
    //sysCommand("clear");
    sysCommand("color 4F");
    kprintf("\n===================================================\n");
    kprintf("                   KERNEL PANIC                    \n");
    kprintf("===================================================\n");
    
    // Stampiamo le informazioni base dall'oggetto regs
    kprintf("Exception: %s (%x)\n", exception_messages[regs->int_no], regs->int_no);
    kprintf("Error Code: %x\n\n", regs->err_code);
    
    // --- LA SUPER GRIGLIA DI DEBUG ---
    kprintf("RAX: %016lx  RBX: %016lx  RCX: %016lx\n", regs->rax, regs->rbx, regs->rcx);
    kprintf("RDX: %016lx  RSI: %016lx  RDI: %016lx\n", regs->rdx, regs->rsi, regs->rdi);
    kprintf("R8 : %016lx  R9 : %016lx  R10: %016lx\n", regs->r8,  regs->r9,  regs->r10);
    kprintf("RBP: %016lx  RSP: %016lx  RIP: %016lx\n", regs->rbp, regs->rsp, regs->rip);
    kprintf("CS : %016lx  RFL: %016lx\n", regs->cs, regs->rflags);
    
    // Se è un Page Fault (Eccezione 14), leggiamo il registro CR2!
    if (regs->int_no == 14) {
        uint64_t cr2;
        asm volatile("mov %%cr2, %0" : "=r" (cr2));
        kprintf("\n-> FAULTING ADDRESS (CR2): %016lx <-\n", cr2);
    }
    
    kprintf("\nSystem Halted.\n");

    while(true) {
        asm volatile("cli; hlt");
    }
}