#include "idt/keyboard/keyboardLayout.h"

KeyboardLayout* current_layout = &kbd_IT;


KeyboardLayout kbd_IT = {
    /* normal */
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\'', 141, '\b',  /* 141=ì */
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 138, '+', '\n',       /* 138=è */
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 149, 133,  92,             /* 149=ò, 133=à, 92=\ */
        0,  151, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-',   0,             /* 151=ù */
      '*',    0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
        0,    0, '<',   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
    },
    /* shift */
    {
        0,  27, '!', '"', 156, '$', '%', '&', '/', '(', ')', '=', '?', '^', '\b',   /* 156=£ */
      '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 130, '*', '\n',      /* 130=é */
        0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 135, 248, '|',            /* 135=ç, 248=° */
        0,   21, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_',   0,            /* 21=§ */
      '*',    0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
        0,    0, '>',   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
    },
    /* altgr */
    {
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '~',   0,    /* AltGr+ì=~ */
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '[', ']',   0,         /* AltGr+è=[ AltGr++=] */
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '@', '#','\\',              /* AltGr+ò=@ AltGr+à=# AltGr+ù=\ */
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
    },
    /* shift_altgr */
    {
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,    
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '{', '}',   0,    /* <-- Shift+AltGr+è = { e Shift+AltGr++ = } */     
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,              
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        
    }
};

KeyboardLayout kbd_US = {
    /* normal */
    {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
      '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
      '*',    0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
        0,    0, '<',   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
    },
    /* shift */
    {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
      '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
        0,  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,
      '*',    0, ' ',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0, '7', '8', '9', '-', '4', '5', '6', '+', '1', '2', '3', '0', '.',
        0,    0, '>',   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
    },
    /* altgr - US doesn't have AltGr, everything is 0 */
    { 0 },
    /* shift_altgr */
    { 0 }
};

const key_code_t ps2_set1_to_keycode[128] = {
    // 0x00 - 0x0E: Top row and Numbers
    KEY_UNKNOWN, KEY_ESC, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_BACKSPACE,
    
    // 0x0F - 0x1C: First letter row
    KEY_TAB, KEY_Q, KEY_W, KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFT_BRACKET, KEY_RIGHT_BRACKET, KEY_ENTER,
    
    // 0x1D - 0x29: Second letter row
    KEY_LEFT_CTRL, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J, KEY_K, KEY_L, KEY_SEMICOLON, KEY_QUOTE, KEY_BACKTICK,
    
    // 0x2A - 0x36: Third letter row
    KEY_LEFT_SHIFT, KEY_BACKSLASH, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N, KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_RIGHT_SHIFT,
    
    // 0x37 - 0x3A: Bottom row modifiers and space
    KEY_KEYPAD_STAR, KEY_LEFT_ALT, KEY_SPACE, KEY_CAPS_LOCK,
    
    // 0x3B - 0x44: Function keys F1-F10
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,
    
    // 0x45 - 0x46: Locks
    KEY_NUM_LOCK, KEY_SCROLL_LOCK,
    
    // 0x47 - 0x4A: Numeric Keypad (Top row)
    KEY_KEYPAD_7, KEY_KEYPAD_8, KEY_KEYPAD_9, KEY_KEYPAD_MINUS,
    
    // 0x4B - 0x4E: Numeric Keypad (Middle row)
    KEY_KEYPAD_4, KEY_KEYPAD_5, KEY_KEYPAD_6, KEY_KEYPAD_PLUS,
    
    // 0x4F - 0x53: Numeric Keypad (Bottom row and Dot)
    KEY_KEYPAD_1, KEY_KEYPAD_2, KEY_KEYPAD_3, KEY_KEYPAD_0, KEY_KEYPAD_DOT,
    
    // 0x54 - 0x56: Unmapped / SysReq gaps
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN,
    
    // 0x57 - 0x58: Additional Function keys
    KEY_F11, KEY_F12
    
    // Elements from 0x59 to 0x7F are implicitly set to 0 (KEY_UNKNOWN)
};

