#include "term/displayTerminal.h"

bool draw_info = true;
bool draw_info_was_disabled = false;
terminal_info_t terminal_data = {
    .color ={
        .fg = 0xFFFFFF,
        .bg = 0x000000
    },
    .cursor = point(0,0),
    .limit_col = 0,
    .limit_row = 0,
    .max_columns = 0,
    .max_rows = 0,
    .scale = 1,
    .tab_size = 4
};

Spinlock terminal_lock;

void disable_info() {
    draw_info = false;
    draw_info_was_disabled = true;
}   

void apply_terminal_colors(terminal_color_t old_color, terminal_color_t new_color) {
    uint32_t total_pixels = screen_height * pitch_pixels;
    for (uint32_t i = 0; i < total_pixels; i++) {
        if (framebuffer[i] == old_color.fg) {
            framebuffer[i] = new_color.fg;
        } else if (framebuffer[i] == old_color.bg) {
            framebuffer[i] = new_color.bg;
        }
    }
}

void terminal_change_color(terminal_color_t new_color) {
    terminal_color_t old_color = terminal_data.color;
    terminal_data.color.fg = new_color.fg;
    terminal_data.color.bg = new_color.bg;
    apply_terminal_colors(old_color, new_color);
}

void terminal_reset_color() {
    terminal_data.color.fg = 0xFFFFFF;
    terminal_data.color.bg = 0x000000;
}

point cursorp;
bool cursor_visible = false;
bool cursor_blink = true;
volatile bool draw_cursor=true;
uint8_t cursor_shape=0;
void terminal_toggle_cursor_shape() {
    
    point start = point(cursorp.x*8, cursorp.y*16);

    switch (cursor_shape) {

        case 0:
            for (int y = 12; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start.y + y) * pitch_pixels);
        
                for (int x = 0; x < 8; x++) {
                    row_ptr[start.x + x] ^= 0x00FFFFFF; 
                }
            }

        break;
        case 1:
            for (int y = 0; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start.y + y) * pitch_pixels);
            
                for (int x = 0; x < 8; x++) {
                    if (y == 0 || y == 15 || x == 0 || x == 7) {
                        row_ptr[start.x + x] ^= 0x00FFFFFF; 
                    }
                }
            }

        break;
        case 2:
            for (int y = 0; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start.y + y) * pitch_pixels);
        
                for (int x = 0; x < 8; x++) {
                    row_ptr[start.x + x] ^= 0x00FFFFFF; 
                }
            }

        break;
    }
    
}

void remove_cursor_shape() {

    if (cursor_visible) {
        terminal_toggle_cursor_shape();
        cursor_visible = false;
    }
}

void terminal_cursor_update() {
    if (!draw_cursor) return;

    if (!cursor_blink && cursor_visible) return;

    
    
    cursorp = terminal_data.cursor;
    terminal_toggle_cursor_shape();
    cursor_visible = !cursor_visible; 
}

void terminal_restore_cursor(bool was_visible) {
    if (!was_visible) return;

    cursorp = terminal_data.cursor;
    terminal_toggle_cursor_shape();
    cursor_visible = true;
}

void reset_cursor_blink() {
    
    if (!draw_cursor) return;

    cursorp = terminal_data.cursor; 

    if (cursor_visible) return;

    terminal_toggle_cursor_shape();
    cursor_visible = true;
}


void draw_char(char c, point p, terminal_color_t color) {
    unsigned char uc = (unsigned char)c;
    p.x*=8;
    p.y*=16;
    for (uint8_t row = 0; row < 16; row++) {
        unsigned char bits = fontBitmap[uc][row];
        for (uint8_t col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                put_pixel(p.x + col, p.y + row, color.fg);
            } else {
                put_pixel(p.x + col, p.y + row, color.bg);
            }
        }
    }
}



void terminal_scroll() {
    uint32_t font_height = 16; 
    uint32_t text_row_pixels = pitch_pixels * font_height; 
    uint32_t total_pixels = screen_height * pitch_pixels;

    uint32_t protected_rows = draw_info ? 4 : 0;
    
    uint32_t protected_pixels = protected_rows * font_height * pitch_pixels;

   
    for (uint32_t i = protected_pixels; i < total_pixels - text_row_pixels; i++) {
        framebuffer[i] = framebuffer[i + text_row_pixels];
    }

    
    for (uint32_t i = total_pixels - text_row_pixels; i < total_pixels; i++) {
        framebuffer[i] = terminal_data.color.bg;
    }

    terminal_data.cursor.y = MAX_ROWS - 1;
}

void column_behaviour(terminal_info_t *t) {
    if (t->cursor.x >= MAX_COLUMNS) {
        t->cursor.x = 0;
        t->cursor.y++;
    }
}

