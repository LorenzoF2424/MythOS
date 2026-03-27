#ifndef DISPLAY_TERMINAL_H
#define DISPLAY_TERMINAL_H


#include <stddef.h>
#include <stdint.h>
#include "displayVESA.h"
#include "fontBitmap.h"
#include "terminalCursor.h"
#include "terminalTheme.h"
#include "gnu_utils/point.h"
#include "gnu_utils/string.h"
#include "sched/spinlock.h"
#include "mem/kheap.h"
#include "idt/mouse/displayMouse.h"


#define MAX_INPUT_LEN 16384
#define MAX_BUFFER_COLUMNS 480
#define MAX_BUFFER_ROWS    135



struct TerminalChar {
   
    char c;          
    terminal_color_t color;
    bool locked;
};

struct terminal_input_t {

    point start;
    uint16_t max_x;
    uint16_t pos;
    uint16_t len;
    char *command;

    void setStart();

};

struct terminal_output_t {
    // --- Data ---
    terminal_color_t color;
    point cursor; 
    point max;
    uint16_t buf_start;
    uint8_t scale;
    uint8_t tab_size;
    uint8_t y_offset;
    bool direct;
    bool lock_text;
    bool theme_enabled;

    // --- Methods ---
    void init();
    void clear();
    void visual_scroll();
    void redraw_all();
    void scroll();
    void calculate_max();
    
    // Color and Cursor
    void set_cursor(point p);
    void set_input_limit();
    void apply_colors(terminal_color_t old, terminal_color_t new_c);
    void change_color(terminal_color_t new_c);
    void reset_color();
    
    // Output
    void putchar(char c);
    void putchar_raw(char c);
    void putcharAt(char c, point p);
    void write(const char* str);
    void writeAt(const char* str, point p);
    void render_from_buffer(uint16_t start_pos);
    void write_welcome();

    // Row and Column Behaviour
    void check_bounds();

   
};


// Global Terminal
extern terminal_output_t terminal;
extern terminal_output_t info_bar_window;
extern bool draw_info_was_disabled;
extern terminal_input_t input;
extern TerminalChar terminal_text_buffer[MAX_BUFFER_ROWS][MAX_BUFFER_COLUMNS];
extern uint16_t view_offset;
extern uint16_t history_lines;
extern uint8_t view_scroll;
extern Spinlock terminal_lock;

// Input Extractor
char* getInput();




#endif