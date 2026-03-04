#ifndef COMMANDS_H
#define COMMANDS_H


#include "../../idt/defines.h"

typedef struct {
    const char* name;
    int8_t (*func)(const char args[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]);
    const char* help;
} command_t;

extern int8_t execute_command(const char c[MAX_COMMAND_ARGS][MAX_COMMAND_LEN]);



#endif