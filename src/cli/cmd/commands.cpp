#include "cli/cmd/commands.h"
#include "mem/kheap.h"
#include "mem/pmm.h"
#include "idt/sound/sound.h"
#include "idt/mouse/mouse.h"
#include "idt/mouse/displayMouse.h"
#include "idt/rtc/rtc.h"





int8_t cmd_clear(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    terminal_clear();
    return true;
}

int8_t cmd_echo(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
    size_t i = 1; 
    while (i < MAX_COMMAND_ARGS && c[i][0] != '\0') {
        kprintf("%s ", c[i]);
        i++;
    }
    
    kprintf("\n"); 
    return true;
}

int8_t cmd_check(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    if (strcmp(c[1], "ram") == 0) {


        uint32_t total = get_total_memory_mb();
        uint32_t used_bytes = pmm_get_allocated() + heap_get_allocated();

        double used_mb_float = (double)used_bytes / (1024.0 * 1024.0);

        if (strcmp(c[2], "all") == 0) {
            kprintf("System RAM: %d MB(via CMOS)\n", get_total_memory_mb());
            kprintf("Used RAM: %f MB(via heap+pmm)\n", used_mb_float);
            kprintf("Available RAM: %f MB(via heap+pmm)\n", double(get_total_memory_mb()) - (used_mb_float));
            return true;
        }

        if (strcmp(c[2], "av") == 0) {
            
            if (strcmp(c[3], "b") == 0) {
                
                kprintf("%d B\n", get_total_memory_mb()*1024*1024 - (used_bytes),(used_bytes));
                return true;
            }

            kprintf("Available RAM:\t %f MB\n", double(get_total_memory_mb()) - (used_mb_float),used_mb_float);
            return true;
        }

        if (strcmp(c[2], "adv") == 0) {
            
            if (strcmp(c[3], "b") == 0) {
                
                kprintf("Available RAM: %d B(%d B used)\n", get_total_memory_mb()*1024*1024 - (heap_get_allocated()+pmm_get_allocated()),(heap_get_allocated()+pmm_get_allocated()));
                return true;
            }

            kprintf("Available RAM: %f MB(%f MB used)\n", double(get_total_memory_mb()) - (used_mb_float),used_mb_float);
            return true;
        }
        if (strcmp(c[2], "us") == 0) {
            
            if (strcmp(c[3], "b") == 0) {
                
                kprintf("%d B\n", used_bytes);
                return true;
            }

            kprintf("Used RAM:\t\t %f MB\n", used_mb_float);
            return true;
        }












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


    if (strcmp(c[1], "mouse") == 0) {
        kprintf("=== MOUSE STATUS ===\n");
        kprintf("Position : X = %d | Y = %d\n", mouse.x, mouse.y);
        kprintf("Clicks   : Left = %s | Right = %s | Middle = %s\n", 
                mouse_left_click ? "YES" : "NO", 
                mouse_right_click ? "YES" : "NO", 
                mouse_middle_click ? "YES" : "NO");
        return true;
    }




    return 2;
}

int8_t cmd_cursor(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {

   
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



int8_t cmd_color(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    if (strcmp(c[1], "rgb") == 0) {
        uint32_t bg = htoi(c[2]);
        uint32_t fg = htoi(c[3]);
        change_terminal_color(terminal_color(fg, bg));
         
        return true;
    }
    uint8_t attr = (uint8_t)htoi(c[1]);
    if (attr > 0 && attr < 255) {
        change_terminal_color(terminal_color(vga_palette[attr & 0x0F], vga_palette[(attr >> 4) & 0x0F]));
         
        return true;
    }
    return 2;
}

int8_t cmd_beep(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    play_sound(750);
    sleep(1000);
    nosound();
    kprintf("Beep activated! type 'nosound' to stop it.\n");
    return true;
}

int8_t cmd_mouse(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
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

int8_t cmd_time(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    rtc_time_t t;
    read_rtc(&t);
    apply_timezone(&t);

    if (strcmp(c[1], "all") == 0) {
        kprintf("%02d/%02d/%d - %02d:%02d:%02d\n", 
                t.day, t.month, t.year, t.hour, t.minute, t.second);
        return true;
    }

    if (strcmp(c[1], "date") == 0) {
        kprintf("%02d/%02d/%d\n", t.day, t.month, t.year);
        return true;
    }
    
    
    if (strcmp(c[1], "clock") == 0) {
        kprintf("%02d:%02d\n", t.hour, t.minute);
        return true;
    }


    if (strcmp(c[1], "clockex") == 0) {
        kprintf("%02d:%02d:%02d\n", t.hour, t.minute, t.second);
        return true;
    }

    

    if (strcmp(c[1], "day") == 0) {
        kprintf("%02d\n", t.day);
        return true;
    }
    
    if (strcmp(c[1], "month") == 0) {
        kprintf("%02d\n", t.month);
        return true;
    }
    
    if (strcmp(c[1], "year") == 0) {
        kprintf("%d\n", t.year);
        return true;
    }
    
    if (strcmp(c[1], "hour") == 0) {
        kprintf("%02d\n", t.hour);
        return true;
    }
    
    if (strcmp(c[1], "minute") == 0) {
        kprintf("%02d\n", t.minute);
        return true;
    }
    
    if (strcmp(c[1], "second") == 0) {
        kprintf("%02d\n", t.second);
        return true;
    }

    
    if (strcmp(c[1], "zone") == 0) {
        int8_t new_tz = (int8_t)atoi(c[2]);
        if (new_tz < -12 || new_tz > 14) {
            return 2;
        }        

        if (c[2][0] == '\0') {
            kprintf("Current timezone: UTC ");
            if (system_timezone >= 0) kprintf("+");
            kprintf("%d\n", system_timezone);
            return true;
        }


        system_timezone = new_tz;
        kprintf("Timezone updated to UTC");
        if (system_timezone >= 0) kprintf("+");
        kprintf("%d\n", system_timezone);
        
        return true;
    }

    return 2;
}


int8_t cmd_reboot(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    kprintf("Restarting...\n");
    outb(0x64, 0xFE); 
    return true;
}

int8_t cmd_shutdown(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    kprintf("Shutting down system...\n");

    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);
    outw(0x604, 0x2000);

    kprintf("ACPI shutdown not supported on this hardware.\n");
    kprintf("System halted safely. You may now turn off the power.\n");
    
    __asm__ __volatile__("cli; hlt");  
    return true;
}



int8_t cmd_help(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]);


command_t commands[] = {
    {"help",      cmd_help,   "Provides help information for commands", "[command]"},
    {"clear",     cmd_clear,  "Clears the screen",                      ""},
    {"echo",      cmd_echo,   "Prints text to screen",                  "[text]"},
    {"check",     cmd_check,  "Checks ram, cs, stack, or cpu",          "[ram/cs/stack/cpu] [av/us/all] [b]"},
    {"cursor",    cmd_cursor, "Changes cursor shape",                   "[0-2]"},
    {"color",     cmd_color,  "Changes terminal color",                 "[attr] or rgb [bg] [fg]\n\tattr: 0x0F (hex) where lower nibble is fg and higher nibble is bg\n\tbg/fg: 0xRRGGBB (hex)"},
    {"beep",      cmd_beep,   "sound",                                  ""},
    {"mouse",     cmd_mouse,  "Enables or disables mouse",              "[enable/disable]"},
    {"time",      cmd_time,   "Displays current date and time",         ""},
    {"reboot",    cmd_reboot,   "Reboots the system",                     ""},
    {"shutdown",  cmd_shutdown, "Halts the system and powers off",        ""},


    {"lastindex",   NULL,   "i don't want to have to add the ',' when shift+alt+downing the element of the array", ""}
    
};

size_t num_commands = sizeof(commands) / sizeof(command_t);
int8_t cmd_help(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    
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


int8_t execute_command(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
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