const key_code_t ps2_set2_to_keycode[256] = {
    // 0x00 - 0x0F
    KEY_UNKNOWN, KEY_F9, KEY_UNKNOWN, KEY_F5, KEY_F3, KEY_F1, KEY_F2, KEY_F12,
    KEY_UNKNOWN, KEY_F10, KEY_F8, KEY_F6, KEY_F4, KEY_TAB, KEY_BACKTICK, KEY_UNKNOWN,
    
    // 0x10 - 0x1F
    KEY_UNKNOWN, KEY_LEFT_ALT, KEY_LEFT_SHIFT, KEY_UNKNOWN, KEY_LEFT_CTRL, KEY_Q, KEY_1, KEY_UNKNOWN,
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_Z, KEY_S, KEY_A, KEY_W, KEY_2, KEY_UNKNOWN,
    
    // 0x20 - 0x2F
    KEY_UNKNOWN, KEY_C, KEY_X, KEY_D, KEY_E, KEY_4, KEY_3, KEY_UNKNOWN,
    KEY_UNKNOWN, KEY_SPACE, KEY_V, KEY_F, KEY_T, KEY_R, KEY_5, KEY_UNKNOWN,
    
    // 0x30 - 0x3F
    KEY_UNKNOWN, KEY_N, KEY_B, KEY_H, KEY_G, KEY_Y, KEY_6, KEY_UNKNOWN,
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_M, KEY_J, KEY_U, KEY_7, KEY_8, KEY_UNKNOWN,
    
    // 0x40 - 0x4F
    KEY_UNKNOWN, KEY_COMMA, KEY_K, KEY_I, KEY_O, KEY_0, KEY_9, KEY_UNKNOWN,
    KEY_UNKNOWN, KEY_DOT, KEY_SLASH, KEY_L, KEY_SEMICOLON, KEY_P, KEY_MINUS, KEY_UNKNOWN,
    
    // 0x50 - 0x5F
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_QUOTE, KEY_UNKNOWN, KEY_LEFT_BRACKET, KEY_EQUAL, KEY_UNKNOWN, KEY_UNKNOWN,
    KEY_CAPS_LOCK, KEY_RIGHT_SHIFT, KEY_ENTER, KEY_RIGHT_BRACKET, KEY_UNKNOWN, KEY_BACKSLASH, KEY_UNKNOWN, KEY_UNKNOWN,
    
    // 0x60 - 0x6F
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, KEY_BACKSPACE, KEY_UNKNOWN,
    KEY_UNKNOWN, KEY_KEYPAD_1, KEY_UNKNOWN, KEY_KEYPAD_4, KEY_KEYPAD_7, KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN,
    
    // 0x70 - 0x7F
    KEY_KEYPAD_0, KEY_KEYPAD_DOT, KEY_KEYPAD_2, KEY_KEYPAD_5, KEY_KEYPAD_6, KEY_KEYPAD_8, KEY_ESC, KEY_NUM_LOCK,
    KEY_F11, KEY_KEYPAD_PLUS, KEY_KEYPAD_3, KEY_KEYPAD_MINUS, KEY_KEYPAD_STAR, KEY_KEYPAD_9, KEY_SCROLL_LOCK, KEY_UNKNOWN,
    
    // 0x80 - 0x8F
    KEY_UNKNOWN, KEY_UNKNOWN, KEY_UNKNOWN, KEY_F7
    
    // Remaining elements up to 255 are automatically set to 0 (KEY_UNKNOWN)
};

key_code_t translate_set1_extended(uint8_t scancode) {
    switch (scancode) {
        case 0x48: return KEY_UP;
        case 0x50: return KEY_DOWN;
        case 0x4B: return KEY_LEFT;
        case 0x4D: return KEY_RIGHT;
        case 0x47: return KEY_HOME;
        case 0x4F: return KEY_END;
        case 0x49: return KEY_PAGE_UP;
        case 0x51: return KEY_PAGE_DOWN;
        case 0x52: return KEY_INSERT;
        case 0x53: return KEY_DELETE;
        case 0x38: return KEY_RIGHT_ALT;   // AltGr
        case 0x1D: return KEY_RIGHT_CTRL;  // Right Ctrl
        case 0x5B: return KEY_LEFT_GUI;    // Windows/Super Key (Left)
        case 0x5C: return KEY_RIGHT_GUI;   // Windows/Super Key (Right)
        case 0x35: return KEY_KEYPAD_DIVIDE;
        case 0x1C: return KEY_KEYPAD_ENTER;
        default: return KEY_UNKNOWN;
    }
}

key_code_t translate_set2_extended(uint8_t scancode) {
    switch(scancode) {
        case 0x75: return KEY_UP;
        case 0x72: return KEY_DOWN;
        case 0x6B: return KEY_LEFT;
        case 0x74: return KEY_RIGHT;
        case 0x6C: return KEY_HOME;
        case 0x69: return KEY_END;
        case 0x7D: return KEY_PAGE_UP;
        case 0x7A: return KEY_PAGE_DOWN;
        case 0x70: return KEY_INSERT;
        case 0x71: return KEY_DELETE;
        case 0x11: return KEY_RIGHT_ALT;    // AltGr
        case 0x14: return KEY_RIGHT_CTRL;   // Right Ctrl
        case 0x1F: return KEY_LEFT_GUI;     // Left Windows Key
        case 0x27: return KEY_RIGHT_GUI;    // Right Windows Key
        case 0x4A: return KEY_KEYPAD_DIVIDE;
        case 0x5A: return KEY_KEYPAD_ENTER;
        default: return KEY_UNKNOWN;
    }
}