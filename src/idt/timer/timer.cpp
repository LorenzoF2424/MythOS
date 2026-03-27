#include "idt/timer/timer.h"

volatile uint64_t ticks = 0;
uint64_t previous_tick = 0; 
uint64_t current_tick = 0; 

void init_timer(uint32_t frequency) {
   
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    
    // 32 = IRQ0 (Timer)
    idt_set_gate(32, (uint64_t)timer_isr, cs, 0x8E);
    
    uint32_t divisor = 1193180 / frequency;

    
    outb(0x43, 0x36);

    
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

extern "C" void timer_handler_c() {
    ticks++; 
    outb(0x20, 0x20); 
}



bool every(uint64_t ms) {      
    
    return (previous_tick / ms) < (current_tick / ms);
}