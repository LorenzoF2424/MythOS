#ifndef DISPLAY_TERMINAL_H
#define DISPLAY_TERMINAL_H

#include "displayVESA.h"
#include "fontBitmap.h"
#include <stddef.h> // Per size_t

struct terminal_info_t {
    uint32_t color_fg;
    uint32_t color_bg;
    uint16_t cursor_col;
    uint16_t cursor_row;
    uint16_t limit_col;
    uint16_t limit_row;
    uint16_t max_columns; 
    uint16_t max_rows; 
    uint8_t scale;
    uint8_t tab_size;
};

// Usa inline per le variabili globali negli header!
inline terminal_info_t terminal_data = {
    .color_fg = 0xFFFFFF,
    .color_bg = 0x000000,
    .cursor_col = 0,
    .cursor_row = 0,
    .limit_col = 0,
    .limit_row = 0,
    .max_columns = 0,
    .max_rows = 0,
    .scale = 1,
    .tab_size = 4
};


inline void change_terminal_color(uint32_t fg, uint32_t bg) {
    terminal_data.color_fg = fg;
    terminal_data.color_bg = bg;
}

inline void reset_terminal_color() {
    terminal_data.color_fg = 0xFFFFFF;
    terminal_data.color_bg = 0x000000;
}

inline void draw_char(char c, int x, int y, uint32_t fg, uint32_t bg) {
    unsigned char uc = (unsigned char)c;
    
    for (uint8_t row = 0; row < 16; row++) {
        unsigned char bits = font_bitmap[uc][row];
        for (uint8_t col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                put_pixel(x + col, y + row, fg);
            } else {
                put_pixel(x + col, y + row, bg);
            }
        }
    }
}

inline void terminal_scroll() {
    // Ora ragioniamo in PIXEL a 32-bit, non più in byte!
    uint32_t text_row_pixels = pitch_pixels * 16; 
    uint32_t total_pixels = screen_height * pitch_pixels;
    uint32_t pixels_to_copy = total_pixels - text_row_pixels;

    // 1. Sposta tutto lo schermo in alto di una riga
    for (uint32_t i = 0; i < pixels_to_copy; i++) {
        framebuffer[i] = framebuffer[i + text_row_pixels];
    }

    // 2. Pulisci l'ultima riga in basso con il colore di sfondo
    for (uint32_t i = pixels_to_copy; i < total_pixels; i++) {
        framebuffer[i] = terminal_data.color_bg;
    }

    // 3. Riposiziona il cursore
    terminal_data.cursor_row = MAX_ROWS - 1;
}

inline void column_behaviour(terminal_info_t *t) {
    if (t->cursor_col >= MAX_COLUMNS) {
        t->cursor_col = 0;
        t->cursor_row++;
    }
}

inline void row_behaviour(terminal_info_t *t) {
    if (t->cursor_row >= MAX_ROWS) 
        terminal_scroll(); 
}

inline void print_behaviour(terminal_info_t *t) {
    if (t->cursor_col >= MAX_COLUMNS) {
        t->cursor_col = 0;
        t->cursor_row++;
    }

    if (t->cursor_row >= MAX_ROWS) 
        terminal_scroll(); 
}

inline void terminal_set_cursor(terminal_info_t *t, uint16_t x, uint16_t y) {
    t->cursor_col = x;
    t->cursor_row = y;
}

inline void terminal_set_input_limit() {
    terminal_data.limit_col = terminal_data.cursor_col;
    terminal_data.limit_row = terminal_data.cursor_row;
}

inline void terminal_putchar(char c) {
    if (c == '\b') {
        if (terminal_data.cursor_row == terminal_data.limit_row && 
            terminal_data.cursor_col <= terminal_data.limit_col) 
            return; 

        if (terminal_data.cursor_col > 0) 
            terminal_data.cursor_col--;
        else if (terminal_data.cursor_row > terminal_data.limit_row) {
            terminal_data.cursor_row--;
            terminal_data.cursor_col = MAX_COLUMNS - 1;
        } else return;
        
        uint16_t pixel_x = terminal_data.cursor_col * 8;
        uint16_t pixel_y = terminal_data.cursor_row * 16;
        draw_rect(pixel_x, pixel_y, 8, 16, terminal_data.color_bg);
        
        return;
    }

    if (c == '\n') {
        terminal_data.cursor_col = 0;
        terminal_data.cursor_row++;
        print_behaviour(&terminal_data);
        return; 
    }

    if (c == '\t') {
        uint16_t spaces_to_next_tab = terminal_data.tab_size - (terminal_data.cursor_col % terminal_data.tab_size);
        terminal_data.cursor_col += spaces_to_next_tab;
        column_behaviour(&terminal_data);
        return; 
    }

    uint16_t pixel_x = terminal_data.cursor_col * 8;
    uint16_t pixel_y = terminal_data.cursor_row * 16;
    draw_char(c, pixel_x, pixel_y, terminal_data.color_fg, terminal_data.color_bg);
    terminal_data.cursor_col++;

    print_behaviour(&terminal_data);
}

inline void terminal_putchar_at(char c, uint16_t x, uint16_t y) {
    uint16_t tempx = terminal_data.cursor_col;
    uint16_t tempy = terminal_data.cursor_row;
    terminal_set_cursor(&terminal_data, x, y);
    terminal_putchar(c);
    terminal_set_cursor(&terminal_data, tempx, tempy);
}

inline void terminal_write(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

inline void terminal_write_at(const char* str, uint16_t x, uint16_t y) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar_at(str[i], x++, y);
    }
}

inline void terminal_clear() {
    // Riscritta per usare il nuovo array a 32 bit! Molto più pulita.
    uint32_t total_pixels = screen_height * pitch_pixels;
    
    for (uint32_t i = 0; i < total_pixels; i++) {
        framebuffer[i] = terminal_data.color_bg;
    }
    terminal_set_cursor(&terminal_data, 0, 0);
}

inline void terminal_write_welcome_message() {
    terminal_write("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n");
    terminal_write("\xBA        Welcome to MythicOS!!         \xBA\n");
    terminal_write("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\n\n");
}

#endif