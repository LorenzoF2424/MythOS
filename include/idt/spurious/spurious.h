#ifndef SPURIOUS_H
#define SPURIOUS_H

#include <stdint.h>
#include "idt/idt.h"

void irq7_handler();
void irq15_handler();
void init_spurious();

#endif