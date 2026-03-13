#include "idt/mouse/mouse.h"


point mouse;
bool mouse_left_click = false;
bool mouse_right_click = false;
bool mouse_middle_click = false;


uint8_t mouse_cycle = 0;
int8_t mouse_byte[3];
bool mouse_active = true; 


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
    //  gate 44 (IRQ12)
    uint16_t cs;
    asm volatile ("mov %%cs, %0" : "=r"(cs));
    idt_set_gate(44, (uint64_t)mouse_isr, cs, 0x8E);

    
    mouse_wait(1);
    outb(0x64, 0xA8);

    
    mouse_wait(1);
    outb(0x64, 0x20); 
    mouse_wait(0);
    uint8_t status = inb(0x60);
    status |= 2; 
    
    
    mouse_wait(1);
    outb(0x64, 0x60); 
    mouse_wait(1);
    outb(0x60, status);

    
    mouse_write(0xF6); 
    mouse_read();      

    mouse_write(0xF4); 
    mouse_read();      
}

extern "C" void mouse_handler_c() {
    uint8_t status = inb(0x64);
    
    if (status & 0x01) {
        
       
        uint8_t data = inb(0x60);
     
        if (status & 0x20) {
            
            mouse_byte[mouse_cycle] = data;
            mouse_cycle++;

            if (mouse_cycle == 3) {
                mouse_cycle = 0;

                if (mouse_byte[0] & 0x08) {
                    mouse.x += (int8_t)mouse_byte[1];
                    mouse.y -= (int8_t)mouse_byte[2];
                    

                    if (mouse.x < 0) mouse.x = 0;
                    if (mouse.y < 0) mouse.y = 0;
                    
                    if (mouse.x > 1024 - 1) mouse.x = 1024 - 1; 
                    if (mouse.y > 768 - 1)  mouse.y = 768 - 1;

                    mouse_left_click = mouse_byte[0] & 0x01;
                    mouse_right_click = mouse_byte[0] & 0x02;
                    mouse_middle_click = mouse_byte[0] & 0x04;
                }
            }
        } 
        

    }

    
    outb(0xA0, 0x20); 
    outb(0x20, 0x20); 
}