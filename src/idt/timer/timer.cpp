#include "idt/timer/timer.h"

uint64_t ticks = 0; 

void init_timer(uint32_t frequency) {
    
    uint32_t divisor = 1193180 / frequency;

    
    outb(0x43, 0x36);

    
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

extern "C" void timer_handler_c() {
    ticks++; 
    outb(0x20, 0x20); 
}