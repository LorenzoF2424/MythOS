#include "term/displayVESA.h"

BOOTBOOT bootboot;
uint32_t fb;


uint32_t pitch_pixels;
uint32_t screen_width;
uint32_t screen_height;
uint32_t* framebuffer;

uint16_t MAX_COLUMNS;
uint16_t MAX_ROWS;

vga_pallette_type current_palette=DEFAULT;

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

terminal_color_t terminal_color(uint32_t bg, uint32_t fg) {
    terminal_color_t c;
    c.fg = fg;
    c.bg = bg;
    return c;
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



uint32_t vga_to_rgb(uint8_t vga_color) {
  
    uint8_t r_3bit = (vga_color >> 5) & 0x07; 
    uint8_t g_3bit = (vga_color >> 2) & 0x07; 
    uint8_t b_2bit = vga_color & 0x03;        

  
    uint32_t r = (r_3bit * 255) / 7;
    uint32_t g = (g_3bit * 255) / 7;
    
    uint32_t b = (b_2bit * 255) / 3;

    return (r << 16) | (g << 8) | b;
}


const uint32_t vga_palette[VGA_PALLETTE_NUMBER][16] = {
{
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
},{
    0x1E1E1E, // 0: Black (soft dark)
    0x264F78, // 1: Blue
    0x4EC94E, // 2: Green
    0x4EC9B0, // 3: Cyan
    0xF44747, // 4: Red
    0xC586C0, // 5: Magenta
    0xCE9178, // 6: Brown/Orange
    0xD4D4D4, // 7: Light Gray
    0x808080, // 8: Dark Gray
    0x569CD6, // 9: Light Blue
    0xB5CEA8, // A: Light Green
    0x9CDCFE, // B: Light Cyan
    0xF44747, // C: Light Red
    0xC586C0, // D: Light Magenta
    0xDCDC87, // E: Yellow
    0xE8E8E8  // F: White (soft)
},{
    0x073642, // 0: Black
    0x268BD2, // 1: Blue
    0x859900, // 2: Green
    0x2AA198, // 3: Cyan
    0xDC322F, // 4: Red
    0xD33682, // 5: Magenta
    0xB58900, // 6: Brown
    0xEEE8D5, // 7: Light Gray
    0x002B36, // 8: Dark Gray
    0x839496, // 9: Light Blue
    0x586E75, // A: Light Green
    0x657B83, // B: Light Cyan
    0xCB4B16, // C: Light Red
    0x6C71C4, // D: Light Magenta
    0x93A1A1, // E: Yellow
    0xFDF6E3  // F: White
},{
    0x000000, // 0: Black         (Very dark blue-grey background, not pure black)
    0x4A69BD, // 1: Blue          (Desaturated ocean blue)
    0x6A994E, // 2: Green         (Dark sage green)
    0x38ADA9, // 3: Cyan          (Teal/Water green)
    0xD63031, // 4: Red           (Soft brick red, great for errors/panics)
    0x8E44AD, // 5: Magenta       (Soft dark purple)
    0xCA6702, // 6: Brown         (Burnt orange)
    0xDFE6E9, // 7: Light Grey    (Off-white, ideal for DEFAULT TEXT)

    0x636E72, // 8: Dark Grey     (Perfect for disabled or subtle text)
    0x82CCDD, // 9: Light Blue    (Pastel sky blue)
    0xA7C957, // 10: Light Green  (Soft apple green, great for "OK" / "SUCCESS")
    0x94D2BD, // 11: Light Cyan   (Light aqua blue)
    0xFF7675, // 12: Light Red    (Light coral red)
    0xD6A2E8, // 13: Light Magenta(Pastel lilac)
    0xFDCB6E, // 14: Yellow       (Mustard/sand yellow, not blinding)
    0xDDDDDD  // 15: White        (Cream/ice white)
}
};

