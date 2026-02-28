#include "IDT.h"


idt_entry_t idt[256];
idt_ptr_t idt_ptr;
char history[MAX_HISTORY][64000]; // Usando MAX_INPUT_LEN
int8_t history_count=0;
int8_t history_index=0;
bool shift_pressed=false;
bool altgr_pressed=false;
bool extended_scancode=false;



void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {

    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_mid  = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].base_high = (uint32_t)((base >> 32) & 0xFFFFFFFF);
    
    idt[num].sel = sel;
    idt[num].ist = 0;
    idt[num].flags = flags; 
 
    idt[num].always0 = 0;
}

void init_idt() {
    idt_ptr.limit = sizeof(struct idt_entry_t) * 256 - 1;
    idt_ptr.base  = (uint64_t)&idt;

   
    uint8_t *idt_raw = (uint8_t*)&idt;
    for (size_t i = 0; i < sizeof(struct idt_entry_t) * 256; i++) {
        idt_raw[i] = 0;
    }

    
    idt_flush((uint64_t)&idt_ptr);
}

void remap_pic() {
    
    outb(PIC1_COMMAND, 0x11);
    outb(PIC2_COMMAND, 0x11);

    
    outb(PIC1_DATA, 0x20); 
    outb(PIC2_DATA, 0x28); 

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);

  
    outb(PIC1_DATA, 0xFD); 
    outb(PIC2_DATA, 0xFF);
}

void init_keyboard() {
    remap_pic();
    init_idt(); 
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    idt_set_gate(33, (uint64_t)keyboard_isr, cs, 0x8E);
    asm volatile ("sti");
}

void prepare_for_next_command() {

    strinit(input_buffer,MAX_INPUT_LEN); 
    input_len=0;
    input_pos=0;


    kprintf("MythicOS>");        
    terminal_set_input_limit();  
}

int8_t execute_command(const char c[MAX_COMMAND_ARGS][256]) {
    
    if (strcmp(c[0], "clear") == 0) {
        terminal_clear();
        return true;
    } 

    if (strcmp(c[0], "echo") == 0) {
        kprintf("%s\n", input_buffer+5);
        return true;
    }

    
    if (strcmp(c[0], "check") == 0) {

        if (strcmp(c[1], "ram") == 0) {
            kprintf("Total System RAM: %d MB(via CMOS)\n", get_total_memory_mb());
            return true;
        }
        if (strcmp(c[1], "cs") == 0) {
            uint16_t cs;
            asm volatile ("mov %%cs, %0" : "=r"(cs));
            kprintf("CS = %x\n", cs);        
            return true;
        }
        if (strcmp(c[1], "stack") == 0) {
            uint64_t rsp;
            asm volatile("mov %%rsp, %0" : "=r"(rsp));
            kprintf("RSP = %lx/%ld\n", rsp, rsp);      
            return true;
        }


        if (strcmp(c[1], "cpu") == 0) {
            uint32_t ebx, ecx, edx;
            uint32_t unused;
            asm volatile("cpuid" 
                        : "=a"(unused), "=b"(ebx), "=d"(edx), "=c"(ecx) 
                        : "a"(0));

            char vendor[13];
            *(uint32_t*)(vendor) = ebx;
            *(uint32_t*)(vendor + 4) = edx;
            *(uint32_t*)(vendor + 8) = ecx;
            vendor[12] = '\0';

            kprintf("CPU Vendor ID: %s\t\t\t", vendor);
            
            uint32_t eax1, ecx1;
            asm volatile("cpuid" : "=a"(eax1), "=c"(ecx1) : "a"(1) : "ebx", "edx");
            kprintf("Features: %s %s\n", 
                    (ecx1 & (1 << 0)) ? "SSE3" : "No-SSE3",
                    (ecx1 & (1 << 31)) ? "Hypervisor" : "Physical");
            
            return true;
        }

        //kprintf("check cpu/ram/stack/cs\n");
        return 2;
    }



    if (strcmp(c[0], "cursor") == 0) {

        uint8_t cs=atoi(c[1]);
        if (cs>=0 && cs<3) {

            cursor_shape=cs;
            return true;
        }


        
        return 2;
    }

    if (strcmp(c[0], "color") == 0) {

        if (strcmp(c[1], "rgb") == 0) {

            uint32_t bg = htoi(c[2]);
            uint32_t fg = htoi(c[3]);
            

            change_terminal_color(fg, bg);
            terminal_clear(); 
            return true;
            

            return 2;
        }
        uint8_t attr = (uint8_t)htoi(c[1]);
        if (attr >0 && attr <255) {

            change_terminal_color(vga_palette[attr & 0x0F], vga_palette[(attr >> 4) & 0x0F]);
            terminal_clear(); 
            return true;
        }

     
        
        return 2;
    }


    if (strcmp(c[0], "beep") == 0) {
        play_sound(750); 
      
        kprintf("Beep activated! type 'nosound' to stop it.\n");
        return true;
    }   

    if (strcmp(c[0], "nosound") == 0) {
        nosound();
        return true;
    }

    //-----------------------DEFAULT(NOT FOUND)-----------------------------------------
    return false;
        
}

