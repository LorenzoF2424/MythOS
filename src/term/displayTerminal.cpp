#include "term/displayTerminal.h"

terminal_output_t terminal = {
    .color     = { .fg = 0xFFFFFF, .bg = 0x000000 },
    .cursor    = point(0, 0),
    .max       = point(0, 0),
    .buf_start = 0,
    .scale     = 1,
    .tab_size  = 4,
    .y_offset  = 4,
    .direct    = false,
    .lock_text = false,
    .theme_enabled = true
};
terminal_output_t info_bar_window;

// ==========================================
// Input handling structure
// ==========================================
terminal_input_t input = {
    .start   = point(0, 0),
    .max_x   = 0,
    .pos     = 0,
    .len     = 0,
    .command = nullptr
};

TerminalChar terminal_text_buffer[MAX_BUFFER_ROWS][MAX_BUFFER_COLUMNS];

// ==========================================
// Global scrollback buffer variables
// ==========================================
uint16_t view_offset   = 0;
uint16_t history_lines = 0;
uint8_t  view_scroll   = 1;

Spinlock terminal_lock;

bool    draw_info              = true;
bool    draw_info_was_disabled = false;


// ==========================================
// UTILITY FUNCTIONS
// ==========================================

void draw_char(char c, point p, terminal_color_t color) {
    unsigned char uc = (unsigned char)c;
    int32_t px = p.x * 8 * terminal.scale;
    int32_t py = p.y * 16 * terminal.scale;

    for (uint8_t row = 0; row < 16; row++) {
        unsigned char bits    = fontBitmap[uc][row];
        uint32_t* row_ptr = framebuffer + ((py + row) * pitch_pixels) + px;
        for (uint8_t col = 0; col < 8; col++) {
            row_ptr[col] = (bits & (0x80 >> col)) ? color.fg : color.bg;
        }
    }
}

// ==========================================
// RING BUFFER HELPER
// Maps logical cursor row → physical buffer row using buf_start
// ==========================================
static inline uint32_t buf_row(const terminal_output_t* t, uint32_t logical_y) {
    return (t->buf_start + logical_y) % MAX_BUFFER_ROWS;
}

// ==========================================
// TERMINAL_T METHODS
// ==========================================

void terminal_output_t::calculate_max() {
    max = point(
        screen_width  / (8  * scale),
        screen_height / (16 * scale) - y_offset  // reserve rows for info bar
    );
}

void terminal_output_t::init() {
    spinlock_acquire(&terminal_lock);
    for (uint32_t y = 0; y < MAX_BUFFER_ROWS; y++)
        for (uint32_t x = 0; x < MAX_BUFFER_COLUMNS; x++)
            terminal_text_buffer[y][x] = (TerminalChar){' ', color, false};

    this->calculate_max();
    spinlock_release(&terminal_lock);
    init_default_theme();

}

void terminal_output_t::clear() {
    spinlock_acquire(&terminal_lock);

    for (uint32_t y = 0; y < MAX_BUFFER_ROWS; y++)
        for (uint32_t x = 0; x < MAX_BUFFER_COLUMNS; x++)
            terminal_text_buffer[y][x] = (TerminalChar){' ', color, false};

    uint32_t total_pixels = screen_height * pitch_pixels;
    for (uint32_t i = 0; i < total_pixels; i++)
        framebuffer[i] = color.bg;

    cursor      = point(0, 0);
    buf_start   = 0;
    input.start = point(0, 0);
    
    // Reset global scroll variables on clear
    view_offset   = 0;
    history_lines = 0;

    spinlock_release(&terminal_lock);
}

