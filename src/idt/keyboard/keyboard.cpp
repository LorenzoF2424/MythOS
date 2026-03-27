#include "idt/keyboard/keyboard.h"
// Ensure you have the correct headers included here

bool key_states[MAX_KEY_CODES] = {false};
uint8_t current_scancode_set = 1; 
static bool expecting_release = false;

bool extended_scancode = false;
char history[MAX_HISTORY][MAX_COMMAND_LEN]; 
int8_t history_count = 0;
int8_t history_index = 0;

void init_keyboard() {
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    idt_set_gate(33, (uint64_t)keyboard_isr, cs, 0x8E);
}

extern "C" void keyboard_handler_c() {
    uint8_t scancode = inb(0x60);
    kbd_push(scancode);
    outb(0x20, 0x20); 
}

// ==========================================
// ACTION HANDLER
// Executes the logic for a fully decoded key
// ==========================================
void process_key(key_code_t key, bool shift_active, bool altgr_active) {

    //klog("%s(%x,%d,%d)", __func__, key, shift_active, altgr_active);
    
    char* cmd = nullptr;

    switch (key) {
        case KEY_UP:
            // Early return if no history
            if (history_count == 0 || history_index >= history_count) return;
            
            // Save the working draft if we are leaving the current line (index 0)
            if (history_index == 0) {
                cmd = getInput();
                if (cmd) {
                    strncpy(history[0], cmd, MAX_COMMAND_LEN - 1);
                    history[0][MAX_COMMAND_LEN - 1] = '\0';
                    free(cmd);
                } else {
                    history[0][0] = '\0';
                }
            }

            history_index++;
            
            // Move cursor to the end of the line before refreshing
            while (input.pos < input.len) {
                terminal.cursor.x++;
                if (terminal.cursor.x >= terminal.max.x) { 
                    terminal.cursor.x = 0; 
                    terminal.cursor.y++; 
                }
                input.pos++;
            }
            
            refresh_command_line();
            input.pos = input.len; 
            return; 
            
        case KEY_DOWN:
            // Early return if at the working draft
            if (history_index <= 0) return;

            history_index--;
            
            // Move cursor to the end of the line before refreshing
            while (input.pos < input.len) {
                terminal.cursor.x++;
                if (terminal.cursor.x >= terminal.max.x) { 
                    terminal.cursor.x = 0; 
                    terminal.cursor.y++; 
                }
                input.pos++;
            }
            
            refresh_command_line();
            input.pos = input.len;
            return;
            
        case KEY_LEFT:
            if (input.pos <= 0) return; 

            input.pos--;
            if (terminal.cursor.x > 0) {
                terminal.cursor.x--;
            } else {
                terminal.cursor.x = terminal.max.x - 1;
                terminal.cursor.y--;
            }
            return;

        case KEY_RIGHT:
            if (input.pos >= input.len) return; 

            input.pos++;
            terminal.cursor.x++;
            if (terminal.cursor.x >= terminal.max.x) {
                terminal.cursor.x = 0;
                terminal.cursor.y++;
            }
            return;

        case KEY_DELETE: 
            // Guard clause 1: if cursor is at the end of the line, nothing to delete
            if (input.pos >= input.len) return;
            
            cmd = getInput();
            // Guard clause 2: if reading the screen failed, abort safely
            if (!cmd) return;
            
            char new_cmd[MAX_INPUT_LEN];
            
            // 1. Copy everything BEFORE the cursor (unchanged)
            for (int i = 0; i < input.pos; i++) {
                new_cmd[i] = cmd[i];
            }
            
            // 2. Copy everything AFTER the cursor, shifting left by 1 to overwrite
            for (int i = input.pos + 1; i < input.len; i++) {
                new_cmd[i - 1] = cmd[i];
            }
            
            // 3. Redraw the updated line from the start
            remove_cursor_shape();
            terminal.cursor = input.start;
            
            for (int i = 0; i < input.len - 1; i++) {
                terminal.putchar(new_cmd[i]);
            }
            terminal.putchar(' '); // Erase the leftover character at the end
            
            // 4. Update the logical length (pos remains unchanged)
            input.len--;
            
            // 5. Restore the physical cursor to the exact same position
            terminal.cursor = input.start;
            for (int i = 0; i < input.pos; i++) {
                terminal.cursor.x++;
                if (terminal.cursor.x >= terminal.max.x) {
                    terminal.cursor.x = 0; 
                    terminal.cursor.y++;
                }
            }
            
            free(cmd);
            return;

        case KEY_HOME:
            // Move logical position and physical cursor to the start of the input
            input.pos = 0;
            terminal.cursor = input.start;
            return;

        case KEY_END:
            // Move logical position to the end
            input.pos = input.len;
            terminal.cursor = input.start;
            
            // Recalculate physical cursor position based on input length
            for (int i = 0; i < input.pos; i++) {
                terminal.cursor.x++;
                if (terminal.cursor.x >= terminal.max.x) {
                    terminal.cursor.x = 0; 
                    terminal.cursor.y++;
                }
            }
            return;

        case KEY_PAGE_UP:
            if (view_offset + view_scroll <= history_lines) {
                view_offset += view_scroll;
            } else {
                view_offset = history_lines;
            }
            terminal.redraw_all();
            return;

        case KEY_PAGE_DOWN:
            if (view_offset >= view_scroll) {
                view_offset -= view_scroll;
            } else {
                view_offset = 0; 
            }
            terminal.redraw_all();
            return;

        case KEY_INSERT:
        case KEY_LEFT_GUI:
        case KEY_RIGHT_GUI:
            return; 

        default: { 
            uint8_t mod_index = (altgr_active << 1) | shift_active;
            const unsigned char* layout_maps[4] = {
                current_layout->normal,      
                current_layout->shift,       
                current_layout->altgr,       
                current_layout->shift_altgr  
            };
            
            unsigned char ascii = layout_maps[mod_index][key];
            if (ascii == 0) return;
            
            switch (ascii) {
                case '\n':
                    // Reset camera to the present when hitting enter
                    view_offset = 0;
                    terminal.putchar('\n'); 
                    command_handler();
                    input.pos = 0; 
                    return;

                case '\b':
                    // Reset camera to the present when typing
                    view_offset = 0;

                    // Early return if there is nothing to delete
                    if (input.pos <= 0) return;

                    // Simple deletion at the end of the string
                    if (input.pos == input.len) {
                        terminal.putchar('\b'); 
                        input.len--;
                        input.pos--;
                        return; 
                    }
                        
                    // Mid-string deletion
                    cmd = getInput(); 
                    if (!cmd) return;

                    char new_cmd[MAX_INPUT_LEN];
                    for (int i = 0; i < input.pos - 1; i++) new_cmd[i] = cmd[i];
                    for (int i = input.pos; i < input.len; i++) new_cmd[i - 1] = cmd[i];
                        
                    remove_cursor_shape();
                    terminal.cursor = input.start;
                    for (int i = 0; i < input.len - 1; i++) terminal.putchar(new_cmd[i]);
                    terminal.putchar(' '); 
                        
                    input.len--;
                    input.pos--;
                        
                    terminal.cursor = input.start;
                    for (int i = 0; i < input.pos; i++) {
                        terminal.cursor.x++;
                        if (terminal.cursor.x >= terminal.max.x) {
                            terminal.cursor.x = 0; terminal.cursor.y++;
                        }
                    }
                    free(cmd);
                    return;

                case 32 ... 126:    
                case 128 ... 255:   
                    // Reset camera to the present when typing
                    view_offset = 0;

                    // Early return if input buffer is full
                    if (input.len >= MAX_INPUT_LEN - 1) return; 

                    // Simple insertion at the end of the string
                    if (input.pos == input.len) {
                        terminal.putchar(ascii); 
                        input.len++;
                        input.pos++;
                        return;
                    }
                    
                    // Mid-string insertion
                    cmd = getInput(); 
                    if (!cmd) return; 

                    char insert_cmd[MAX_INPUT_LEN];
                    for (int i = 0; i < input.pos; i++) insert_cmd[i] = cmd[i];
                    insert_cmd[input.pos] = ascii; 
                    for (int i = input.pos; i < input.len; i++) insert_cmd[i + 1] = cmd[i];
                            
                    remove_cursor_shape();
                    terminal.cursor = input.start;
                    for (int i = 0; i <= input.len; i++) terminal.putchar(insert_cmd[i]);
                            
                    input.len++;
                    input.pos++;
                            
                    terminal.cursor = input.start;
                    for (int i = 0; i < input.pos; i++) {
                        terminal.cursor.x++;
                        if (terminal.cursor.x >= terminal.max.x) {
                            terminal.cursor.x = 0; terminal.cursor.y++;
                        }
                    }
                    free(cmd);
                    return;
            }
            return;
        } 
    }
}

