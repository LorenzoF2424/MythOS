#include "term/displayTerminal.h"
// If you create a specific header for the cursor, include it here instead, 
// e.g., #include "term/terminalCursor.h"

// ==========================================
// Cursor State Variables
// ==========================================
bool    cursor_visible = false;
bool    cursor_blink   = true;
bool    draw_cursor    = true;
uint8_t cursor_shape   = 0;
point   cursorp;

// ==========================================
// Cursor Drawing Logic
// ==========================================

void terminal_toggle_cursor_shape() {
    // 1. Calculate logical row
    uint32_t screen_y = terminal.cursor.y;

    // 2. Calculate physical starting point, applying info bar offset
    point start = point(
        terminal.cursor.x * 8  * terminal.scale,
        (screen_y + terminal.y_offset) * 16 * terminal.scale
    );

    // 3. Safety Check: Early return to prevent out-of-bounds memory access
    if (start.x + 8  > (int32_t)screen_width)  return;
    if (start.y + 16 > (int32_t)screen_height) return;

    // 4. Draw the selected cursor shape using XOR to toggle visibility
    switch (cursor_shape) {
        case 0: // Underline
            for (int y = 12; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start.y + y) * pitch_pixels);
                for (int x = 0; x < 8; x++) row_ptr[start.x + x] ^= 0x00FFFFFF;
            }
            break;
            
        case 1: // Box outline
            for (int y = 0; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start.y + y) * pitch_pixels);
                for (int x = 0; x < 8; x++) {
                    if (y == 0 || y == 15 || x == 0 || x == 7) {
                        row_ptr[start.x + x] ^= 0x00FFFFFF;
                    }
                }
            }
            break;
            
        case 2: // Full block
            for (int y = 0; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start.y + y) * pitch_pixels);
                for (int x = 0; x < 8; x++) row_ptr[start.x + x] ^= 0x00FFFFFF;
            }
            break;
    }
}

// ==========================================
// Cursor Management Functions
// ==========================================

void remove_cursor_shape() {
    // Early return if it's already hidden
    if (!cursor_visible) return;

    terminal_toggle_cursor_shape();
    cursor_visible = false;
}

void terminal_cursor_update() {
    // 1. If drawing is globally disabled, exit immediately
    if (!draw_cursor) return;

    // 2. If blink is off and cursor is already visible, nothing to do
    if (!cursor_blink && cursor_visible) return;

    // 3. CAMERA CHECK: Calculate the physical position on screen
    int32_t screen_y = terminal.cursor.y + view_offset;

    // 4. VIEWPORT BOUNDS: Hide the cursor if it's off-camera
    if (screen_y < 0 || screen_y >= (int32_t)terminal.max.y) {
        if (cursor_visible) {
            remove_cursor_shape(); 
            cursor_visible = false;
        }
        return; 
    }

    // 5. BLINK EXECUTION: Toggle shape
    cursorp = terminal.cursor;
    terminal_toggle_cursor_shape();
    cursor_visible = !cursor_visible;
}

void terminal_restore_cursor(bool was_visible) {
    // Early return if we don't need to restore it
    if (!was_visible) return;
    
    cursorp = terminal.cursor;
    terminal_toggle_cursor_shape();
    cursor_visible = true;
}

void reset_cursor_blink() {
    // Early return if cursor drawing is disabled
    if (!draw_cursor) return;
    
    cursorp = terminal.cursor;
    
    // Early return if it's already visible (no need to XOR twice)
    if (cursor_visible) return;
    
    terminal_toggle_cursor_shape();
    cursor_visible = true;
}