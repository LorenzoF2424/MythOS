#include "idt/mouse/displayMouse.h"


uint32_t mouse_bg_buffer[16 * 16]; 
point old_mouse(-1, -1);       
bool first_mouse_draw = true;


bool was_mouse_active = true;
void update_mouse_cursor() {

    if (!mouse_active && !was_mouse_active) return;
    if (!mouse_active && was_mouse_active) {
        
        if (!first_mouse_draw) {
            for (int y = 0; y < 16; y++) {
                for (int x = 0; x < 16; x++) {
                    int draw_x = old_mouse.x + x;
                    int draw_y = old_mouse.y + y;
                    
                    if (draw_x >= 0 && draw_x < screen_width && draw_y >= 0 && draw_y < screen_height) {
                        framebuffer[draw_y * pitch_pixels + draw_x] = mouse_bg_buffer[y * 16 + x];
                    }
                }
            }
        }

        
        was_mouse_active = false; 



        return;
    
    }

    was_mouse_active = true;
    if (mouse == old_mouse && !first_mouse_draw) {
        return; 
    }

   
    if (!first_mouse_draw) {
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                int draw_x = old_mouse.x + x;
                int draw_y = old_mouse.y + y;
                
                if (draw_x >= 0 && draw_x < screen_width && draw_y >= 0 && draw_y < screen_height) {
                    uint32_t bg_color = mouse_bg_buffer[y * 16 + x];
                    framebuffer[draw_y * pitch_pixels + draw_x] = bg_color;
                }
            }
        }
    }

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int draw_x = mouse.x + x;
            int draw_y = mouse.y + y;
            
            if (draw_x >= 0 && draw_x < screen_width && draw_y >= 0 && draw_y < screen_height) {
                mouse_bg_buffer[y * 16 + x] = framebuffer[draw_y * pitch_pixels + draw_x];
                
                if (mouse_bitmap[y][x] == 1) {
                    framebuffer[draw_y * pitch_pixels + draw_x] = 0xFFFFFF; 
                } else if (mouse_bitmap[y][x] == 2) {
                    framebuffer[draw_y * pitch_pixels + draw_x] = 0x000000; 
                }
            }
        }
    }

    old_mouse = mouse;
    first_mouse_draw = false;
}