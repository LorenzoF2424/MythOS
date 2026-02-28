#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>
#include <stdint.h>
#include "displayTerminal.h"

void print_int(int num);
void print_long(long num);

void print_hex32(uint32_t num);
void print_hex64(uint64_t num);

void kprintf(const char* format, ...);

#endif