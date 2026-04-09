#include "cli/commands.h"
#include "idt/keyboard/keyboard.h" 
#include "idt/sound/sound.h"

// The dynamic registry (hidden from other files)
command_t registered_commands[MAX_REGISTERED_COMMANDS];
size_t command_count = 0;

// Forward declaration for the internal help command
int8_t cmd_help_internal(int argc, char** argv);

// ==========================================
// CLI Engine Core
// ==========================================
void cmd_register_all() {

    register_system_commands();
    register_color_commands();
    register_time_commands();
    register_check_commands();
    register_vfs_commands();


}


void init_cmd_manager() {
    command_count = 0;
    
    // The manager only registers its own internal help command.
    // ALL other commands are registered by the external modules (cmd_vfs, cmd_system, etc.)
    register_command("help", cmd_help_internal, "Provides help information", "[command]");
    cmd_register_all();
}

bool register_command(const char* name, int8_t (*func)(int argc, char **argv), const char* desc, const char* usage) {
    // Early return: registry is full
    if (command_count >= MAX_REGISTERED_COMMANDS) return false; 
    
    // Early return: invalid parameters
    if (name == nullptr || func == nullptr) return false;       

    registered_commands[command_count].name = name;
    registered_commands[command_count].func = func;
    registered_commands[command_count].desc = desc;
    registered_commands[command_count].usage = usage;
    
    command_count++;
    return true;
}

int8_t execute_command(int argc, char** argv) {
    // Early return: empty command
    if (argc == 0) return true;

    // Search for the command in the registry
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(argv[0], registered_commands[i].name) == 0) {
            
            // Early return: command registered but not implemented
            if (registered_commands[i].func == nullptr) return true; 
            
            return registered_commands[i].func(argc, argv);
        }
    }
    
    // Command not found
    return 0; 
}

// ==========================================
// Internal Help System
// ==========================================

int8_t cmd_help_internal(int argc, char** argv) {
    // Print all commands if no specific argument is provided
    if (argc < 2) {
        kprintf("Command List:\n");
        for (size_t i = 0; i < command_count; i++) {
            kprintf(" %s\t%s\n", registered_commands[i].name, registered_commands[i].usage);
        }
        return true;
    }

    // Print specific command details
    for (size_t i = 0; i < command_count; i++) {
        if (strcmp(argv[1], registered_commands[i].name) == 0) {
            kprintf("%s - %s\nUsage: %s %s\n", 
                    registered_commands[i].name, 
                    registered_commands[i].desc, 
                    registered_commands[i].name, 
                    registered_commands[i].usage);
            return true;
        }
    }
    
    // Command details not found
    return 2; 
}

