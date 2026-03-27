#include "idt/mouse/displayMouse.h"

uint32_t mouse_bg_buffer[16 * 16]; 
point old_mouse(-1, -1);       
bool first_mouse_draw = true;
bool was_mouse_active = true;



// ==========================================
// Erases the mouse from the framebuffer
// ==========================================
void erase_mouse() {
    // Early return if the mouse is already erased or hasn't been drawn yet
    if (first_mouse_draw) return;

    // Restore the background pixels over the current mouse position
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int draw_x = old_mouse.x + x;
            int draw_y = old_mouse.y + y;
            
            if (draw_x >= 0 && draw_x < screen_width && draw_y >= 0 && draw_y < screen_height) {
                framebuffer[draw_y * pitch_pixels + draw_x] = mouse_bg_buffer[y * 16 + x];
            }
        }
    }
    
    // CRITICAL: Reset this flag so the OS knows the mouse is currently hidden
    // and needs to recapture the background next time it draws!
    first_mouse_draw = true; 
}

void update_mouse_cursor() {
    // Early return if mouse is inactive and already processed
    if (!mouse_active && !was_mouse_active) return;
    
    // Handle the transition from active to inactive (restore background)
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
    
    // Early return if mouse hasn't moved (prevents redundant drawing)
    if (mouse == old_mouse && !first_mouse_draw) return; 
    
    // Restore the old background pixels before drawing the mouse at the new position
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
    
    // Draw the new mouse cursor and save the background beneath it
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int draw_x = mouse.x + x;
            int draw_y = mouse.y + y;
            
            if (draw_x >= 0 && draw_x < screen_width && draw_y >= 0 && draw_y < screen_height) {
                // Save the current pixel
                mouse_bg_buffer[y * 16 + x] = framebuffer[draw_y * pitch_pixels + draw_x];
                
                // Draw mouse outline or fill based on the bitmap
                if (mouse_bitmap[y][x] == 1) {
                    framebuffer[draw_y * pitch_pixels + draw_x] = 0xFFFFFF; // Border
                } else if (mouse_bitmap[y][x] == 2) {
                    framebuffer[draw_y * pitch_pixels + draw_x] = 0x000000; // Fill
                }
            }
        }
    }
    
    // Update tracking variables for the next frame
    old_mouse = mouse;
    first_mouse_draw = false;
}