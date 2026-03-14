#include "idt/keyboard/keyboard.h"


bool shift_pressed = false;
bool altgr_pressed = false;
bool extended_scancode = false;
char history[MAX_HISTORY][MAX_COMMAND_LEN]; 
int8_t history_count=0;
int8_t history_index=0;



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

void handle_character_input(char c) {
    if (input_len < MAX_INPUT_LEN - 1) {
        for (size_t i = input_len; i > input_pos; i--) {
            input_buffer[i] = input_buffer[i - 1];
        }
        
        input_buffer[input_pos] = c;
        input_len++;
        input_buffer[input_len] = '\0';

        point old;
        old.x = terminal_data.cursor.x;
        old.y = terminal_data.cursor.y;

        terminal_render_from_buffer(input_pos); 

        terminal_set_cursor(&terminal_data, point(old.x + 1, old.y));
        column_behaviour(&terminal_data); 

        input_pos++;
    }
}

void process_keyboard_events() {
    uint8_t scancode;
    
    
    while(kbd_pop(&scancode)) {
        
        draw_cursor=false;
        remove_cursor_shape();

        if (scancode == 0xE0) {
            extended_scancode = true;
            continue; 
        }

        if (extended_scancode) {
            extended_scancode = false;
            
            switch (scancode) {
                case 0x48: // up arrow
                    if (history_count > 0 && history_index < history_count - 1) {
                        history_index++;
                        refresh_command_line();
                    }
                break;
                case 0x50: // down arrow
                    if (history_index >= 0) {
                        history_index--;
                        refresh_command_line();
                    }
                break;

                case 0x4B: // left arrow
                    if (input_pos > 0) {
                        
                        input_pos--;
                        terminal_data.cursor.x--;
                        column_behaviour(&terminal_data);
                    }
                break;

                case 0x4D: // right arrow
                    if (input_pos < input_len) {
                        input_pos++;
                        terminal_data.cursor.x++;
                        column_behaviour(&terminal_data);
                    }
                break;

                case 0x38: altgr_pressed = true; break;  // AltGr pressed (0x38) 
                case 0xB8: altgr_pressed = false; break; // AltGr released (0x38 | 0x80)
                default:
                break;
            }
            continue;
        }

        if ((scancode & 0x7F) == 0x2A || (scancode & 0x7F) == 0x36) {
            shift_pressed = !(scancode & 0x80);
            continue;
        }

        if (scancode & 0x80) {
            continue;
        }

        unsigned char ascii = 0;
        if (altgr_pressed) {
            switch (scancode) {
                case 0x1A: ascii = shift_pressed ? '{' : '['; break; 
                case 0x1B: ascii = shift_pressed ? '}' : ']'; break; 
                case 0x27: ascii = shift_pressed ? (unsigned char)135 : '@'; break; 
                case 0x2B: ascii = shift_pressed ? (unsigned char)248 : '#'; break; 
            }
        } else ascii = shift_pressed ? kbd_IT_shift[scancode] : kbd_IT[scancode];
        
        if (ascii==0) {
            continue;
        }

        if (ascii >= 32 && ascii <= 126)  
            handle_character_input(ascii); 

        if (ascii=='\n') {
            terminal_putchar('\n'); 
            command_handler();
            continue;
        }

        if (ascii == '\b' && input_pos > 0) {
            
            for (size_t i = input_pos - 1; i < input_len - 1; i++) {
                input_buffer[i] = input_buffer[i + 1];
            }
            input_len--;
            input_pos--;
            input_buffer[input_len] = '\0'; 

            terminal_putchar('\b'); 
            point saved = terminal_data.cursor;
                
            terminal_render_from_buffer(input_pos); 

            int chars_printed = input_len - input_pos;
            
            int erase_x = saved.x + chars_printed;

            draw_rect(erase_x * 8, saved.y * 16, 8, 16, terminal_data.color.bg); 
            
            terminal_data.cursor = saved;
        }
    }

    draw_cursor=true;
}