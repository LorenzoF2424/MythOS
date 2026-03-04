#include "term/displayVESA.h"

BOOTBOOT bootboot;
uint32_t fb;


uint32_t pitch_pixels;
uint32_t screen_width;
uint32_t screen_height;
uint32_t* framebuffer;

uint16_t MAX_COLUMNS;
uint16_t MAX_ROWS;



void init_display() {
    pitch_pixels  = bootboot.fb_scanline / 4;
    framebuffer   = &fb;
    screen_width  = bootboot.fb_width;
    screen_height = bootboot.fb_height;
    MAX_COLUMNS   = screen_width / 8;
    MAX_ROWS      = screen_height / 16;
}

uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
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

const uint32_t vga_palette[16] = {
    0x000000, // 0: Black
    0x0000AA, // 1: Blue
    0x00AA00, // 2: Green
    0x00AAAA, // 3: Cyan
    0xAA0000, // 4: Red
    0xAA00AA, // 5: Magenta
    0xAA5500, // 6: Brown
    0xAAAAAA, // 7: Light Gray
    0x555555, // 8: Dark Gray
    0x5555FF, // 9: Light Blue
    0x55FF55, // A: Light Green
    0x55FFFF, // B: Light Cyan
    0xFF5555, // C: Light Red
    0xFF55FF, // D: Light Magenta
    0xFFFF55, // E: Yellow
    0xFFFFFF  // F: White
};


uint32_t vga_to_rgb(uint8_t vga_color) {
  
    uint8_t r_3bit = (vga_color >> 5) & 0x07; 
    uint8_t g_3bit = (vga_color >> 2) & 0x07; 
    uint8_t b_2bit = vga_color & 0x03;        

  
    uint32_t r = (r_3bit * 255) / 7;
    uint32_t g = (g_3bit * 255) / 7;
    
    uint32_t b = (b_2bit * 255) / 3;

    return (r << 16) | (g << 8) | b;
}

