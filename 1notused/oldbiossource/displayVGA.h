#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "string.h"
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 


enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) 
{
	return fg | bg << 4;
}

typedef struct {
    char c;
    uint8_t color;
}VGAslot;

VGAslot terminal_history[100][80];


uint32_t cursorAt(uint32_t x,uint32_t y) { // converts x to column and y to row based on numbers from 0x0 to 80x25

    uint8_t r=0;
    if (x>=VGA_WIDTH) {
        r=x%80;
        y+=x/80;
        x=r;
    }
    if (y>=VGA_HEIGHT) y=y%25;

    return VGA_MEMORY+(((y*VGA_WIDTH)+x)*2);
}

void putcVGAat(const char c,uintptr_t location,uint8_t color) {   
    *(volatile char*)(location)=c; 
    *(volatile char*)(location+1)=color; 
}

void cls() {

    for (uint8_t i=0;i<VGA_HEIGHT;i++) 
        for (uint8_t j=0;j<VGA_WIDTH;j++) 
            putcVGAat(0,cursorAt(j,i),15);

}



void printfVGA32at(const char *str, uintptr_t location, uint8_t color) {

    for (size_t i=0;i<strlen(str);i++){
        putcVGAat(str[i],location+(i*2),color);
    
   }



}