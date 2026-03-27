#include "cli/cmd/commands.h"
#include "mem/kheap.h"
#include "mem/pmm.h"
#include "idt/sound/sound.h"
#include "idt/mouse/mouse.h"
#include "idt/mouse/displayMouse.h"
#include "idt/rtc/rtc.h"
#include "idt/keyboard/keyboardLayout.h"





int8_t cmd_clear(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    

    terminal.clear();
    return true;
}

int8_t cmd_echo(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
    size_t i = 1; 
    while (i < MAX_COMMAND_ARGS && c[i][0] != '\0') {
        if (c[i][0] == '"' && c[i][strlen(c[i]) - 2] == '"') { 
            for (size_t j = 0; j < strlen(c[i]) - 2; j++) {
                c[i][j] = c[i][j + 1];
            }
           
        } 
        kprintf("%s ", c[i]);
        i++;
    }
    
    kprintf("\n"); 
    return true;
}

int8_t cmd_format_memory_info(uint32_t value_in_bytes, uint32_t format_hash) {
    
    switch(format_hash) {
        case hash("b"):
            kprintf("%d B", value_in_bytes);
            return true;
        case hash("kb"):
            kprintf("%f KB", (double)value_in_bytes / 1024.0);
            return true;
        default:
            kprintf("%f MB", (double)value_in_bytes / (1024.0 * 1024.0));
            return true;
        case hash("gb"):
            kprintf("%f GB", (double)value_in_bytes / (1024.0 * 1024.0 * 1024.0));
            return true;
    }
    return 2; 
}

