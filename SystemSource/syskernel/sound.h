#include <stddef.h>
#include <stdint.h>
#include "io.h"

void play_sound(uint32_t nFrequence) {
    if (nFrequence == 0) return;
    uint32_t Div = 1193180 / nFrequence;
    
    // Configura il PIT
    outb(0x43, 0xB6); // Canale 2, Accesso lobyte/hibyte, Modo 3 (Onda quadra)
    outb(0x42, (uint8_t) (Div & 0xFF));
    outb(0x42, (uint8_t) ((Div >> 8) & 0xFF));

    // Attiva lo speaker
    uint8_t state = inb(0x61);
    outb(0x61, state | 3); // Forza bit 0 e 1 a 1
}

void nosound() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}