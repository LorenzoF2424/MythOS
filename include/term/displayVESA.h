#ifndef DISPLAY_VESA_H
#define DISPLAY_VESA_H

#include <stdint.h>
#include "bootboot.h"


extern BOOTBOOT bootboot;
extern uint32_t fb;


extern uint32_t pitch_pixels;
extern uint32_t screen_width;
extern uint32_t screen_height;
extern uint32_t* framebuffer;

extern uint16_t MAX_COLUMNS;
extern uint16_t MAX_ROWS;

struct vga_color_t {
    uint32_t fg;
    uint32_t bg;
};
extern const uint32_t vga_palette[16];

void init_display();
uint32_t color(uint8_t r, uint8_t g, uint8_t b);
void put_pixel(uint16_t x, uint16_t y, uint32_t color);
void draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
uint32_t vga_to_rgb(uint8_t vga_color);

#endif