#ifndef TERMINAL_CURSOR_H
#define TERMINAL_CURSOR_H

#include <stdint.h>
#include <stdbool.h>
#include "gnu_utils/point.h"

// Note: Ensure the 'point' structure is defined before including this file, 
// or include the header that defines it here (e.g., #include "utils/types.h").

// ==========================================
// Global Cursor State Variables
// ==========================================
extern bool    cursor_visible;
extern bool    cursor_blink;
extern bool    draw_cursor;
extern uint8_t cursor_shape;
extern point   cursorp;

// ==========================================
// Cursor Management Functions
// ==========================================
void terminal_toggle_cursor_shape();
void remove_cursor_shape();
void terminal_cursor_update();
void terminal_restore_cursor(bool was_visible);
void reset_cursor_blink();

#endif