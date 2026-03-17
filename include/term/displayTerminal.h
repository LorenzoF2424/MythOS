#ifndef DISPLAY_TERMINAL_H
#define DISPLAY_TERMINAL_H

#include "displayVESA.h"
#include "fontBitmap.h"
#include "gnu_utils/point.h"
#include "sched/spinlock.h"
#include <stddef.h>
#include <stdint.h>

#define MAX_INPUT_LEN 16384


struct terminal_info_t {
    terminal_color_t color;
    point cursor;
    uint16_t limit_col;
    uint16_t limit_row;
    uint16_t max_columns; 
    uint16_t max_rows; 
    uint8_t scale;
    uint8_t tab_size;
};

extern terminal_info_t terminal_data;
extern Spinlock terminal_lock;
extern bool draw_info;
extern bool draw_info_was_disabled;
extern bool cursor_visible;
extern bool cursor_blink;
extern volatile bool draw_cursor;
extern uint8_t cursor_shape;
extern char input_buffer[MAX_INPUT_LEN];
extern uint16_t input_len;
extern uint16_t input_pos;
extern point cursorp;

void change_terminal_color(terminal_color_t new_color);
void reset_terminal_color();
void terminal_toggle_cursor_shape();
void remove_cursor_shape();
void terminal_cursor_update();
void terminal_restore_cursor(bool was_visible);
void reset_cursor_blink();
void draw_char(char c, point p, terminal_color_t color);
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