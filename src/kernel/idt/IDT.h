#ifndef IDT_H
#define IDT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "io.h"
#include "sound.h"


#include "../gnu_utils/string.h"
#include "../terminal_driver/kprintf.h"


// --- COSTANTI PIC ---
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// --- COSTANTI SHELL ---
#define MAX_HISTORY 10
#define MAX_COMMAND_ARGS 16

// --- STRUTTURE IDT ---
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

// --- VARIABILI ESTERNE (Definite nel .cpp) ---
extern idt_entry_t idt[256];
extern idt_ptr_t idt_ptr;
extern char history[MAX_HISTORY][64000]; // Usando MAX_INPUT_LEN
extern int8_t history_count;
extern int8_t history_index;
extern bool shift_pressed;
extern bool altgr_pressed;
extern bool extended_scancode;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
void init_idt();
void remap_pic();
extern "C" void idt_flush(uint64_t ptr_addr);


void init_keyboard();
extern "C" void keyboard_isr();
extern "C" void keyboard_handler_c();
void handle_character_input(char c);


void prepare_for_next_command();
void sysCommand(const char *command);
void sysCommandAt(const char *command, uint16_t x, uint16_t y);
int8_t execute_command(const char c[MAX_COMMAND_ARGS][256]);
void command_handler();
void refresh_command_line();



#endif