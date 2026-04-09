#ifndef COMMAND_H
#define COMMAND_H

#include "idt/keyboard/keyboard.h"
#include "cli/commands.h"
#include "fs/vfs.h"

void prepare_for_next_command();
void sysCommand(const char *command);
const char* capture_command(const char* cmd);
void sysCommandAt(terminal_output_t *t, const char *command, point p);
void command_handler();
void refresh_command_line();


#endif