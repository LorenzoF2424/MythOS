#ifndef CLI_H
#define CLI_H
#include "string.h"
#include "kprintf.h"
#include "IDT.h"
#define SECOND 100000000
uint64_t ticks = 0; 

void cli_main() {

    if (ticks % (SECOND/3) == 0 && draw_cursor) {
        terminal_toggle_cursor_shape();
        cursor_visible = !cursor_visible; 
    }  





}

















#endif