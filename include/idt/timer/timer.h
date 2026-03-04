#ifndef TIMER_H
#define TIMER_H

#include "idt/defines.h" 
#include "cmos/io.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


extern uint64_t ticks;
void init_timer(uint32_t frequency);
extern "C" void timer_isr();
extern "C" void timer_handler_c();


#endif