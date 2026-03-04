#ifndef IDT_H
#define IDT_H



#include "idt/defines.h"


#include "../cli/cmd/commands.h"
#include "../mem/pmm.h"



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


void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags);
void init_idt();
void remap_pic();
extern "C" void idt_flush(uint64_t ptr_addr);
void kbd_push(uint8_t scancode);
bool kbd_pop(uint8_t *scancode);




#endif