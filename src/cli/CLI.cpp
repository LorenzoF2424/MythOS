#include "cli/CLI.h"
#include "gnu_utils/string.h"

void draw_info_bar();



uint16_t info_bar_refresh_rate = 250;
void cli_main() {

    process_keyboard_events();
    process_mouse_scroll();

    

    if (draw_info && every(info_bar_refresh_rate))
        draw_info_bar();
    
    if (!draw_info) terminal.y_offset=0;














        
    update_mouse_cursor();

    draw_cursor = true; 
    
    if (every(500)) terminal_cursor_update();
    



 
}



void draw_info_bar() {

    klogs_enabled = false;
    info_bar_window.color.bg=terminal.color.bg;
    char info_bar_border[MAX_COLUMNS+1]; 
    
    for (size_t i=0;i<info_bar_window.max.x;i++) {
        info_bar_border[i]='_';
    }
    info_bar_border[MAX_COLUMNS]='\0';
    
   
        
    info_bar_window.max.x=terminal.max.x;
    if (every(4000)) {
        uint32_t info_bar_height_pixels = 4 * 16 * info_bar_window.scale;
        draw_rect(0, 0, screen_width, info_bar_height_pixels, info_bar_window.color.bg);
    }
       
    info_bar_window.cursor=point(0,1);
    info_bar_window.writeAt("Av   RAM: ", point(0, 1));
    sysCommandAt(&info_bar_window, "check ram av kb", point(11, 1));
    info_bar_window.writeAt("/", point(25, 1));
    sysCommandAt(&info_bar_window, "check ram av b", point(27, 1));

    info_bar_window.writeAt("Used RAM: ", point(0, 2));
    sysCommandAt(&info_bar_window, "check ram us kb", point(11, 2));
    info_bar_window.writeAt("/", point(25, 2));
    sysCommandAt(&info_bar_window, "check ram us b", point(27, 2));
        
    sysCommandAt(&info_bar_window, "time clock", point(info_bar_window.max.x - 8, 1));
    sysCommandAt(&info_bar_window, "time date", point(info_bar_window.max.x - 11, 2));
    
    

    kprintfAt(&info_bar_window, point(45,1),"%s",klogs[MAX_KLOGS-2]);
    

    kprintfAt(&info_bar_window, point(45,2), "%s",klogs[MAX_KLOGS-1]);
            
            
    info_bar_window.writeAt(info_bar_border, point(0, 3));


    klogs_enabled = true;
}


void info_bar_thread() { 
    
    info_bar_window = terminal;
    info_bar_window.direct = true;
    
    char info_bar_border[MAX_COLUMNS+1]; 
    
    for (size_t i=0;i<info_bar_window.max.x;i++) {
        info_bar_border[i]='_';
    }
    info_bar_border[MAX_COLUMNS]='\0';
    
    while (true) { if (draw_info) {
        
        info_bar_window.max.x=terminal.max.x;
        if (draw_info_was_disabled) {
            uint32_t total_pixels = 3 * 16 * pitch_pixels;
            for (uint32_t i = 0; i < total_pixels; i++) {
            framebuffer[i] = info_bar_window.color.bg;
            }
            draw_info_was_disabled = false;
        }
       
        info_bar_window.cursor=point(0,1);
        info_bar_window.writeAt("Available RAM: ", point(0, 1));
        sysCommandAt(&info_bar_window,"check ram av kb", point(16, 1));
        info_bar_window.writeAt("/", point(30, 1));
        sysCommandAt(&info_bar_window,"check ram av b", point(32, 1));

        info_bar_window.writeAt("Used RAM: ", point(0, 2));
        sysCommandAt(&info_bar_window,"check ram us kb", point(16, 2));
        info_bar_window.writeAt("/", point(30, 2));
        sysCommandAt(&info_bar_window,"check ram us b", point(32, 2));
        
        sysCommandAt(&info_bar_window,"time clock", point(info_bar_window.max.x - 8, 1));
        sysCommandAt(&info_bar_window,"time date", point(info_bar_window.max.x - 11, 2));
            
            
        info_bar_window.writeAt(info_bar_border, point(0, 3));

        
    }thread_sleep(info_bar_refresh_rate);}
}