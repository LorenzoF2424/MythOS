#ifndef DISPLAY_TERMINAL_H
#define DISPLAY_TERMINAL_H

#include "displayVESA.h"
#include "fontBitmap.h"
#include "gnu_utils/point.h"
#include <stddef.h>
#include <stdint.h>

#define MAX_INPUT_LEN 64000


struct terminal_info_t {
    vga_color_t color;
    point cursor;
    uint16_t limit_col;
    uint16_t limit_row;
    uint16_t max_columns; 
    uint16_t max_rows; 
    uint8_t scale;
    uint8_t tab_size;
};

extern terminal_info_t terminal_data;
extern bool cursor_visible;
extern volatile bool draw_cursor;
extern uint8_t cursor_shape;
extern char input_buffer[MAX_INPUT_LEN];
extern uint16_t input_len;
extern uint16_t input_pos;
extern point cursorp;

void change_terminal_color(uint32_t fg, uint32_t bg);
void reset_terminal_color();
void terminal_toggle_cursor_shape();
void remove_cursor_shape();
void draw_char(char c, point p, vga_color_t color);
void terminal_scroll();
void column_behaviour(terminal_info_t *t);
void row_behaviour(terminal_info_t *t);
void print_behaviour(terminal_info_t *t);
void terminal_set_cursor(terminal_info_t *t, point p);
void terminal_set_input_limit();
void terminal_render_from_buffer(uint16_t start_pos);
void terminal_putchar(char c);
void terminal_putchar_at(char c, point p);
void terminal_write(const char* str);
void terminal_write_at(const char* str, point p);
void terminal_clear();
void terminal_write_welcome_message();

#endif