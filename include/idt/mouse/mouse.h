#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>
#include "cmos/io.h" 
#include "idt/idt.h" 
#include "gnu_utils/point.h" 

// ==========================================
// Global Mouse State Variables
// ==========================================

// Current X and Y physical coordinates of the mouse on the screen
extern point mouse;

// Tracks the Z-axis (scroll wheel)
extern int32_t mouse_scroll;

// Button states (true when pressed)
extern bool mouse_left_click;
extern bool mouse_right_click;
extern bool mouse_middle_click;

// Flag to track if the mouse driver is currently enabled and active
extern bool mouse_active;

// ==========================================
// Mouse Driver Functions
// ==========================================

// Assembly ISR wrapper for the mouse interrupt (IRQ12)
extern "C" void mouse_isr();

// Initializes the PS/2 mouse controller, enables interrupts, and sets up IntelliMouse mode
void init_mouse();
void process_mouse_scroll();

#endif 