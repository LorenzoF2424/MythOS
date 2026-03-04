#ifndef COMMAND_H
#define COMMAND_H

#include "idt/defines.h"
#include "idt/keyboard/keyboard.h"
#include "cli/cmd/commands.h"


void prepare_for_next_command();
void sysCommand(const char *command);
void sysCommandAt(const char *command, uint16_t x, uint16_t y);
void command_handler();
void refresh_command_line();


#endif