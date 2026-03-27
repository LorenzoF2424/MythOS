#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Hardware and locking dependencies
#include "term/displayTerminal.h"
#include "term/printfCore.h"
#include "sched/spinlock.h"
#include "gnu_utils/point.h"

struct terminal_output_t; 
struct point;
// ==========================================
// Terminal Output APIs (Screen)
// ==========================================

// Prints formatted text directly to the active terminal (Thread-Safe)
void kprintf(const char* format, ...);

// Prints formatted text at a specific (x, y) location (Thread-Safe)
void kprintfAt(terminal_output_t *t, point p, const char* format, ...);

// Prints formatted text without acquiring the spinlock (Strictly for Kernel Panics)
void unlocked_kprintf(const char* format, ...);

// ==========================================
// Memory Output APIs (RAM Buffers)
// ==========================================

// Formats a string and stores it safely in a memory buffer
int snprintf(char* buffer, size_t n, const char* format, ...);

// Core memory formatter that accepts a pre-packed va_list (Useful for custom wrappers like klog)
int vsnprintf(char* buffer, size_t n, const char* format, va_list args);

#endif 