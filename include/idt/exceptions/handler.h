#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>


extern void init_exceptions();

extern "C" void exception_handler(uint64_t interrupt_number, uint64_t error_code);



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