void terminal_output_t::redraw_all() {
    erase_mouse();

    for (uint32_t screen_y = 0; screen_y < (uint32_t)max.y; screen_y++) {
        
        // CAMERA LOGIC: Calculate physical row by moving back 'view_offset'.
        // Add MAX_BUFFER_ROWS before modulo to avoid negative numbers in C++.
        uint32_t buffer_y = (buf_start + screen_y + MAX_BUFFER_ROWS - view_offset) % MAX_BUFFER_ROWS;
        
        for (uint32_t x = 0; x < (uint32_t)max.x; x++) {
            
            // Early Return (Break): Prevent reading outside logical bounds
            if (x >= MAX_BUFFER_COLUMNS) break;
            
            // Fetch the character and its specific saved color from RAM
            TerminalChar t = terminal_text_buffer[buffer_y][x];
            
            // Ensure null terminators are drawn as spaces to clean the background
            char display_char = (t.c == '\0') ? ' ' : t.c;

            // THE FIX: Use 't.color' instead of 'terminal.color' to respect the theme!
            draw_char(display_char, point((int32_t)x, (int32_t)(screen_y + y_offset)), t.color);
        }
    }
}

void terminal_output_t::scroll() {
    // 1. Update global history count
    uint32_t max_history_possible = MAX_BUFFER_ROWS - max.y;
    if (history_lines < max_history_possible) {
        history_lines++;
    }

    // O(1) ring-buffer scroll: just advance buf_start by one row
    // and clear the new last logical row — no data copying needed.
    buf_start = (buf_start + 1) % MAX_BUFFER_ROWS;

    // Clear the new last row (it will appear at the bottom of the screen)
    uint32_t last = buf_row(this, max.y - 1);
    for (uint32_t x = 0; x < MAX_BUFFER_COLUMNS; x++)
        terminal_text_buffer[last][x] = (TerminalChar){' ', color, false};

    if (input.start.y > 0) input.start.y--;
    else input.start = point(0, 0);
}

void terminal_output_t::visual_scroll() {

    erase_mouse();
    // If the camera is shifted (user is looking at history), 
    // we only update the logical ring buffer and exit. No physical drawing!
    if (view_offset > 0) {
        scroll();
        return; 
    }

    // Move framebuffer pixels up by one character row using memmove (very fast).
    // Then draw only the new bottom row — no full redraw needed.
    uint32_t row_pixels = 16 * scale * pitch_pixels;   // pixels per text row
    uint32_t offset     = y_offset * row_pixels;       // skip info bar rows

    // Shift pixels up by one row
    memmove(
        framebuffer + offset,
        framebuffer + offset + row_pixels,
        (max.y - 1) * row_pixels * sizeof(uint32_t)
    );

    // Clear the new bottom row
    uint32_t last_row_start = offset + (max.y - 1) * row_pixels;
    for (uint32_t i = 0; i < row_pixels; i++)
        framebuffer[last_row_start + i] = color.bg;

    // Advance the ring buffer and draw the new last row
    scroll();

    uint32_t last_buf = buf_row(this, max.y - 1);
    for (uint32_t x = 0; x < (uint32_t)max.x && x < MAX_BUFFER_COLUMNS; x++) {
        draw_char(
            terminal_text_buffer[last_buf][x].c,
            point((int32_t)x, (int32_t)(max.y - 1 + y_offset)),
            terminal_text_buffer[last_buf][x].color
        );
    }
}

void terminal_output_t::check_bounds() {
    if (max.x == 0 || max.y == 0) calculate_max();
    uint16_t wrap_limit_x = (max.x > 0) ? max.x : MAX_BUFFER_COLUMNS;

    // Wrap to next line if we went past the right edge
    if (cursor.x >= wrap_limit_x) {
        cursor.x = 0;
        cursor.y++;
    }

    // Scroll visually whenever the cursor moves below the visible viewport.
    // visual_scroll() also calls scroll() which advances buf_start (O(1)).
    // cursor.y is decremented each time so it stays at max.y - 1.
    while (cursor.y >= max.y) {
        visual_scroll();
        cursor.y--;
    }
}

void terminal_output_t::set_cursor(point p) {
    if (p.x < MAX_BUFFER_COLUMNS && p.y < MAX_BUFFER_ROWS)
        cursor = p;
}

