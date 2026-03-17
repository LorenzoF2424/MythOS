#include "idt/exceptions/tss.h"






static uint8_t double_fault_stack[4096]; 
tss_t main_tss;

void init_tss() {
    
    for(int i = 0; i < sizeof(tss_t); i++) ((uint8_t*)&main_tss)[i] = 0;

    
    main_tss.ist[0] = (uint64_t)double_fault_stack + 4096;
}