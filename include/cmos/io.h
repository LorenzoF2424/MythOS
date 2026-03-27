#ifndef IO_H
#define IO_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gnu_utils/string.h"
#include "term/kprintf.h"

extern volatile uint64_t ticks;
extern "C" void outb(uint16_t port, uint8_t value);
extern "C" uint8_t inb(uint16_t port);
static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

uint8_t cmos_read(uint8_t reg);
uint32_t get_total_memory_mb();
void sleep(uint64_t ms);



#endif