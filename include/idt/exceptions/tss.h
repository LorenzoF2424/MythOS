#ifndef TSS_H
#define TSS_H

#include <stdint.h>

struct tss_entry_struct {
    uint32_t reserved0;
    uint64_t rsp0;      
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];    
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));

typedef struct tss_entry_struct tss_t;
extern tss_t main_tss;
// Funzioni per inizializzare e caricare la TSS
void init_tss();
extern "C" void flush_tss(); // Questa sarà in assembly

#endif