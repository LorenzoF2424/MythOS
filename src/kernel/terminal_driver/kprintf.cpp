#include "kprintf.h"

// -----------------decimals Numbers-----------------
void print_int(int num) {
    if (num == 0) { terminal_putchar('0'); return; }
    if (num < 0) { terminal_putchar('-'); num = -num; }

    char buffer[32];
    int8_t i = 0;
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (--i >= 0) terminal_putchar(buffer[i]);
}

void print_long(long num) {
    if (num == 0) { terminal_putchar('0'); return; }
    if (num < 0) { terminal_putchar('-'); num = -num; }

    char buffer[32];
    int i = 0;
    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    while (--i >= 0) terminal_putchar(buffer[i]);
}

void print_float(double num) {
    // Gestione dei numeri negativi
    if (num < 0) {
        terminal_putchar('-');
        num = -num;
    }

    // Estrai e stampa la parte intera riutilizzando print_long
    long int_part = (long)num;
    print_long(int_part);

    // Stampa la virgola
    terminal_putchar('.');

    // Calcola la parte decimale (es. 3.1415 - 3 = 0.1415)
    double frac_part = num - (double)int_part;

    // Estrai le cifre decimali una ad una (Precisione a 4 cifre)
    int precision = 4;
    for (int i = 0; i < precision; i++) {
        frac_part *= 10;
        long digit = (long)frac_part;
        terminal_putchar('0' + digit); // Stampa il singolo numero
        frac_part -= digit;            // Rimuovi la cifra appena stampata
    }
}

// ----------------- hexadecimals Numbers -----------------
void print_hex32(uint32_t num) {
    terminal_write("0x");
    if (num == 0) { terminal_putchar('0'); return; }
    char buffer[32]; int i = 0;
    while (num > 0) {
        int rem = num % 16;
        buffer[i++] = (rem < 10) ? ('0'+rem) : ('A'+rem-10);
        num /= 16;
    }
    while (--i >= 0) terminal_putchar(buffer[i]);
}

void print_hex64(uint64_t num) {
    terminal_write("0x");
    if (num == 0) { terminal_putchar('0'); return; }
    char buffer[32]; int i = 0;
    while (num > 0) {
        int rem = num % 16;
        buffer[i++] = (rem < 10) ? ('0'+rem) : ('A'+rem-10);
        num /= 16;
    }
    while (--i >= 0) terminal_putchar(buffer[i]);
}

void print_ptr(uint64_t ptr) {
    terminal_write("0x");
    char buffer[16];
    const char* hex_chars = "0123456789ABCDEF";

    // Riempiamo il buffer partendo dalla fine per avere sempre 16 cifre
    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[ptr & 0xF];
        ptr >>= 4;
    }

    // Stampiamo tutto il buffer
    for (int i = 0; i < 16; i++) {
        terminal_putchar(buffer[i]);
    }
}

// ----------------- kprintf -----------------
void kprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%') {
            i++;
            switch (format[i]) {
                case 'p': { uint64_t ptr = (uint64_t)va_arg(args, void*); print_ptr((uint64_t)ptr); break; }
                case 'd': { int num = va_arg(args,int); print_int(num); break; }
                case 'x': { uint32_t num = va_arg(args,uint32_t); print_hex32(num); break; }
                case 's': { const char* str = va_arg(args,const char*); terminal_write(str); break; }
                case 'c': { char c = (char)va_arg(args,int); terminal_putchar(c); break; }

                case 'f': { double num = va_arg(args, double); print_float(num); break; }
                case 'l': { // long o long hex
                    i++;
                    switch(format[i]) {
                        case 'd': { long num = va_arg(args,long); print_long(num); break; }
                        case 'x': { uint64_t num = va_arg(args,uint64_t); print_hex64(num); break; }
                        default: terminal_write("PRINT_ERR"); terminal_putchar(format[i]); break;
                    }
                    break;
                }
                case '%': { terminal_putchar('%'); break; }
                default: { terminal_write("PRINT_ERR"); terminal_putchar(format[i]); break; }
            }
        } else {
            terminal_putchar(format[i]);
        }
    }

    va_end(args);
}