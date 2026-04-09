#ifndef IDT_H
#define IDT_H



#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gnu_utils/string.h"
#include "term/kprintf.h"
#include "cmos/io.h"


#include "idt/exceptions/handler.h"

#include "mem/pmm.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define MAX_HISTORY 10
#define MAX_ARGC 16
#define MAX_ARG_LEN 256
#define MAX_COMMAND_LEN (MAX_ARGC * MAX_ARG_LEN)
#define KBD_BUFFER_SIZE 256

struct idt_entry_t {
    uint16_t base_low;      
    uint16_t sel;           
    uint8_t  ist;           
    uint8_t  flags;         
    uint16_t base_mid;      
    uint32_t base_high;     
    uint32_t always0;       
} __attribute__((packed));

struct idt_ptr_t {
    uint16_t limit;
    uint64_t base;          
} __attribute__((packed));


extern idt_entry_t idt[256];
extern idt_ptr_t idt_ptr;
extern "C" void idt_flush(uint64_t ptr_addr);

void remap_pic();
void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
void init_idt();
void kbd_push(uint8_t scancode);
bool kbd_pop(uint8_t *scancode);




#endif