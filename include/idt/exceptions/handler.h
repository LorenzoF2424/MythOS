#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

#define LOG_SIZE 256
#define MAX_KLOGS 16

struct registers_t {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((packed));

extern char klogs[MAX_KLOGS][LOG_SIZE];
extern uint8_t log_index;
extern bool klogs_enabled;

extern void init_exceptions();

void klog(const char* format, ...);

extern "C" void exception_handler(registers_t* regs);



extern "C" void isr0();  // Division by zero
extern "C" void isr1();  // Debug
extern "C" void isr2();  // Non-maskable interrupt
extern "C" void isr3();  // Breakpoint
extern "C" void isr4();  // Overflow
extern "C" void isr5();  // Bound range exceeded
extern "C" void isr6();  // Invalid opcode
extern "C" void isr7();  // Device not available
extern "C" void isr8();  // Double fault
extern "C" void isr9();  // Coprocessor segment overrun
extern "C" void isr10(); // Invalid TSS
extern "C" void isr11(); // Segment not present
extern "C" void isr12(); // Stack-segment fault
extern "C" void isr13(); // General protection fault
extern "C" void isr14(); // Page fault
extern "C" void isr15(); // Reserved
extern "C" void isr16(); // x87 floating-point exception
extern "C" void isr17(); // Alignment check
extern "C" void isr18(); // Machine check
extern "C" void isr19(); // SIMD floating-point exception
extern "C" void isr20(); // Virtualization exception
extern "C" void isr21(); // Control Protection Exception
extern "C" void isr22(); // Reserved
extern "C" void isr23(); // Reserved
extern "C" void isr24(); // Reserved
extern "C" void isr25(); // Reserved
extern "C" void isr26(); // Reserved
extern "C" void isr27(); // Reserved
extern "C" void isr28(); // Hypervisor Injection Exception
extern "C" void isr29(); // VMM Communication Exception
extern "C" void isr30(); // Security Exception
extern "C" void isr31(); // Reserved

#endif



