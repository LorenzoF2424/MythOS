#include "idt/gdt.h"
#include "idt/exceptions/tss.h"

// 7 slot GDT (TSS occupies 2)
gdt_entry_t gdt[7];
gdt_ptr_t gdt_ptr;


void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_mid    = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_set_tss(int num, uint64_t base, uint32_t limit) {
    tss_descriptor_t* tss_desc = (tss_descriptor_t*)&gdt[num];
    
    tss_desc->limit_low = limit & 0xFFFF;
    tss_desc->base_low = base & 0xFFFF;
    tss_desc->base_mid = (base >> 16) & 0xFF;
    tss_desc->access = 0x89; // Present, Executable, TSS Type
    tss_desc->limit_high = (limit >> 16) & 0x0F;
    tss_desc->base_high_low = (base >> 24) & 0xFF;
    tss_desc->base_high_high = (base >> 32) & 0xFFFFFFFF;
    tss_desc->reserved = 0;
}

void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 7) - 1;
    gdt_ptr.base  = (uint64_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);
    
    // 2. Kernel Code (0x08): Access 0x9A, Granularity 0xAF (L bit for 64-bit)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF);
    
    // 3. Kernel Data (0x10): Access 0x92, Granularity 0xCF
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    
    // 4. User Code (0x18): Access 0xFA
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF);
    
    // 5. User Data (0x20): Access 0xF2
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // 6. TSS Descriptor (0x28) -  slot 5 and 6
    gdt_set_tss(5, (uint64_t)&main_tss, sizeof(main_tss) - 1);

    // Loading the new GDT
    asm volatile("lgdt %0" : : "m"(gdt_ptr));

    // Loading the TSS (offset 0x28 because it's the 5th slot: 5 * 8 = 40 = 0x28)
    asm volatile("ltr %%ax" : : "a"(0x28));
}