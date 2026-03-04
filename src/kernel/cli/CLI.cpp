#include "CLI.h"





void cli_main() {

    process_keyboard_events();
    
    draw_cursor = true; 

    if (ticks % 500 == 0 && draw_cursor) {
        terminal_toggle_cursor_shape();
        cursor_visible = !cursor_visible; 
    }


    sysCommandAt("check ram av", 54, 0);



}