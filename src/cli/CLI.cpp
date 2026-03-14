#include "cli/CLI.h"





void cli_main() {

    process_keyboard_events();
    
    if (every(250)) {
        sysCommandAt("check ram av", point(54, 0));
        sysCommandAt("time clock", point(MAX_COLUMNS - 8, 0));
        sysCommandAt("time date", point(MAX_COLUMNS - 10, 1));

    }


    


    update_mouse_cursor();


    
    draw_cursor = true; 
    
    if (every(500) && draw_cursor) {
        cursorp = terminal_data.cursor;
        terminal_toggle_cursor_shape();
        cursor_visible = !cursor_visible; 
    }




}