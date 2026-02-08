ENTRY(main)
SECTIONS
{
    . = 0x8400;  /* Kernel sempre a 0x8400 */
    .text : { *(.text) }
    .data : { *(.data) }
    .bss  : { *(.bss) }
}