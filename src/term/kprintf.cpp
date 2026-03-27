#include "term/kprintf.h"
#include <stdarg.h>

// ==========================================
// SECTION 3: Terminal Output Wrappers (Screen)
// ==========================================

static void kprintf_putc_callback(char c, void* arg) {
    terminal.putchar(c);
}

void kprintf(const char* format, ...) {
    // Early return: Prevent null pointer dereference
    if (!format) return;

    spinlock_acquire(&terminal_lock);
    
    va_list args;
    va_start(args, format);
    printf_core(kprintf_putc_callback, nullptr, format, args);
    va_end(args);
    
    spinlock_release(&terminal_lock);
}

void kprintfAt(terminal_output_t *t, point p, const char* format, ...) {
    // Early Return: Safety check
    if (!format || !t) return;
    
    spinlock_acquire(&terminal_lock);

    // Save current cursor visibility and hide it to prevent flickering
    bool was_visible = cursor_visible;
    remove_cursor_shape();

    // Backup the current global terminal state
    terminal_output_t temp = terminal;

    // Apply the target terminal configuration
    terminal = *t;
    terminal.cursor = p;
    terminal.direct = true;

    va_list args;
    va_start(args, format);

    // Run the core engine (now printing to the redirected terminal at point p)
    printf_core(kprintf_putc_callback, nullptr, format, args);

    va_end(args);

    // Restore the original terminal state
    terminal = temp;
    terminal.direct = false;

    // Restore cursor and its shape
    draw_cursor = true;
    terminal_restore_cursor(was_visible);

    spinlock_release(&terminal_lock);
}

void unlocked_kprintf(const char* format, ...) {
    if (!format) return;
    
    va_list args;
    va_start(args, format);
    printf_core(kprintf_putc_callback, nullptr, format, args);
    va_end(args);
}

// ==========================================
// SECTION 4: Memory Output Wrappers (RAM Buffers)
// ==========================================

struct snprintf_context {
    char* buffer;
    size_t remaining;
    size_t written;
};

static void snprintf_putc_callback(char c, void* arg) {
    snprintf_context* ctx = (snprintf_context*)arg;
    
    // Early return: Prevent buffer overflow
    if (ctx->remaining == 0) return;
    
    *(ctx->buffer)++ = c;
    ctx->remaining--;
    ctx->written++;
}

int vsnprintf(char* buffer, size_t n, const char* format, va_list args) {
    if (!buffer || n == 0 || !format) return 0;
    
    // Initialize context state, reserving 1 byte for null terminator
    snprintf_context ctx = { buffer, n - 1, 0 };
    
    printf_core(snprintf_putc_callback, &ctx, format, args);
    
    // Safety null termination
    *(ctx.buffer) = '\0';
    
    return ctx.written;
}

int snprintf(char* buffer, size_t n, const char* format, ...) {
    if (!format) return 0;

    va_list args;
    va_start(args, format);
    
    int written = vsnprintf(buffer, n, format, args);
    
    va_end(args);
    return written;
}