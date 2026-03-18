#include "cli/CLI.h"
#include "gnu_utils/string.h"





uint16_t info_bar_refresh_rate = 250;
void cli_main() {

    process_keyboard_events();
    

    
    


    update_mouse_cursor();


    
    draw_cursor = true; 
    
    if (every(500)) terminal_cursor_update();
    



 
}






void info_bar_thread() { 
    
    char info_bar_border[MAX_COLUMNS+1]; 
    
    for (size_t i=0;i<MAX_COLUMNS;i++) {
        info_bar_border[i]='_';
    }
    info_bar_border[MAX_COLUMNS]='\0';
    
    while (true) { if (draw_info) {
        
        if (draw_info_was_disabled) {
            uint32_t total_pixels = 3 * 16 * pitch_pixels;
            for (uint32_t i = 0; i < total_pixels; i++) {
            framebuffer[i] = terminal_data.color.bg;
            }
            draw_info_was_disabled = false;
        }
       
        
        terminal_write_at("Available RAM: ", point(0, 1));
        sysCommandAt("check ram av kb", point(16, 1));
        terminal_write_at("/", point(30, 1));
        sysCommandAt("check ram av b", point(32, 1));

        terminal_write_at("Used RAM: ", point(0, 2));
        sysCommandAt("check ram us kb", point(16, 2));
        terminal_write_at("/", point(30, 2));
        sysCommandAt("check ram us b", point(32, 2));
        
        sysCommandAt("time clock", point(MAX_COLUMNS - 8, 1));
        sysCommandAt("time date", point(MAX_COLUMNS - 11, 2));
            
            
        terminal_write_at(info_bar_border, point(0, 3));

        
    }thread_sleep(info_bar_refresh_rate);}
}