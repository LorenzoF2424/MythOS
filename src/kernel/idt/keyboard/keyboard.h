#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "idt/IDT.h"
#include "idt/defines.h" 
#include "idt/keyboard/command.h" 
#include "idt/timer/timer.h" 
#include "cmos_io/io.h"
#include "cli/cmd/commands.h"
extern bool shift_pressed;
extern bool altgr_pressed;
extern bool extended_scancode;
extern char history[MAX_HISTORY][MAX_COMMAND_LEN]; // Usando MAX_INPUT_LEN
extern int8_t history_count;
extern int8_t history_index;

void init_keyboard();
extern "C" void keyboard_isr();
extern "C" void keyboard_handler_c();

void process_keyboard_events();
void handle_character_input(char c);
void refresh_command_line();

#endif