int8_t cmd_check(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
    switch (hash(c[1])) {
        
        case hash("ram"): {

            uint32_t total_mb = get_total_memory_mb();
            uint32_t total_bytes = total_mb * 1024 * 1024;
            uint32_t used_bytes = pmm_get_allocated() + heap_get_allocated();
            uint32_t available_bytes = total_bytes - used_bytes;
            
            // Il formato desiderato è in c[3]
            uint32_t print_format = hash(c[3]); 

            switch (hash(c[2])) {
                case hash("all"):
                    kprintf("System RAM: %d MB(via CMOS)\n", total_mb);
                    kprintf("Used RAM: %f MB(via heap+pmm)\n", (double)used_bytes / (1024.0 * 1024.0));
                    kprintf("Available RAM: %f MB(via heap+pmm)\n", (double)available_bytes / (1024.0 * 1024.0));
                    return true;

                case hash("av"):
                    cmd_format_memory_info(available_bytes, print_format);
                    kprintf("\n"); 
                    return true;

                case hash("adv"):
                    kprintf("Available RAM:\t ");
                    cmd_format_memory_info(available_bytes, print_format);
                    kprintf("(");
                    cmd_format_memory_info(used_bytes, print_format); 
                    kprintf(" used)\n"); 
                    return true;

                case hash("us"):
                    
                    cmd_format_memory_info(used_bytes, print_format);
                    kprintf("\n"); 
                    return true;

                default:
                    kprintf("Total System RAM: %d MB(via CMOS)\n", total_mb);
                    return true;
            }

        } 

        case hash("cs"): {
            uint16_t cs;
            asm volatile ("mov %%cs, %0" : "=r"(cs));
            kprintf("CS = %x\n", cs);        
            return true;
        }

        case hash("stack"): {
            uint64_t rsp;
            asm volatile("mov %%rsp, %0" : "=r"(rsp));
            kprintf("RSP = %lx/%ld\n", rsp, rsp);      
            return true;
        }

        case hash("cpu"): {
            uint32_t ebx, ecx, edx;
            uint32_t unused;
            asm volatile("cpuid" : "=a"(unused), "=b"(ebx), "=d"(edx), "=c"(ecx) : "a"(0));

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

        case hash("mouse"): {
            kprintf("=== MOUSE STATUS ===\n");
            kprintf("Position : X = %d | Y = %d\n", mouse.x, mouse.y);
            kprintf("Clicks   : Left = %s | Right = %s | Middle = %s\n", 
                    mouse_left_click ? "YES" : "NO", 
                    mouse_right_click ? "YES" : "NO", 
                    mouse_middle_click ? "YES" : "NO");
            
            // Display the accumulated scroll wheel value (Z-axis)
            kprintf("Scroll   : Z = %d\n", mouse_scroll);
            
        }return true;
        

        case hash("ascii"):
            
            kprintf("ASCII Table (Code Page 437) - 8 Columns Layout:\n");

            // 1. Top border (8 columns, each 8 characters wide)
            // \xDA = ┌, \xC4 = ─, \xC2 = ┬, \xBF = ┐
            kprintf("\xDA"); 
            for (int c = 0; c < 8; c++) {
                kprintf("\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4"); // 8 horizontal lines
                if (c < 7) kprintf("\xC2");         // T-junction pointing down
            }
            kprintf("\xBF\n"); // Top-right corner

            // 2. Print the 32 rows
            for (int r = 0; r < 32; r++) {
                
                kprintf("\xB3"); // Leftmost vertical border (│)
                
                for (int c = 0; c < 8; c++) {
                    // Calculate the actual ASCII value based on column (0-7) and row (0-31)
                    int i = (c * 32) + r;
                    
                    // Filter out control characters that would break the terminal layout
                    char display_c = (i == 0 || i == '\b' || i == '\t' || i == '\n' || i == '\r') ? '.' : (unsigned char)i;
                    
                    // Print with luxurious spacing:
                    // " %3d: %c " = 1 space + 3 nums + 2 (": ") + 1 char + 1 space = 8 chars
                    kprintf(" %3d: %c \xB3", i, display_c);
                }
                kprintf("\n");
            }

            // 3. Print the bottom border
            // \xC0 = └, \xC1 = ┴, \xD9 = ┘
            kprintf("\xC0"); 
            for (int c = 0; c < 8; c++) {
                kprintf("\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4"); // 8 horizontal lines
                if (c < 7) kprintf("\xC1");         // T-junction pointing up
            }
            kprintf("\xD9\n"); // Bottom-right corner

            
        return true;

        case hash("logs"):

            for (size_t i=0;i<log_index;i++)
                kprintf("%s\n", klogs[i]);

        return true;
        default: return 2;
    }

    return 2;
}

int8_t cmd_cursor(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {

   
    if (strcmp(c[1], "blink") == 0) {
        if (strcmp(c[2], "enable") == 0) {
            kprintf("Cursor blinking enabled\n");
            cursor_blink = true;
            return true;
        }
        if (strcmp(c[2], "disable") == 0) {
            kprintf("Cursor blinking disabled\n");
            cursor_blink = false;
            return true;
        }
    }

    uint8_t cs = atoi(c[1]);
    if (cs < 3) {
        cursor_shape = cs;
        return true;
    }
    return 2;
}

int8_t cmd_color(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {

    switch (hash(c[1])) {
        // ==========================================
        // Palette Type Command (e.g., "color type 1")
        // ==========================================
        case hash("type"): {
            switch(htoi(c[2])) {
                case 0: current_palette = DEFAULT; break;
                case 1: current_palette = SOFT; break;
                case 2: current_palette = SOLARIZED; break;
                case 3: current_palette = CUSTOM; break;
                default: return 2; // Early return for invalid type
            }
            return true;
        }

        // ==========================================
        // RGB Command (e.g., "color rgb 000000 FFFFFF")
        // ==========================================
        case hash("rgb"): {
            uint32_t bg = htoi(c[2]);
            uint32_t fg = htoi(c[3]);
            
            // Early Return: Background and Foreground cannot be the same
            if (bg == fg) {
                kprintf("Background and Foreground cannot be the same!\n");
                return 2;
            }
            
            // Check if we are coloring the info bar
            if (hash(c[4]) == hash("info")) {
                info_bar_window.change_color(terminal_color(fg, bg));
                return true;
            }
            
            // Otherwise, color the main terminal
            terminal.change_color(terminal_color(fg, bg));
            return true;
        }

        // ==========================================
        // Theme & Predefined Colors Command (e.g., "color theme matrix" or "color theme red")
        // ==========================================
        case hash("theme"): {
            // Early Return 1: Check if the user provided an argument 
            // c[0]="color", c[1]="theme", c[2]=<name>
            if (!c[2] || c[2][0] == '\0') {
               
                return 2;
            }

            // 1. Try to load it as a syntax highlighting theme first
            if (load_theme(c[2])) {
                // Force the terminal to rescan the RAM buffer and re-apply colors
                // We pass the current base color so the background stays the same
                terminal.change_color(terminal.color); 
                
                kprintf("Theme '%s' applied successfully!\n", c[2]);
                return true;
            }

           
            return 2;
            
        }

        // ==========================================
        // Standard VGA Attribute Command (e.g., "color 0A")
        // ==========================================
        default: {
            // Parse as an integer first to validate bounds safely
            int32_t raw_attr = htoi(c[1]);
            
            // Early Return 1: Check if the color is within 0-255 range
            if (raw_attr < 0 || raw_attr > 255) { 
                kprintf("Invalid Color.\n");
                return 2;
            }

            // Safe cast now that we know it's within bounds
            uint8_t attr = (uint8_t)raw_attr;
            
            // Early Return 2: Background and Foreground cannot be the same
            if ((attr >> 4) == (attr & 0x0F)) {
                kprintf("Background and Foreground cannot be the same!\n");
                return 2;
            }

            // Check if we are coloring the info bar
            if (hash(c[2]) == hash("info")) {
                info_bar_window.color = terminal_color(vga_palette[current_palette][(attr >> 4) & 0x0F], vga_palette[current_palette][attr & 0x0F]);
                return true;
            }

            // Otherwise, apply to the main terminal
            terminal.change_color(terminal_color(vga_palette[current_palette][(attr >> 4) & 0x0F], vga_palette[current_palette][attr & 0x0F]));
            return true;
        }
    }
    
    return 2;
}

int8_t cmd_beep(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    play_sound(750);
    sleep(1000);
    nosound();
    kprintf("Beep activated! type 'nosound' to stop it.\n");
    return true;
}

int8_t cmd_mouse(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
    if (strcmp(c[1], "disable") == 0) {
        mouse_active = false;
        kprintf("Mouse disabled\n");

        return true;
    } 


    if (strcmp(c[1], "enable") == 0) {
        mouse_active = true;
            
        first_mouse_draw = true; 
        kprintf("Mouse enabled\n");


        return true;
    } 
    
  
    
    return 2;
    
}

int8_t cmd_time(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
    rtc_time_t t;
    read_rtc(&t);
    apply_timezone(&t);

    switch (hash(c[1])) {
        case hash("all"):     kprintf("%02d/%02d/%d - %02d:%02d:%02d\n", t.day, t.month, t.year, t.hour, t.minute, t.second); return true;
        case hash("date"):    kprintf("%02d/%02d/%d\n", t.day, t.month, t.year); return true;
        case hash("clock"):   kprintf("%02d:%02d\n", t.hour, t.minute); return true;
        case hash("clockex"): kprintf("%02d:%02d:%02d\n", t.hour, t.minute, t.second); return true;
        case hash("day"):     kprintf("%02d\n", t.day); return true;
        case hash("month"):   kprintf("%02d\n", t.month); return true;
        case hash("year"):    kprintf("%d\n", t.year); return true;
        case hash("hour"):    kprintf("%02d\n", t.hour); return true;
        case hash("minute"):  kprintf("%02d\n", t.minute); return true;
        case hash("second"):  kprintf("%02d\n", t.second); return true;

        case hash("zone"): {
            if (c[2][0] == '\0') {
                kprintf("Current timezone: UTC ");
                if (system_timezone >= 0) kprintf("+");
                kprintf("%d\n", system_timezone);
                return true;
            }

            int8_t new_tz = (int8_t)atoi(c[2]);
            if (new_tz < -12 || new_tz > 14) {
                kprintf("Invalid timezone. Must be between -12 and +14.\n");
                return 2;
            }        

            system_timezone = new_tz;
            kprintf("Timezone updated to UTC");
            if (system_timezone >= 0) kprintf("+");
            kprintf("%d\n", system_timezone);
            
            return true;
        }

        default: return 2;
    }

    return 2;
}

int8_t cmd_reboot(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    kprintf("Restarting...\n");
    outb(0x64, 0xFE); 
    return true;
}

int8_t cmd_shutdown(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    kprintf("Shutting down system...\n");

    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);
    outw(0x604, 0x2000);

    kprintf("ACPI shutdown not supported on this hardware.\n");
    kprintf("System halted safely. You may now turn off the power.\n");
    
    __asm__ __volatile__("cli; hlt");  
    return true;
}

int8_t cmd_keyboard(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
    switch(hash(c[1])) {
        case hash("it"): current_layout = &kbd_IT; return true;
        case hash("us"): current_layout = &kbd_US; return true;
        default: return 2;
    }
    
    
    return 2;
}


int8_t cmd_scroll(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {

    uint8_t num = (uint8_t)atoi(c[1]);
    if (num < 0 && num > 255) return 2;
    
    
    view_scroll = num;
    return true;
}



int8_t cmd_fault(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {


    int *a;
    a=(int*)0x67;
    *a = 7;


    return true;
}



int8_t cmd_help(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]);


command_t commands[] = {
    {"help",        cmd_help,       "Provides help information for commands",   "[command]"},
    {"clear",       cmd_clear,      "Clears the screen",                        ""},
    {"echo",        cmd_echo,       "Prints text to screen",                    "[text]"},
    {"check",       cmd_check,      "Checks system info or prints tables",      "[option] [format]\n\toptions: ram [av/us/adv/all], cs, stack, cpu, mouse, ascii\n\tformat (for ram): b, kb, mb, gb"},
    {"cursor",      cmd_cursor,     "Changes cursor shape or blink state",      "[0-2] / blink [enable/disable]"},
    {"color",       cmd_color,      "Changes terminal colors or theme",         "[attr] [info] / rgb [bg] [fg] [info] / theme [name] / type [0-3]\n\tattr: 0x0F\n\tbg/fg: 0xRRGGBB\n\tthemes: dracula, matrix, ocean, disable"},
    {"beep",        cmd_beep,       "Plays a test sound",                       ""},
    {"mouse",       cmd_mouse,      "Enables or disables the mouse",            "enable / disable"},
    {"time",        cmd_time,       "Displays current date and time",           "[option]\n\toptions: all, date, clock, clockex, day, month, year, hour, minute, second, zone [offset]"},
    {"reboot",      cmd_reboot,     "Reboots the system",                       ""},
    {"shutdown",    cmd_shutdown,   "Halts the system and powers off",          ""},
    {"keyboard",    cmd_keyboard,   "Changes keyboard layout",                  "it / us"},
    {"scroll",      cmd_scroll,     "Changes amount of rows for scroll",        "[0-255]"},
    {"fault",       cmd_fault,      "Page Faults",                              ""},

    {"lastindex",   NULL,           "i don't want to have to put a , when doing SHIFT+ALT+DOWN", ""}
};

size_t num_commands = sizeof(commands) / sizeof(command_t);
int8_t cmd_help(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
    if (c[1][0] == '\0') {
        kprintf("Command List:\n");
        for (size_t i=0;i<num_commands;i++)
        kprintf(" %s\t%s\n",commands[i].name, commands[i].usage);
        return true;
    } else {
        for (size_t i=0;i<num_commands;i++) {
            if (strcmp(c[1], commands[i].name) == 0) {
                kprintf("%s - %s\nUsage: %s %s\n", commands[i].name, commands[i].desc, commands[i].name, commands[i].usage);
                return true;
            }
        }
        
        
    }

    
    
    return 2;
}


int8_t execute_command(char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    size_t num_commands = sizeof(commands) / sizeof(command_t);
    
  

    for (size_t i = 0; i < num_commands; i++) 
        if (strcmp(c[0], commands[i].name) == 0) {
            if (commands[i].func == NULL) {
                kprintf("Command '%s' is registered but not implemented yet!\n", commands[i].name);
                return true; 
            }
            return commands[i].func(c);
        }
    return false;
}