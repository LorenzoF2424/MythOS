#ifndef DISPLAY_VESA_H
#define DISPLAY_VESA_H

#include <stdint.h>
#include "bootboot.h"

extern BOOTBOOT bootboot;
extern uint32_t fb;

// Usiamo per evitare errori del linker se includi questo .h in più file
uint32_t pitch_pixels;
uint32_t screen_width;
uint32_t screen_height;
uint32_t* framebuffer;

uint16_t MAX_COLUMNS;
uint16_t MAX_ROWS;

void init_display() {
    // Dividiamo per 4 per ottenere lo scanline in PIXEL (dato che i pixel sono a 32-bit)
    pitch_pixels  = bootboot.fb_scanline / 4;
    framebuffer   = &fb;
    screen_width  = bootboot.fb_width;
    screen_height = bootboot.fb_height;
    MAX_COLUMNS   = screen_width / 8;
    MAX_ROWS      = screen_height / 16;
}

uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
    // Formato standard per BOOTBOOT (generalmente ARGB o BGRA)
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void put_pixel(uint16_t x, uint16_t y, uint32_t color) {
    
    if (x >= screen_width || y >= screen_height) return;

    framebuffer[y * pitch_pixels + x] = color;
}

void draw_rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
    uint16_t endX = x + width;
    uint16_t endY = y + height;
    for (uint16_t _y = y; _y < endY; _y++)
        for (uint16_t _x = x; _x < endX; _x++)
            put_pixel(_x, _y, color);
}

void draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    int dx =  (x2 - x1);
    int dy = -(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;

    while (1) {
        put_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

#endif