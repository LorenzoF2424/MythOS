#include "term/printfCore.h" 

// ==========================================
// Internal Formatting Helpers (Hidden from other files)
// ==========================================

static void core_print_number(printf_putc_t putc_cb, void* arg, uint64_t num, int base, bool is_signed, int padding, char pad_char) {
    char buffer[64]; 
    int i = 0;
    bool is_negative = false;

    if (is_signed && base == 10 && (int64_t)num < 0) {
        is_negative = true;
        num = (uint64_t)(-(int64_t)num);
    }

    if (num == 0) {
        buffer[i++] = '0';
    }

    while (num > 0) {
        int rem = num % base;
        buffer[i++] = (rem < 10) ? ('0' + rem) : ('A' + rem - 10);
        num /= base;
    }

    while (i < padding) {
        buffer[i++] = pad_char;
    }

    if (is_negative) {
        buffer[i++] = '-';
    }

    while (--i >= 0) {
        putc_cb(buffer[i], arg); 
    }
}

static void core_print_float(printf_putc_t putc_cb, void* arg, double num) {
    if (num < 0) {
        putc_cb('-', arg);
        num = -num;
    }

    uint64_t int_part = (uint64_t)num;
    core_print_number(putc_cb, arg, int_part, 10, false, 0, ' ');

    putc_cb('.', arg);

    double frac_part = num - (double)int_part;
    for (int i = 0; i < 4; i++) {
        frac_part *= 10;
        uint64_t digit = (uint64_t)frac_part;
        putc_cb('0' + digit, arg);
        frac_part -= digit;
    }
}

// ==========================================
// Main Printf Logic Engine
// ==========================================
void printf_core(printf_putc_t putc_cb, void* arg, const char* format, va_list args) {
    // Early Return: Safety check for null pointer
    if (!format) return;

    for (int i = 0; format[i] != '\0'; i++) {
        
        // Early Return (Continue pattern): Standard character
        if (format[i] != '%') {
            putc_cb(format[i], arg);
            continue; 
        }

        i++; 
        
        // Early Return (Break pattern): String ends unexpectedly
        if (format[i] == '\0') break;

        int padding = 0;
        char pad_char = ' ';
        
        if (format[i] == '0') {
            pad_char = '0';
            i++;
        }
        
        while (format[i] >= '0' && format[i] <= '9') {
            padding = (padding * 10) + (format[i] - '0');
            i++;
        }

        switch (format[i]) {
            case 'd': { 
                int64_t num = va_arg(args, int); 
                core_print_number(putc_cb, arg, (uint64_t)num, 10, true, padding, pad_char); 
                break; 
            }
            case 'x': { 
                uint32_t num = va_arg(args, uint32_t); 
                putc_cb('0', arg); putc_cb('x', arg);
                core_print_number(putc_cb, arg, (uint64_t)num, 16, false, padding, pad_char); 
                break; 
            }
            case 'p': { 
                // Look ahead to see if the user wants our custom [address|value] format
                char next_char = format[i + 1];
                
                // If it's a known custom sub-specifier ('d' for int, 'f' for float, 'x' for hex)
                if (next_char == 'd' || next_char == 'f' || next_char == 'x') {
                    i++; // Consume the sub-specifier character
                    
                    void* raw_ptr = va_arg(args, void*);
                    
                    // Early Return equivalent (Break): Prevent fatal Page Faults!
                    if (!raw_ptr) {
                        const char* null_msg = "[NULL|NULL]";
                        while (*null_msg) putc_cb(*null_msg++, arg);
                        break;
                    }
                    
                    // 1. Print Address part: [0x...|
                    putc_cb('[', arg);
                    putc_cb('0', arg); putc_cb('x', arg);
                    core_print_number(putc_cb, arg, (uint64_t)raw_ptr, 16, false, 8, '0');
                    putc_cb(' ', arg);
                    putc_cb('|', arg);
                    putc_cb(' ', arg);
                    
                    // 2. Safely cast and dereference based on the requested type
                    if (next_char == 'd') {
                        // Assume pointer to 32-bit signed integer
                        int32_t val = *(int32_t*)raw_ptr;
                        core_print_number(putc_cb, arg, (uint64_t)val, 10, true, 0, ' ');
                    } 
                    else if (next_char == 'x') {
                        // Assume pointer to 32-bit unsigned integer (hex)
                        uint32_t val = *(uint32_t*)raw_ptr;
                        putc_cb('0', arg); putc_cb('x', arg);
                        core_print_number(putc_cb, arg, (uint64_t)val, 16, false, 0, ' ');
                    }
                    else if (next_char == 'f') {
                        // Assume pointer to double (floats are promoted to double in va_args, 
                        // but here we are casting the pointer directly)
                        double val = *(double*)raw_ptr;
                        core_print_float(putc_cb, arg, val);
                    }
                    
                    // 3. Close the bracket
                    putc_cb(']', arg);
                    
                } else {
                    // Standard %p behavior (just the address)
                    uint64_t ptr = (uint64_t)va_arg(args, void*); 
                    putc_cb('0', arg); putc_cb('x', arg);
                    core_print_number(putc_cb, arg, ptr, 16, false, 16, '0'); 
                }
                break; 
            }
            case 's': { 
                const char* str = va_arg(args, const char*); 
                if (!str) str = "(null)";
                while (*str) putc_cb(*str++, arg); 
                break; 
            }
            case 'c': { 
                char c = (char)va_arg(args, int); 
                putc_cb(c, arg); 
                break; 
            }
            case 'f': { 
                double num = va_arg(args, double); 
                core_print_float(putc_cb, arg, num); 
                break; 
            }
            case 'l': { 
                i++; 
                switch(format[i]) {
                    case 'd': { 
                        int64_t num = va_arg(args, long); 
                        core_print_number(putc_cb, arg, (uint64_t)num, 10, true, padding, pad_char); 
                        break; 
                    }
                    case 'x': { 
                        uint64_t num = va_arg(args, uint64_t); 
                        putc_cb('0', arg); putc_cb('x', arg);
                        core_print_number(putc_cb, arg, num, 16, false, padding, pad_char); 
                        break; 
                    }
                    default: 
                        putc_cb('E', arg); putc_cb('R', arg); putc_cb('R', arg);
                        putc_cb(format[i], arg); 
                        break;
                }
                break;
            }
            case '%': { 
                putc_cb('%', arg); 
                break; 
            }
            default: { 
                putc_cb('%', arg); 
                putc_cb(format[i], arg); 
                break; 
            }
        }
    }
}