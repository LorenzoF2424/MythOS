#include "kernel/cpu/cpu.h"

// ==========================================
// CPU Initialization Implementation
// ==========================================

void enable_fpu() {
    uint64_t cr0, cr4;

    // 1. Configure Control Register 0 (CR0)
    // Clear the EM (Emulation) flag (bit 2) - tells CPU not to emulate math
    // Set the MP (Monitor Co-processor) flag (bit 1)
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); 
    cr0 |= (1 << 1); 
    asm volatile ("mov %0, %%cr0" :: "r"(cr0));

    // 2. Configure Control Register 4 (CR4)
    // Set OSFXSR (bit 9) to enable SSE instructions and FXSAVE/FXRSTOR
    // Set OSXMMEXCPT (bit 10) to enable Unmasked SIMD Floating-Point Exceptions
    asm volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 9);  
    cr4 |= (1 << 10); 
    asm volatile ("mov %0, %%cr4" :: "r"(cr4));

    // 3. Initialize the FPU state
    asm volatile ("fninit");
}