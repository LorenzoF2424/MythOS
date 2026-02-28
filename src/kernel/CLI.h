#ifndef CLI_H
#define CLI_H
#include "gnu_utils/string.h"
#include "terminal_driver/kprintf.h"
#include "idt/IDT.h"
#define SECOND 100000000
inline uint64_t ticks = 0; 

inline void cli_main() {

    if (ticks % (SECOND/3) == 0 && draw_cursor) {
        terminal_toggle_cursor_shape();
        cursor_visible = !cursor_visible; 
    }  





}

















#endif