// ==========================================
// MAIN EVENT LOOP
// Parses raw hardware scancodes and tracks modifiers
// ==========================================
void process_keyboard_events() {
    uint8_t scancode;
    bool key_processed = false;
    
    while(kbd_pop(&scancode)) {
        
        key_processed = true;
        draw_cursor = false;
        remove_cursor_shape();

        // Handle special prefix scancodes (continue the while loop)
        if (scancode == 0xE0) { extended_scancode = true; continue; }
        if (scancode == 0xF0) { expecting_release = true; continue; }

        bool released = false;
        if (current_scancode_set == 1) {
            released = (scancode & 0x80) != 0;
            scancode = scancode & 0x7F;
        } else {
            released = expecting_release;
            expecting_release = false;
        }

        key_code_t key = KEY_UNKNOWN;
        if (current_scancode_set == 1) {
            if (extended_scancode) key = translate_set1_extended(scancode);
            else if (scancode < 0x80) key = (key_code_t)scancode;
        } else if (current_scancode_set == 2) {
            if (extended_scancode) key = translate_set2_extended(scancode);
            else key = ps2_set2_to_keycode[scancode];
        }

        extended_scancode = false;
        if (key == KEY_UNKNOWN) continue; 

        key_states[key] = !released;
        if (released) continue;

        bool shift_active = key_states[KEY_LEFT_SHIFT] || key_states[KEY_RIGHT_SHIFT];
        bool altgr_active = key_states[KEY_RIGHT_ALT];

        // Trigger the specific action for the pressed key
        process_key(key, shift_active, altgr_active);
    }
    
    // Visibility Check: Only allow cursor logic if we are not looking at history
    if (key_processed) {
        if (view_offset == 0) {
            draw_cursor = true;
            reset_cursor_blink();
        } else {
            // While scrolling history, keep the cursor logically OFF 
            // so putchar and other functions don't trigger it.
            draw_cursor = false; 
            if (cursor_visible) {
                remove_cursor_shape();
                cursor_visible = false;
            }
        }
    }
}