#ifndef DEFINES_H
#define DEFINES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "../gnu_utils/string.h"
#include "../terminal_driver/kprintf.h"
#include "../cmos_io/io.h"


#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

#define MAX_HISTORY 10
#define MAX_COMMAND_LEN 65536
#define MAX_COMMAND_ARGS 16
#define KBD_BUFFER_SIZE 256



#endif