void sysCommand(const char *command) {
    if (command == NULL || command[0] == '\0') return;

    //-------------------------------SPECIAL CASES------------------------------------------
    if (strcmp(command, "reboot") == 0) {
        kprintf("Restarting...\n");
        outb(0x64, 0xFE); 
    }


    //-------------------------------FINDING ARGS------------------------------------------
    char input_buffer_temp[MAX_INPUT_LEN];
    strcpy(input_buffer_temp, command);
    
    char args[MAX_COMMAND_ARGS][256];
    for(int i=0; i < MAX_COMMAND_ARGS; i++) args[i][0] = '\0';

    uint8_t num_args=0;
    char* token = strtok(input_buffer_temp, ' ');
    
    while (token != NULL) {
        if (num_args>= MAX_COMMAND_ARGS) {
            kprintf("Error: Too many arguments.\n");
            return;
        }
        strcpy(args[num_args++], token);
        token = strtok(NULL, ' ');
    }

    if (num_args<=0) return;
    switch (execute_command(args)) {

        case 0: kprintf("%s: command not found\n", args[0]); break;
        case 2: kprintf("%s: not enough parameters\n", args[0]); break;


    } 
    
    
}

void sysCommandAt(const char *command, uint16_t x, uint16_t y) {

    uint16_t tempx = terminal_data.cursor_col;
    uint16_t tempy = terminal_data.cursor_row;
    terminal_set_cursor(&terminal_data, x, y);
    sysCommand(command);
    terminal_set_cursor(&terminal_data, tempx, tempy);


}

void command_handler() {
    input_buffer[input_len] = '\0'; 

    if (input_len > 0) {
       
        for (int i = MAX_HISTORY - 1; i > 0; i--) {
            for (int j = 0; j < MAX_INPUT_LEN; j++) {
                history[i][j] = history[i-1][j];
            }
        }
        strcpy(history[0], input_buffer);
        if (history_count < MAX_HISTORY) history_count++;
    }
    
    history_index = -1; 

   
    sysCommand(input_buffer);
}

void refresh_command_line() {
    
    while (input_pos < input_len) {
        terminal_data.cursor_col++;
        column_behaviour(&terminal_data);
        input_pos++;
    }

    while (input_len > 0) {
        terminal_putchar('\b'); 
        input_len--;
        input_pos--;            
    }

    if (history_index == -1) {
        input_buffer[0] = '\0';
        return;
    }

    for (int i = 0; history[history_index][i] != '\0'; i++) {
        input_buffer[i] = history[history_index][i];
        terminal_putchar(input_buffer[i]); 
        input_len++;
        input_pos++; 
    }
    
    // Chiudi correttamente la stringa
    input_buffer[input_len] = '\0'; 
}

void handle_character_input(char c) {
    if (input_len < MAX_INPUT_LEN - 1) {
        for (size_t i = input_len; i > input_pos; i--) {
            input_buffer[i] = input_buffer[i - 1];
        }
        
        input_buffer[input_pos] = c;
        input_len++;
        input_buffer[input_len] = '\0';

        uint16_t old_col = terminal_data.cursor_col;
        uint16_t old_row = terminal_data.cursor_row;

        terminal_render_from_buffer(input_pos); 

        terminal_set_cursor(&terminal_data, old_col + 1, old_row);
        column_behaviour(&terminal_data); 

        input_pos++;
    }
}

extern "C" void keyboard_handler_c() {
    
    uint8_t scancode = inb(0x60);


    draw_cursor=false;
    remove_cursor_shape();

    if (scancode == 0xE0) {
        extended_scancode = true;
        outb(0x20, 0x20);
        return; 
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
                    
                    remove_cursor_shape();
                    input_pos--;
                    terminal_data.cursor_col--;
                    column_behaviour(&terminal_data);
                    
                }
            break;

            case 0x4D: // right arrow
                if (input_pos < input_len) {
                    
                    
                    input_pos++;
                    terminal_data.cursor_col++;
                    column_behaviour(&terminal_data);
                   
                }
            break;

            case 0x38: altgr_pressed = true; break;  // AltGr pressed (0x38) 
            case 0xB8: altgr_pressed = false; break; // AltGr released (0x38 | 0x80)
            default:

            break;
        }
        outb(0x20, 0x20);
       
        return;
    }

    if ((scancode & 0x7F) == 0x2A || (scancode & 0x7F) == 0x36) {
        shift_pressed = !(scancode & 0x80);
        outb(0x20, 0x20);
        return;
    }

    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
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
        outb(0x20, 0x20);
        return;
    }

    if (ascii >= 32 && ascii <= 126)  
        handle_character_input(ascii); 


    if (ascii=='\n') {
       
        terminal_putchar('\n'); 
        command_handler();
        prepare_for_next_command();
        outb(0x20, 0x20);
        return;
    }

    if (ascii=='\b') {
      
        if (input_pos > 0) {
            for (size_t i = input_pos - 1; i < input_len - 1; i++) {
                input_buffer[i] = input_buffer[i + 1];
            }
            input_len--;
            input_pos--;
            input_buffer[input_len] = '\0'; 

            terminal_putchar('\b'); 

            uint16_t saved_col = terminal_data.cursor_col;
            uint16_t saved_row = terminal_data.cursor_row;

            terminal_render_from_buffer(input_pos); 

            draw_rect(terminal_data.cursor_col * 8, terminal_data.cursor_row * 16, 8, 16, terminal_data.color_bg); 
            terminal_data.cursor_col = saved_col;
            terminal_data.cursor_row = saved_row;
        }
        outb(0x20, 0x20);
        return;
    }

    
    

    outb(0x20, 0x20);
    
}


