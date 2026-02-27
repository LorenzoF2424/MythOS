#include <stddef.h>
#include <stdint.h>
#include "io.h"

void play_sound(uint32_t nFrequence) {
    if (nFrequence == 0) return;
    uint32_t Div = 1193180 / nFrequence;
    
    
    outb(0x43, 0xB6); 
    outb(0x42, (uint8_t) (Div & 0xFF));
    outb(0x42, (uint8_t) ((Div >> 8) & 0xFF));

    
    uint8_t state = inb(0x61);
    outb(0x61, state | 3); 
}

void nosound() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}