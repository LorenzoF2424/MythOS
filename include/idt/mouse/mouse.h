#ifndef MOUSE_H
#define MOUSE_H


#include <stdint.h>
#include <stdbool.h>
#include "cmos/io.h" 
#include "idt/idt.h" 
#include "gnu_utils/point.h" 


extern point mouse;


extern bool mouse_left_click;
extern bool mouse_right_click;
extern bool mouse_middle_click;
extern bool mouse_active;

extern "C" void mouse_isr();
void init_mouse();



#endif