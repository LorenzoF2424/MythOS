#include "idt/exceptions/handler.h"
#include "idt/idt.h" 
#include "term/kprintf.h"
#include "idt/keyboard/command.h"

char klogs[MAX_KLOGS][LOG_SIZE];
uint8_t log_index;
bool klogs_enabled = true;

// --- EXCEPTION ERROR MESSAGES ---
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

    // Initialize log buffers with null terminators
    for (size_t i = 0; i < MAX_KLOGS; i++) {
        strinit(klogs[i], LOG_SIZE);
    }
}

void klog(const char* format, ...) {
    // Early Return: Safety check for null pointer or disabled logging
    if (!format || !klogs_enabled) return;

    va_list args;
    va_start(args, format);

    if (log_index < MAX_KLOGS) {
        // Buffer has space: format directly into the next available slot
        vsnprintf(klogs[log_index], LOG_SIZE, format, args);
        log_index++;
    } else {
        // Buffer is full: shift all existing logs UP by one position
        for (size_t i = 0; i < MAX_KLOGS - 1; i++) {
            strcpy(klogs[i], klogs[i + 1]);
        }
        
        // Re-initialize the last slot to ensure it's clean before writing
        strinit(klogs[MAX_KLOGS - 1], LOG_SIZE);
        vsnprintf(klogs[MAX_KLOGS - 1], LOG_SIZE, format, args);
    }

    va_end(args);
}

extern "C" void exception_handler(registers_t* regs) {
    // Disable logging during panic to prevent buffer corruption
    klogs_enabled = false;
    terminal.y_offset = 0;
    // Set panic theme (Red background, White text)
    sysCommand("color 4F");
    sysCommand("clear");

    kprintf("===================================================\n");
    kprintf("                   KERNEL PANIC                    \n");
    kprintf("===================================================\n");
    
    // Display basic exception information
    kprintf("Exception: %s (%x)\n", exception_messages[regs->int_no], regs->int_no);
    kprintf("Error Code: %x\n\n", regs->err_code);
    
    // --- CPU REGISTER GRID ---
    kprintf("RAX: %016lx  RBX: %016lx  RCX: %016lx\n", regs->rax, regs->rbx, regs->rcx);
    kprintf("RDX: %016lx  RSI: %016lx  RDI: %016lx\n", regs->rdx, regs->rsi, regs->rdi);
    kprintf("R8 : %016lx  R9 : %016lx  R10: %016lx\n", regs->r8,  regs->r9,  regs->r10);
    kprintf("RBP: %016lx  RSP: %016lx  RIP: %016lx\n", regs->rbp, regs->rsp, regs->rip);
    kprintf("CS : %016lx  RFL: %016lx\n\n", regs->cs, regs->rflags);
    
    // Special handling for Page Faults (Exception 14)
    if (regs->int_no == 14) {
        uint64_t cr2;
        asm volatile("mov %%cr2, %0" : "=r" (cr2));
        kprintf("-> FAULTING ADDRESS (CR2): %016lx <-\n\n", cr2);
    } else kprintf("\n\n");
    
    // Dump klog buffer to the screen for post-mortem analysis
    kprintf("klogs:\n");
    for (size_t i = 0; i < log_index; i++) {
        kprintf("%s\n", klogs[i]);
    }

    kprintf("\nSystem Halted.\n");

    // Enter infinite halt loop
    while(true) {
        asm volatile("cli; hlt");
    }
}