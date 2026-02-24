#ifndef DISPLAY_VESA_H
#define DISPLAY_VESA_H

#include <stddef.h>
#include <stdint.h>




struct VBEInfoBlock {
    uint16_t attributes;
    uint8_t  winA, winB;
    uint16_t granularity;
    uint16_t winsize;
    uint16_t segmentA, segmentB;
    uint32_t realFctPtr;
    uint16_t pitch;          // offset 16
    uint16_t width;          // offset 18
    uint16_t height;         // offset 20
    uint8_t  wchar, ychar;   // offset 22, 23
    uint8_t  planes;         // offset 24
    uint8_t  bpp;            // offset 25
    uint8_t  banks;          // offset 26
    uint8_t  memory_model;   // offset 27
    uint8_t  bank_size;      // offset 28
    uint8_t  image_pages;    // offset 29
    uint8_t  reserved0;      // offset 30
    uint8_t  red_mask, red_pos;
    uint8_t  green_mask, green_pos;
    uint8_t  blue_mask, blue_pos;
    uint8_t  rsv_mask, rsv_pos;
    uint8_t  direct_color;
    uint32_t framebuffer;    // offset 40 
} __attribute__((packed));

VBEInfoBlock* vbe_info = (VBEInfoBlock*)0x7000;
uint8_t* lfb;
uint16_t pitch;
uint16_t screen_width;
uint16_t screen_height;
uint16_t MAX_COLUMNS;
uint16_t MAX_ROWS;
void init_display() {
    pitch           = vbe_info->pitch;
    screen_width    = vbe_info->width;
    screen_height   = vbe_info->height;
    lfb=(uint8_t*)(uintptr_t)vbe_info->framebuffer;
    MAX_COLUMNS=screen_width/8;
    MAX_ROWS=screen_height/16;
}

uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

void put_pixel(uint16_t x, uint16_t y, uint32_t color) {
    uint32_t offset = y * pitch + x * 3;
    lfb[offset + 0] =  color        & 0xFF; // B
    lfb[offset + 1] = (color >> 8)  & 0xFF; // G
    lfb[offset + 2] = (color >> 16) & 0xFF; // R
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