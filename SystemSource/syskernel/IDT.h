#ifndef IDT_H
#define IDT_H
#include "string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "io.h"
#include "kprintf.h"
#include "sound.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define MAX_COMMAND_LEN 256
#define MAX_HISTORY 10
#define MAX_COMMAND_ARGS 16

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

struct idt_entry_t idt[256];
struct idt_ptr_t idt_ptr;



void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {

    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_mid  = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].base_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    
    idt[num].sel = sel;
    idt[num].ist = 0;
    idt[num].flags = flags | 0x60; 
    idt[num].always0 = 0;
}

extern "C" void idt_flush(uint64_t ptr_addr);

void init_idt() {
    idt_ptr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint64_t)&idt;

   
    uint8_t *idt_raw = (uint8_t*)&idt;
    for (size_t i = 0; i < sizeof(struct idt_entry_t) * 256; i++) {
        idt_raw[i] = 0;
    }

    
    idt_flush((uint64_t)&idt_ptr);
}

void remap_pic() {
    
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    
    outb(PIC1_DATA, 0x20); 
    outb(PIC2_DATA, 0x28); 

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

  
    outb(PIC1_DATA, 0xFD); 
    outb(PIC2_DATA, 0xFF);
}

char command_buffer[MAX_COMMAND_LEN];
uint8_t command_len = 0; 
void prepare_for_next_command() {
    command_len = 0;             
    kprintf("MythicOS> ");        
    terminal_set_input_limit();  
}

bool commandNotFound=false;
void execute_command(const char c[MAX_COMMAND_ARGS][256]) {
    
    if (strcmp(c[0], "clear") == 0) {
        terminal_clear();
        return;
    } 

    if (strcmp(c[0], "echo") == 0) {
        kprintf("%s\n", command_buffer+5);
        return;
    }

    

    if (strcmp(c[0], "mem") == 0) {
        kprintf("System Memory:\nTotal RAM: %d MB(via CMOS)\n", get_total_memory_mb());
        return;
    }

    if (strcmp(c[0], "beep") == 0) {
        play_sound(750); 
      
        kprintf("Beep activated! type 'nosound' to stop it.\n");
        return;
    }   

    if (strcmp(c[0], "nosound") == 0) {
        nosound();
        return;
    }

    //-----------------------DEFAULT(NOT FOUND)-----------------------------------------
    kprintf("%s: command not found\n", c[0]);
        
}

char history[MAX_HISTORY][MAX_COMMAND_LEN];
int8_t history_count = 0;   
int8_t history_index = -1;  
void command_handler() {

    command_buffer[command_len] = '\0'; 
    //----------------------COMMANDS THAT DO NOT REQUIRE THIS FUNCTION-------------------

    if (command_buffer[0]=='\0') return;
    if (strcmp(command_buffer, "reboot\0") == 0) {
        kprintf("Restarting...\n");
        outb(0x64, 0xFE); 
    }
    //----------------------------------COMMAND HISTORY----------------------------------

    if (command_len > 0) {
        for (int i = MAX_HISTORY - 1; i > 0; i--) {
            for (int j = 0; j < MAX_COMMAND_LEN; j++) {
                history[i][j] = history[i-1][j];
            }
        }
        for (int j = 0; j < MAX_COMMAND_LEN; j++) {
            history[0][j] = command_buffer[j];
            if (command_buffer[j] == '\0') break;
        }
        
        if (history_count < MAX_HISTORY) history_count++;
    }
    
    history_index = -1; 
    char command_buffer_temp[MAX_COMMAND_LEN];
    strcpy(command_buffer_temp,command_buffer);
    uint8_t arg_count = 0;
    char args[MAX_COMMAND_ARGS][256];
    //--------------------------------FINDING COMMAND----------------------------------

    for(int a=0; a < MAX_COMMAND_ARGS; a++) args[a][0] = '\0';
    {   
        uint8_t i=0;
        char* token=strtok(command_buffer_temp,' ');
        
        while (token != NULL) {
            if (i>=MAX_COMMAND_ARGS) {
                kprintf("Maximum number of arguments for command exceeded.\n");
                prepare_for_next_command();
                return;
            }
            strcpy(args[i++],token);
            token=strtok(NULL,' ');
            arg_count++;
            
        }
        
    }

    
    //--------------------COMMAND EXECUTION---------------------------------------------

    execute_command(args);
}

void refresh_command_line() {
    while (command_len > 0) {
        terminal_putchar('\b');
        command_len--;
    }

    if (history_index == -1) {
        return;
    }

    for (int i = 0; history[history_index][i] != '\0'; i++) {
        terminal_putchar(command_buffer[command_len++] = history[history_index][i]);
    }
}


bool shift_pressed = false;
bool altgr_pressed = false;
bool extended_scancode = false; 
extern "C" void keyboard_handler_c() {
    uint8_t scancode = inb(0x60);

    if (scancode == 0xE0) {
        extended_scancode = true;
        outb(0x20, 0x20);
        return; 
    }

   if (extended_scancode) {
        extended_scancode = false;

        
        if (scancode == 0x48) {
            if (history_count > 0 && history_index < history_count - 1) {
                history_index++;
                refresh_command_line();
            }
        }
        
        if (scancode == 0x50) {
            if (history_index >= 0) {
                history_index--;
                refresh_command_line();
            }
        }
        
        if ((scancode & 0x7F) == 0x38) {
            altgr_pressed = !(scancode & 0x80);
        }
        
        outb(0x20, 0x20);
        return;
    }

    if ((scancode & 0x7F) == 0x2A || (scancode & 0x7F) == 0x36) {
        shift_pressed = !(scancode & 0x80);
        outb(0x20, 0x20);
        return;
    }

    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    

    unsigned char ascii = 0;
    if (altgr_pressed) {
        switch (scancode) {
            case 0x1A: ascii = shift_pressed ? '{' : '['; break; 
            case 0x1B: ascii = shift_pressed ? '}' : ']'; break; 
            case 0x27: ascii = shift_pressed ? (unsigned char)135 : '@'; break; 
            case 0x2B: ascii = shift_pressed ? (unsigned char)248 : '#'; break; 
        }
    } else ascii = shift_pressed ? kbd_IT_shift[scancode] : kbd_IT[scancode];
    
    if (ascii==0) {
        outb(0x20, 0x20);
        return;
    }

    terminal_putchar(ascii);

    if (ascii=='\n') { 
        command_handler();
        prepare_for_next_command();
        outb(0x20, 0x20);
        return;
    }

    if (ascii=='\b') {
        if (command_len > 0)
            command_len--;
        outb(0x20, 0x20);
        return;
    }

    if (command_len<(MAX_COMMAND_LEN-1)) 
        command_buffer[command_len++] = ascii;
    
        
    

    outb(0x20, 0x20);
}

extern "C" void keyboard_isr();

void init_keyboard() {
    remap_pic();
    init_idt(); 
    idt_set_gate(33, (uint64_t)keyboard_isr, 0x08, 0x8E);
    asm volatile ("sti");
}

#endif