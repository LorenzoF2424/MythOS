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
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xAF); // 0x08 kernel code
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // 0x10 kernel data
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xAF); // 0x18 user code
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // 0x20 user data
    gdt_set_tss(5, (uint64_t)&main_tss, sizeof(main_tss) - 1); // 0x28 TSS

    asm volatile("lgdt %0" : : "m"(gdt_ptr));

    asm volatile(
        "pushq $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "pushq %%rax\n"
        "lretq\n"
        "1:\n"
        ::: "rax"
    );

    asm volatile(
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        ::: "ax"
    );

    asm volatile("ltr %%ax" : : "a"(0x28));
}