void terminal_output_t::set_input_limit() {
    input.start = cursor;
    input.max_x = max.x;
}

void terminal_output_t::apply_colors(terminal_color_t old_c, terminal_color_t new_c) {
    uint32_t total_pixels = screen_height * pitch_pixels;
    for (uint32_t i = 0; i < total_pixels; i++) {
        if      (framebuffer[i] == old_c.fg) framebuffer[i] = new_c.fg;
        else if (framebuffer[i] == old_c.bg) framebuffer[i] = new_c.bg;
    }
}

void terminal_output_t::change_color(terminal_color_t new_c) {
    // Update the current pen color
    color = new_c;

    // Iterate through the entire logical text buffer in RAM
    for (uint32_t y = 0; y < MAX_BUFFER_ROWS; y++) {
        for (uint32_t x = 0; x < MAX_BUFFER_COLUMNS; x++) {
            
            // Early return equivalent (continue) for locked characters (e.g., Kernel Panics)
            if (terminal_text_buffer[y][x].locked) continue;
            
            // Dynamically re-evaluate the character's color based on the active theme
            if (theme_enabled) {
                terminal_text_buffer[y][x].color = get_theme_color(terminal_text_buffer[y][x].c, new_c);
            } else {
                // If the theme is disabled, apply the standard flat color
                terminal_text_buffer[y][x].color = new_c;
            }
        }
    }

    // Overwrite the entire physical screen background with the new background color
    uint32_t total_pixels = screen_height * pitch_pixels;
    for (uint32_t i = 0; i < total_pixels; i++) {
        framebuffer[i] = new_c.bg;
    }

    // Redraw the current viewport with the freshly calculated colors
    redraw_all();
}

void terminal_output_t::reset_color() {
    color = (terminal_color_t){0xFFFFFF, 0x000000};
}



void terminal_output_t::putchar(char c) {

    erase_mouse();
    // Note: remove_cursor_shape() is NOT called here.
    // It is called once at the start of write() / kprintf() so that
    // bulk printing doesn't XOR the cursor pixels thousands of times.

    switch (c) {
        case '\b': {
            if (cursor.x > 0) {
                // Don't backspace over the prompt
                if (cursor.y == input.start.y && cursor.x <= input.start.x) return;
                cursor.x--;
            } else if (cursor.y > input.start.y) {
                cursor.y--;
                uint16_t wrap_limit = (max.x > 0) ? max.x : MAX_BUFFER_COLUMNS;
                if (wrap_limit > MAX_BUFFER_COLUMNS) wrap_limit = MAX_BUFFER_COLUMNS;
                cursor.x = wrap_limit - 1;
            } else return;

            // Update ring-buffer cell unconditionally
            if (!direct)
                terminal_text_buffer[buf_row(this, cursor.y)][cursor.x] = {' ', color, false};

            // FIX: Ignore the global camera offset if drawing UI (direct mode)
            int32_t screen_y = cursor.y + (direct ? 0 : view_offset);
            
            // Only draw the background rectangle if the line is currently visible
            if (screen_y >= 0 && screen_y < (int32_t)max.y) {
                draw_rect(
                    cursor.x * 8 * scale,
                    (screen_y + y_offset) * 16 * scale,
                    8 * scale, 16 * scale,
                    color.bg
                );
            }
            return;
        }

        case '\t': {
            uint16_t spaces = tab_size - (cursor.x % tab_size);
            for (uint16_t i = 0; i < spaces; i++) putchar(' ');
            return;
        }

        case '\n':
            cursor.x = 0;
            cursor.y++;
            check_bounds();
            return;

       default: {
            // 1. Ensure the cursor is within physical buffer limits before starting
            check_bounds(); 

            // Early Return 1: If cursor is completely out of logical bounds, 
            // advance the position for the next char and abort drawing
            if (cursor.x >= MAX_BUFFER_COLUMNS || cursor.y >= (int32_t)max.y) {
                cursor.x++; 
                check_bounds(); 
                return;
            }

            // 2. Determine the character's color based on the theme engine
            terminal_color_t char_color = color;
            if (theme_enabled && !lock_text) {
                char_color = get_theme_color(c, color);
            }

            // 3. Save to the logical RAM buffer unconditionally using the new color
            if (!direct) {
                terminal_text_buffer[buf_row(this, cursor.y)][cursor.x] = {c, char_color, lock_text};
            }

            // 4. Calculate physical screen Y based on the global camera offset
            int32_t screen_y = cursor.y + (direct ? 0 : view_offset);
            
            // Early Return 2: If the character is logically saved but currently 
            // off-camera (scrolled away), advance the position and skip drawing
            if (screen_y < 0 || screen_y >= (int32_t)max.y) {
                cursor.x++; 
                check_bounds(); 
                return;
            }

            // 5. Draw the character to the visible framebuffer
            draw_char(c, point(cursor.x, screen_y + y_offset), char_color);
            
            // 6. Advance the cursor for the next character
            cursor.x++; 
            check_bounds(); 
            return;
        }
    }
}

