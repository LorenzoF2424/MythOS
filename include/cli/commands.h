#ifndef COMMANDS_H
#define COMMANDS_H

#include "idt/idt.h" 


#define MAX_REGISTERED_COMMANDS 64

// Represents a dynamically registered command
typedef struct {
    const char* name;
    int8_t (*func)(int argc, char **argv);
    const char* desc;
    const char* usage;
} command_t;

// ==========================================
// CLI Engine API
// ==========================================

// Initializes the command manager (must be called in main)
void init_cmd_manager();

// Allows various modules to register their commands
bool register_command(const char* name, int8_t (*func)(int argc, char **argv), const char* desc, const char* usage);

// Executes a command by searching the dynamic registry
int8_t execute_command(int argc, char **argv);

void register_system_commands();
void register_color_commands();
void register_time_commands();
void register_check_commands();
void register_vfs_commands();


#endif 