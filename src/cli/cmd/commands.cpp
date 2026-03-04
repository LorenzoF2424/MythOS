#include "cli/cmd/commands.h"
#include "mem/kheap.h"
#include "mem/pmm.h"
#include "idt/sound/sound.h"

int8_t cmd_clear(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    terminal_clear();
    return true;
}

int8_t cmd_echo(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    kprintf("%s\n", input_buffer + 5);
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
                
                kprintf("Available RAM: %d B(%d B used)\n", get_total_memory_mb()*1024*1024 - (heap_get_allocated()+pmm_get_allocated()),(heap_get_allocated()+pmm_get_allocated()));
                return true;
            }

            kprintf("Available RAM: %f MB(%f B used)\n", double(get_total_memory_mb()) - (used_mb_float),used_mb_float);
            return true;
        }
        if (strcmp(c[2], "us") == 0) {
            
            kprintf("Used RAM: %f MB(via heap+pmm)\n", used_mb_float);
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
    return 2;
}

int8_t cmd_cursor(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    uint8_t cs = atoi(c[1]);
    if (cs >= 0 && cs < 3) {
        cursor_shape = cs;
        return true;
    }
    return 2;
}

int8_t cmd_color(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    if (strcmp(c[1], "rgb") == 0) {
        uint32_t bg = htoi(c[2]);
        uint32_t fg = htoi(c[3]);
        change_terminal_color(fg, bg);
        terminal_clear(); 
        return true;
    }
    uint8_t attr = (uint8_t)htoi(c[1]);
    if (attr > 0 && attr < 255) {
        change_terminal_color(vga_palette[attr & 0x0F], vga_palette[(attr >> 4) & 0x0F]);
        terminal_clear(); 
        return true;
    }
    return 2;
}

int8_t cmd_beep(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    play_sound(750);
    sleep(1000);
    kprintf("Beep activated! type 'nosound' to stop it.\n");
    return true;
}






command_t commands[] = {
    {"clear",   cmd_clear,   "Clears the screen"},
    {"echo",    cmd_echo,    "Prints text to screen"},
    {"check",   cmd_check,   "Checks ram, cs, stack, or cpu"},
    {"cursor",  cmd_cursor,  "Changes cursor shape"},
    {"color",   cmd_color,   "Changes terminal color"},
    {"beep",    cmd_beep,    "sound"},



    {"cls",   cmd_clear,   "Clears the screen"}
    
};


int8_t execute_command(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]) {
    size_t num_commands = sizeof(commands) / sizeof(command_t);
    
    if (strcmp(c[0],"help") == 0) {
        for (size_t i = 0; i < num_commands; i++) 
            if (strcmp(c[1], commands[i].name) == 0) 
                kprintf("%s\n",commands[i].help);
        return true;
    }

    for (size_t i = 0; i < num_commands; i++) 
        if (strcmp(c[0], commands[i].name) == 0) 
            return commands[i].func(c);
        
    return false;
}