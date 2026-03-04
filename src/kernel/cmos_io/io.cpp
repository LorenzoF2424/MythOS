#include "io.h"


uint8_t cmos_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

uint32_t get_total_memory_mb() {
    
    uint8_t low_byte = cmos_read(0x34);
    uint8_t high_byte = cmos_read(0x35);
    
    uint16_t blocks_64k = low_byte | (high_byte << 8);
    
    uint32_t total_mb = 16 + ((blocks_64k * 64) / 1024);
    
    return total_mb;
}

void sleep(uint64_t ms) {
    uint64_t target_ticks = ticks + ms;
    
    while (ticks < target_ticks) {
        
        __asm__ __volatile__ ("hlt");
    }
}