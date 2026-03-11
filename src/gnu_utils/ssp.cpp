#ifndef STACKSMASHINGPROTECTION_CPP
#define STACKSMASHINGPROTECTION_CPP

#include <stdint.h>
extern void kprintf(const char* str, ...);

// Valori "canarino" di base.
#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif
extern "C" {

    __attribute__((used))
    uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

    
    __attribute__((noreturn, used))
    void __stack_chk_fail(void) {
        
        kprintf("PANIC: STACK COMPROMISED!! Buffer overflow detected.\n");
        
        while(1) {
            asm volatile("cli; hlt");  // Ferma la CPU
        }
    }

}


#endif