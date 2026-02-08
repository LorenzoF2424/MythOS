#include "displayVGA.h"

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;

uint16_t* gerCmdCursorPos() { // converts x to column and y to row based on numbers from 0x0 to 80x25

    return terminal_buffer;
}

void terminalputcVGA(const char c,uintptr_t location,uint8_t color) {   
    *(volatile char*)(location)=c; 
    *(volatile char*)(location+1)=color; 
}






extern "C" void main() { // KERNELLLLLLLLLLLLLLLLLLLLLLL

    cls();

    const char *s="BENVENUTO NEL KERNEL FUNZIONA PORCO DIO SUCCHIAMELO!!!";
    const char *s2="che bello usare c++";
    /*for (int i=0;i<80*25*2;i=i+2) {
        *(char*)(0xb8000+(i))= 0;
        *(char*)(0xb8000+(i+1))= 0x0F;

    }*/
    int color=15;
    for (int i=0;i<16;i++)  {
        printfVGA32at((char*)s,cursorAt(0,i),color);
        color--;
    }

    printfVGA32at(s2,cursorAt(0,24),VGA_COLOR_BLUE);


    while (1) {
    }
}