void terminal_output_t::putcharAt(char c, point p) {
    if (p.x >= MAX_BUFFER_COLUMNS || p.y >= MAX_BUFFER_ROWS) return;

    spinlock_acquire(&terminal_lock);
    bool was_visible = cursor_visible;
    remove_cursor_shape();

    draw_char(c, point(p.x, p.y + y_offset), color);

    terminal_restore_cursor(was_visible);
    spinlock_release(&terminal_lock);
}

void terminal_output_t::write(const char* str) {
    spinlock_acquire(&terminal_lock);
    remove_cursor_shape();  // once, before bulk printing
    for (size_t i = 0; str[i] != '\0'; i++)
        putchar(str[i]);
    spinlock_release(&terminal_lock);
}

void terminal_output_t::writeAt(const char* str, point p) {
    if (p.x >= MAX_BUFFER_COLUMNS || p.y >= MAX_BUFFER_ROWS) return;

    spinlock_acquire(&terminal_lock);
    bool was_visible = cursor_visible;
    remove_cursor_shape();

    point temp_cursor = cursor;
    cursor = p;
    for (size_t i = 0; str[i] != '\0'; i++)
        putcharAt(str[i],point(p.x+i,p.y));
    cursor = temp_cursor;

    terminal_restore_cursor(was_visible);
    spinlock_release(&terminal_lock);
}

void terminal_output_t::write_welcome() {
    write("\xC9\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBB\n");
    write("\xBA         Welcome to MythOS!!          \xBA\n");
    write("\xC8\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xCD\xBC\n\n");
}

void terminal_input_t::setStart() {
    input.start = terminal.cursor;
}

// ==========================================
// INPUT EXTRACTION
// Reads exactly input.len characters from the ring buffer starting at
// input.start, following the same wrap logic used during typing.
// Returns a heap-allocated, null-terminated string (caller must free via
// prepare_for_next_command).
// ==========================================
char* getInput() {
    if (input.len == 0) return nullptr;

    input.command = (char*)malloc(input.len);

    point    current    = input.start;
    uint16_t wrap_limit = (input.max_x > 0) ? input.max_x : MAX_BUFFER_COLUMNS;
    if (wrap_limit > MAX_BUFFER_COLUMNS) wrap_limit = MAX_BUFFER_COLUMNS;

    for (uint16_t i = 0; i < input.len; i++) {
        // Map logical row through ring buffer
        uint32_t physical_y = (terminal.buf_start + current.y) % MAX_BUFFER_ROWS;
        input.command[i]    = terminal_text_buffer[physical_y][current.x].c;
        current.x++;

        if (current.x >= wrap_limit) {
            current.x = 0;
            current.y++;
            if (current.y >= MAX_BUFFER_ROWS)
                current.y = MAX_BUFFER_ROWS - 1;
        }
    }

    input.command[input.len] = '\0';
    return input.command;
}