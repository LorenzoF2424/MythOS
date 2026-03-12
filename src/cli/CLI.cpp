#include "cli/CLI.h"





void cli_main() {

    process_keyboard_events();
    
    if (every(250)) sysCommandAt("check ram av", 54, 0);








    
    draw_cursor = true; 

    if (every(500) && draw_cursor) {
        terminal_toggle_cursor_shape();
        cursor_visible = !cursor_visible; 
    }




}