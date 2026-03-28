#ifndef PRINTF_CORE_H
#define PRINTF_CORE_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "gnu_utils/stdintex.h"

// ==========================================
// Printf Core Engine
// ==========================================

// Defines the signature for the character-output callback
typedef void (*printf_putc_t)(char c, void* arg);

// Core rendering engine that processes the format string and routes characters to the callback
void printf_core(printf_putc_t putc_cb, void* arg, const char* format, va_list args);

#endif 