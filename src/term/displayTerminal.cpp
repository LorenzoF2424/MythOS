#include "term/displayTerminal.h"

terminal_info_t terminal_data = {
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



void change_terminal_color(uint32_t fg, uint32_t bg) {
    terminal_data.color_fg = fg;
    terminal_data.color_bg = bg;
}

void reset_terminal_color() {
    terminal_data.color_fg = 0xFFFFFF;
    terminal_data.color_bg = 0x000000;
}

bool cursor_visible = false;
volatile bool draw_cursor=true;
uint8_t cursor_shape=0;
void terminal_toggle_cursor_shape() {
    int start_x = terminal_data.cursor_col * 8;
    int start_y = terminal_data.cursor_row * 16;

    switch (cursor_shape) {

        case 0:
            for (int y = 12; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start_y + y) * pitch_pixels);
        
                for (int x = 0; x < 8; x++) {
                    row_ptr[start_x + x] ^= 0x00FFFFFF; 
                }
            }

        break;
        case 1:
            for (int y = 0; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start_y + y) * pitch_pixels);
            
                // Scorriamo tutta la larghezza (da 0 a 7)
                for (int x = 0; x < 8; x++) {
                    if (y == 0 || y == 15 || x == 0 || x == 7) {
                        row_ptr[start_x + x] ^= 0x00FFFFFF; 
                    }
                }
            }

        break;
        case 2:
            for (int y = 0; y < 16; y++) {
                uint32_t* row_ptr = framebuffer + ((start_y + y) * pitch_pixels);
        
                for (int x = 0; x < 8; x++) {
                    row_ptr[start_x + x] ^= 0x00FFFFFF; 
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

void draw_char(char c, int x, int y, uint32_t fg, uint32_t bg) {
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



void terminal_scroll() {
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

void column_behaviour(terminal_info_t *t) {
    if (t->cursor_col >= MAX_COLUMNS) {
        t->cursor_col = 0;
        t->cursor_row++;
    }
}

void row_behaviour(terminal_info_t *t) {
    if (t->cursor_row >= MAX_ROWS) 
        terminal_scroll(); 
}

void print_behaviour(terminal_info_t *t) {
    if (t->cursor_col >= MAX_COLUMNS) {
        t->cursor_col = 0;
        t->cursor_row++;
    }

    if (t->cursor_row >= MAX_ROWS) 
        terminal_scroll(); 
}

void terminal_set_cursor(terminal_info_t *t, uint16_t x, uint16_t y) {
    t->cursor_col = x;
    t->cursor_row = y;
}

void terminal_set_input_limit() {
    terminal_data.limit_col = terminal_data.cursor_col;
    terminal_data.limit_row = terminal_data.cursor_row;
}


char input_buffer[MAX_INPUT_LEN];
uint16_t input_len=0;
uint16_t input_pos=0;

void terminal_render_from_buffer(uint16_t start_pos) {
    // Salviamo la posizione attuale per poterci tornare
    uint16_t saved_col = terminal_data.cursor_col;
    uint16_t saved_row = terminal_data.cursor_row;

    // Disegniamo il contenuto del buffer da start_pos alla fine
    for (uint16_t i = start_pos; i < input_len; i++) {
        // Wrapping preventivo
        if (terminal_data.cursor_col >= MAX_COLUMNS) {
            terminal_data.cursor_col = 0;
            terminal_data.cursor_row++;
        }
        if (terminal_data.cursor_row >= MAX_ROWS) terminal_scroll();

        draw_char(input_buffer[i], terminal_data.cursor_col * 8, 
                  terminal_data.cursor_row * 16, terminal_data.color_fg, terminal_data.color_bg);
        terminal_data.cursor_col++;
    }

    // PULIZIA: Se abbiamo accorciato la stringa (backspace), cancelliamo l'ultimo carattere vecchio
    draw_rect(terminal_data.cursor_col * 8, terminal_data.cursor_row * 16, 8, 16, terminal_data.color_bg);

    // Ripristiniamo il cursore dove deve stare per l'utente
    terminal_data.cursor_col = saved_col;
    terminal_data.cursor_row = saved_row;
    
}



// --- TERMINAL PUTCHAR AGGIORNATA ---
inline void terminal_putchar(char c) {

    remove_cursor_shape();

    switch (c) {
        case '\b': 
            if (terminal_data.cursor_col > 0) {
                terminal_data.cursor_col--;
            } else if (terminal_data.cursor_row > terminal_data.limit_row) {
                terminal_data.cursor_row--;
                terminal_data.cursor_col = MAX_COLUMNS - 1;
            } else {
                return; 
            }

            draw_rect(terminal_data.cursor_col * 8, terminal_data.cursor_row * 16, 8, 16, terminal_data.color_bg);
            return;
        break;

        case '\t': {
            uint16_t spaces = terminal_data.tab_size - (terminal_data.cursor_col % terminal_data.tab_size);
            terminal_data.cursor_col += spaces;
            
            // Controlla se il Tab ti ha fatto uscire dallo schermo
            if (terminal_data.cursor_col >= MAX_COLUMNS) {
                terminal_data.cursor_col = 0;
                terminal_data.cursor_row++;
                if (terminal_data.cursor_row >= MAX_ROWS) terminal_scroll();
            }
            return;

        } break;

        case '\n':
            terminal_data.cursor_col = 0;
            terminal_data.cursor_row++;
            if (terminal_data.cursor_row >= MAX_ROWS) terminal_scroll();
            return;
        break;
        default:
            if (terminal_data.cursor_col >= MAX_COLUMNS) {
                terminal_data.cursor_col = 0;
                terminal_data.cursor_row++;
            }
            if (terminal_data.cursor_row >= MAX_ROWS) terminal_scroll();

            draw_char(c, terminal_data.cursor_col * 8, terminal_data.cursor_row * 16, 
                      terminal_data.color_fg, terminal_data.color_bg);
            terminal_data.cursor_col++;
        break;
    }
}



void terminal_putchar_at(char c, uint16_t x, uint16_t y) {
    uint16_t tempx = terminal_data.cursor_col;
    uint16_t tempy = terminal_data.cursor_row;
    terminal_set_cursor(&terminal_data, x, y);
    terminal_putchar(c);
    terminal_set_cursor(&terminal_data, tempx, tempy);
}

void terminal_write(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
    }
}

void terminal_write_at(const char* str, uint16_t x, uint16_t y) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar_at(str[i], x++, y);
    }
}

void terminal_clear() {
    uint32_t total_pixels = screen_height * pitch_pixels;
    for (uint32_t i = 0; i < total_pixels; i++) {
        framebuffer[i] = terminal_data.color_bg;
    }
    terminal_set_cursor(&terminal_data, 0, 0);
}


void terminal_write_welcome_message() {
    terminal_write("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n");
    terminal_write("\xBA        Welcome to MythicOS!!         \xBA\n");
    terminal_write("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\n\n");
}
