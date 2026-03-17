#include "term/kprintf.h"
#include <stdarg.h>



void print_number(uint64_t num, int base, bool is_signed, int padding, char pad_char) {
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
        terminal_putchar(buffer[i]);
    }
}




void print_float(double num) {
    if (num < 0) {
        terminal_putchar('-');
        num = -num;
    }

    // Parte intera
    uint64_t int_part = (uint64_t)num;
    print_number(int_part, 10, false, 0, ' ');

    terminal_putchar('.');

    // Parte decimale (Precisione 4 cifre fissa)
    double frac_part = num - (double)int_part;
    for (int i = 0; i < 4; i++) {
        frac_part *= 10;
        uint64_t digit = (uint64_t)frac_part;
        terminal_putchar('0' + digit);
        frac_part -= digit;
    }
}

void print_ptr(uint64_t ptr) {
    terminal_write("0x");
    print_number(ptr, 16, false, 16, '0');
}

void unlocked_kprintf(const char* format, ...) {



    
}

void kprintf(const char* format, ...) {

    spinlock_acquire(&terminal_lock);
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++;
            
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
                    print_number((uint64_t)num, 10, true, padding, pad_char); 
                    break; 
                }
                case 'x': { 
                    uint32_t num = va_arg(args, uint32_t); 
                    terminal_write("0x");
                    print_number((uint64_t)num, 16, false, padding, pad_char); 
                    break; 
                }
                case 'p': { 
                    uint64_t ptr = (uint64_t)va_arg(args, void*); 
                    print_ptr(ptr); 
                    break; 
                }
                case 's': { 
                    const char* str = va_arg(args, const char*); 
                    terminal_write(str ? str : "(null)"); 
                    break; 
                }
                case 'c': { 
                    char c = (char)va_arg(args, int); 
                    terminal_putchar(c); 
                    break; 
                }
                case 'f': { 
                    double num = va_arg(args, double); 
                    print_float(num); 
                    break; 
                }
                case 'l': { 
                    i++;
                    switch(format[i]) {
                        case 'd': { 
                            int64_t num = va_arg(args, long); 
                            print_number((uint64_t)num, 10, true, padding, pad_char); 
                            break; 
                        }
                        case 'x': { 
                            uint64_t num = va_arg(args, uint64_t); 
                            terminal_write("0x");
                            print_number(num, 16, false, padding, pad_char); 
                            break; 
                        }
                        default: 
                            terminal_write("PRINT_ERR_L"); 
                            terminal_putchar(format[i]); 
                            break;
                    }
                    break;
                }
                case '%': { 
                    terminal_putchar('%'); 
                    break; 
                }
                default: { 
                    terminal_write("PRINT_ERR"); 
                    terminal_putchar(format[i]); 
                    break; 
                }
            }
        } else {
            terminal_putchar(format[i]);
        }
    }

    va_end(args);

    spinlock_release(&terminal_lock);
}



