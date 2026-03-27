#include "idt/mouse/mouse.h"

point mouse;
int32_t mouse_scroll = 0; // Tracks the Z-axis (scroll wheel)

bool mouse_left_click = false;
bool mouse_right_click = false;
bool mouse_middle_click = false;

uint8_t mouse_cycle = 0;
int8_t mouse_byte[4]; // Expanded to 4 bytes to hold the scroll data
uint8_t mouse_type = 0; // 0 for Standard PS/2, 3 for IntelliMouse
bool mouse_active = false; 

void mouse_wait(uint8_t a_type) {
    uint32_t timeout = 100000;
    if (a_type == 0) {
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

void mouse_write(uint8_t command) {
    mouse_wait(1);
    outb(0x64, 0xD4); 
    mouse_wait(1);
    outb(0x60, command);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void init_mouse() {
    // 1. Setup IDT gate for IRQ12 (Interrupt 44)
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    idt_set_gate(44, (uint64_t)mouse_isr, cs, 0x8E);
    
    // 2. Enable auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);
    
    // 3. Enable interrupts in Compaq status byte
    mouse_wait(1);
    outb(0x64, 0x20); 
    mouse_wait(0);
    uint8_t status = inb(0x60);
    status |= 2; 
    
    mouse_wait(1);
    outb(0x64, 0x60); 
    mouse_wait(1);
    outb(0x60, status);
    
    // 4. Set default settings
    mouse_write(0xF6); 
    mouse_read(); // Acknowledge
    
    // 5. THE MAGIC SEQUENCE (Enable IntelliMouse 4-byte packet)
    mouse_write(0xF3); mouse_read(); // Set sample rate
    mouse_write(200);  mouse_read(); // to 200
    
    mouse_write(0xF3); mouse_read(); // Set sample rate
    mouse_write(100);  mouse_read(); // to 100
    
    mouse_write(0xF3); mouse_read(); // Set sample rate
    mouse_write(80);   mouse_read(); // to 80
    
    // 6. Request Device ID to verify if the magic sequence worked
    mouse_write(0xF2); 
    mouse_read(); // Acknowledge
    mouse_type = mouse_read(); // Will be 3 for IntelliMouse, 0 for standard
    
    // 7. Enable Data Reporting
    mouse_write(0xF4); 
    mouse_read(); // Acknowledge
}

extern "C" void mouse_handler_c() {
    uint8_t status = inb(0x64);
    
    // Early Return 1: Check if data is available (bit 0)
    if (!(status & 0x01)) {
        outb(0xA0, 0x20); outb(0x20, 0x20); // Send EOI
        return;
    }
    
    uint8_t data = inb(0x60);
    
    // Early Return 2: Check if data is actually from the mouse (bit 5)
    if (!(status & 0x20)) {
        outb(0xA0, 0x20); outb(0x20, 0x20); // Send EOI
        return;
    }
    
    // Store the byte and advance the cycle
    mouse_byte[mouse_cycle++] = data;
    
    // Determine expected packet size dynamically based on hardware
    uint8_t packet_size = (mouse_type == 3) ? 4 : 3;
    
    // Early Return 3: Wait until the packet is fully assembled
    if (mouse_cycle < packet_size) {
        outb(0xA0, 0x20); outb(0x20, 0x20); // Send EOI
        return;
    }
    
    // --- Packet is complete ---
    mouse_cycle = 0;
    
    // Early Return 4: Synchronization check (Bit 3 of Byte 0 must be 1)
    if (!(mouse_byte[0] & 0x08)) {
        outb(0xA0, 0x20); outb(0x20, 0x20); // Send EOI
        return;
    }
    
    // Update coordinates (PS/2 Y-axis is inverted)
    mouse.x += (int8_t)mouse_byte[1];
    mouse.y -= (int8_t)mouse_byte[2];
    
    // Clamp coordinates to screen boundaries
    if (mouse.x < 0) mouse.x = 0;
    if (mouse.y < 0) mouse.y = 0;
    if (mouse.x > 1024 - 1) mouse.x = 1024 - 1; 
    if (mouse.y > 768 - 1)  mouse.y = 768 - 1;
    
    // Update button states
    mouse_left_click   = mouse_byte[0] & 0x01;
    mouse_right_click  = mouse_byte[0] & 0x02;
    mouse_middle_click = mouse_byte[0] & 0x04;
    
    // Extract Z-axis (scroll wheel) if IntelliMouse is active
    if (packet_size == 4) {
        mouse_scroll -= (int8_t)mouse_byte[3]; // Decrement or increment based on standard
    }
    
    // Send End of Interrupt (EOI) to Master and Slave PICs
    outb(0xA0, 0x20); 
    outb(0x20, 0x20); 
}

void process_mouse_scroll() {
    // Early return: Do nothing if the scroll wheel hasn't moved
    if (mouse_scroll == 0) return;

    // Define scroll speed (e.g., 3 lines per wheel tick)
    int32_t lines_to_scroll = mouse_scroll * 3; 

    if (lines_to_scroll > 0) {
        // Scrolling UP (looking back into history)
        if (view_offset + lines_to_scroll <= history_lines) {
            view_offset += lines_to_scroll;
        } else {
            view_offset = history_lines;
        }
    } else {
        // Scrolling DOWN (returning to the present)
        uint32_t scroll_down = -lines_to_scroll; // Make it positive for subtraction
        if (view_offset >= scroll_down) {
            view_offset -= scroll_down;
        } else {
            view_offset = 0;
        }
    }

    // Reset the scroll accumulator after processing
    mouse_scroll = 0;

    // CRITICAL: Hide the mouse cursor before redrawing to prevent "Ghost Cursors"
    erase_mouse(); 
    terminal.redraw_all();

    // Manage text cursor visibility based on camera position
    if (view_offset == 0) {
        draw_cursor = true;
        reset_cursor_blink();
    } else {
        // Keep the text cursor logically off while viewing history
        draw_cursor = false;
        if (cursor_visible) {
            remove_cursor_shape();
            cursor_visible = false;
        }
    }
}