void row_behaviour(terminal_info_t *t) {
    if (t->cursor.y >= MAX_ROWS) 
        terminal_scroll(); 
}

void print_behaviour(terminal_info_t *t) {
    if (t->cursor.x >= MAX_COLUMNS) {
        t->cursor.x = 0;
        t->cursor.y++;
    }

    if (t->cursor.y >= MAX_ROWS) 
        terminal_scroll(); 
}

void terminal_set_cursor(terminal_info_t *t, point p) {
    t->cursor.x = p.x;
    t->cursor.y = p.y;
}

void terminal_set_input_limit() {
    terminal_data.limit_col = terminal_data.cursor.x;
    terminal_data.limit_row = terminal_data.cursor.y;
}


char input_buffer[MAX_INPUT_LEN];
uint16_t input_len=0;
uint16_t input_pos=0;

void terminal_render_from_buffer(uint16_t start_pos) {
    uint16_t saved_col = terminal_data.cursor.x;
    uint16_t saved_row = terminal_data.cursor.y;

    for (uint16_t i = start_pos; i < input_len; i++) {
        if (terminal_data.cursor.x >= MAX_COLUMNS) {
            terminal_data.cursor.x = 0;
            terminal_data.cursor.y++;
        }
        if (terminal_data.cursor.y >= MAX_ROWS) terminal_scroll();

        draw_char(input_buffer[i], terminal_data.cursor, terminal_data.color);
        terminal_data.cursor.x++;
    }

    draw_rect(terminal_data.cursor.x * 8, terminal_data.cursor.y * 16, 8, 16, terminal_data.color.bg);

    terminal_data.cursor.x = saved_col;
    terminal_data.cursor.y = saved_row;
    
}



inline void terminal_putchar(char c) {

    remove_cursor_shape();

    switch (c) {
        case '\b': 
            if (terminal_data.cursor.x > 0) {
                terminal_data.cursor.x--;
            } else if (terminal_data.cursor.y > terminal_data.limit_row) {
                terminal_data.cursor.y--;
                terminal_data.cursor.x = MAX_COLUMNS - 1;
            } else {
                return; 
            }

            draw_rect(terminal_data.cursor.x * 8, terminal_data.cursor.y * 16, 8, 16, terminal_data.color.bg);
            return;
        break;

        case '\t': {
            uint16_t spaces = terminal_data.tab_size - (terminal_data.cursor.x % terminal_data.tab_size);
            terminal_data.cursor.x += spaces;
            
            if (terminal_data.cursor.x >= MAX_COLUMNS) {
                terminal_data.cursor.x = 0;
                terminal_data.cursor.y++;
                if (terminal_data.cursor.y >= MAX_ROWS) terminal_scroll();
            }
            return;

        } break;

        case '\n':
            terminal_data.cursor.x = 0;
            terminal_data.cursor.y++;
            if (terminal_data.cursor.y >= MAX_ROWS) terminal_scroll();
            return;
        break;
        default:
            if (terminal_data.cursor.x >= MAX_COLUMNS) {
                terminal_data.cursor.x = 0;
                terminal_data.cursor.y++;
            }
            if (terminal_data.cursor.y >= MAX_ROWS) terminal_scroll();

            draw_char(c, terminal_data.cursor, terminal_data.color);
            terminal_data.cursor.x++;
        break;
    }
    
}



void terminal_putchar_at(char c, point p) {

    spinlock_acquire(&terminal_lock);
    bool was_visible = cursor_visible; 
    
    remove_cursor_shape(); 

    point temp = terminal_data.cursor;
    terminal_data.cursor = p;
    terminal_putchar(c);
    terminal_data.cursor = temp;

    terminal_restore_cursor(was_visible);
    spinlock_release(&terminal_lock);

}

void terminal_write(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

void terminal_write_at(const char* str, point p) {
    
    spinlock_acquire(&terminal_lock);
    
    bool was_visible = cursor_visible;
    remove_cursor_shape();

    point temp = terminal_data.cursor;
    terminal_set_cursor(&terminal_data, p);
    
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
    
    terminal_set_cursor(&terminal_data, temp);

    terminal_restore_cursor(was_visible);
    spinlock_release(&terminal_lock);
}

void terminal_clear() {
    spinlock_acquire(&terminal_lock);
    uint32_t total_pixels = screen_height * pitch_pixels;
    for (uint32_t i = 0; i < total_pixels; i++) {
        framebuffer[i] = terminal_data.color.bg;
    }
    terminal_set_cursor(&terminal_data, (point){0, 0});
    spinlock_release(&terminal_lock);
}


void terminal_write_welcome_message() {
    terminal_write("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n");
    terminal_write("\xBA        Welcome to MythOS!!           \xBA\n");
    terminal_write("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\n\n");
}
