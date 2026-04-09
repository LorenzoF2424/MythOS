#include "cli/commands.h"
#include "idt/keyboard/keyboard.h" 
#include "idt/sound/sound.h"



// ==========================================
// Terminal & Display Commands
// ==========================================

int8_t cmd_clear(int argc, char **argv) {
    terminal.clear();
    return true;
}

int8_t cmd_echo(int argc, char **argv) {
    // Early return: nothing to echo
    if (argc < 2) {
        kprintf("\n");
        return true;
    }

    // Print every argument after "echo" separated by a space
    for (int i = 1; i < argc; i++) {
        kprintf("%s ", argv[i]);
    }
    kprintf("\n"); 
    return true;
}

int8_t cmd_cursor(int argc, char **argv) {
    // Early return: missing parameters
    if (argc < 2) return 2;

    // Check for blink enable/disable
    if (strcmp(argv[1], "blink") == 0 && argc > 2) {
        if (strcmp(argv[2], "enable") == 0) {
            cursor_blink = true;
            return true;
        }
        if (strcmp(argv[2], "disable") == 0) {
            cursor_blink = false;
            return true;
        }
    }

    // Handle cursor shape (0-2)
    uint8_t cs = atoi(argv[1]);
    if (cs < 3) {
        cursor_shape = cs;
        return true;
    }
    
    return 2;
}

int8_t cmd_scroll(int argc, char **argv) {
    // Early return: missing parameters
    if (argc < 2) return 2;
    
    view_scroll = (uint8_t)atoi(argv[1]);
    return true;
}

// ==========================================
// Input & Hardware Commands
// ==========================================

int8_t cmd_beep(int argc, char **argv) {
    play_sound(750);
    sleep(1000);
    nosound();
    return true;
}

int8_t cmd_mouse(int argc, char **argv) {
    // Early return: missing parameters
    if (argc < 2) return 2;
    
    if (strcmp(argv[1], "disable") == 0) {
        mouse_active = false;
        return true;
    } 
    
    if (strcmp(argv[1], "enable") == 0) {
        mouse_active = true;
        first_mouse_draw = true; 
        return true;
    } 
    
    return 2;
}

int8_t cmd_keyboard(int argc, char **argv) {
    // Early return: missing parameters
    if (argc < 2) return 2;
    
    switch(hash(argv[1])) {
        case hash("it"): current_layout = &kbd_IT; return true;
        case hash("us"): current_layout = &kbd_US; return true;
        default: return 2;
    }
}

// ==========================================
// Power & Debug Commands
// ==========================================

int8_t cmd_reboot(int argc, char **argv) {
    kprintf("Restarting...\n");
    outb(0x64, 0xFE); 
    return true;
}

int8_t cmd_shutdown(int argc, char **argv) {
    kprintf("Shutting down system...\n");
    // QEMU/Bochs/VirtualBox ACPI shutdown hacks
    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);
    outw(0x604, 0x2000);
    
    kprintf("ACPI shutdown not supported on this hardware.\nSystem halted safely.\n");
    __asm__ __volatile__("cli; hlt");  
    return true;
}

int8_t cmd_fault(int argc, char **argv) {
    kprintf("Triggering Page Fault...\n");
    int *a = (int*)0x67;
    *a = 7;
    return true;
}




// ==========================================
// Module Registration
// ==========================================

void register_system_commands() {
    register_command("clear",    cmd_clear,    "Clears the screen", "");
    register_command("echo",     cmd_echo,     "Prints text to screen", "[text]");
    register_command("cursor",   cmd_cursor,   "Changes cursor shape or blink state", "[0-2] / blink [enable/disable]");
    register_command("scroll",   cmd_scroll,   "Changes amount of rows for scroll", "[0-255]");
    register_command("beep",     cmd_beep,     "Plays a test sound", "");
    register_command("mouse",    cmd_mouse,    "Enables or disables the mouse", "enable / disable");
    register_command("keyboard", cmd_keyboard, "Changes keyboard layout", "it / us");
    register_command("reboot",   cmd_reboot,   "Reboots the system", "");
    register_command("shutdown", cmd_shutdown, "Halts the system and powers off", "");
    register_command("fault",    cmd_fault,    "Triggers a manual Page Fault for debugging", "");
}