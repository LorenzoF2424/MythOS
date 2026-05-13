#ifndef TERMINAL_THEME_H
#define TERMINAL_THEME_H

#include <stdint.h>
#include <stdbool.h>
#include "displayTerminal.h"

// Include the header that defines terminal_color_t here.
// Example: #include "term/displayTerminal.h" (if it doesn't cause a circular loop)

// ==========================================
// Theme Engine Structures
// ==========================================

// Defines a color rule for a single ASCII character
struct theme_rule_t {
    bool active;        // True if this character has a custom color
    uint32_t fg_color;  // The custom foreground color (hex)
};

// Global array holding the rule for every possible ASCII character (0-255)
extern theme_rule_t syntax_theme[256];

// ==========================================
// Theme Engine Management Functions
// ==========================================

// Resets all characters to use the default terminal color
void clear_theme();

// Assigns a specific hex color to a string of characters
void set_theme_rule(const char* chars, uint32_t color);

// Loads the default syntax highlighting preset
void init_default_theme();

// Evaluates a character and returns its specific theme color (O(1) lookup)
terminal_color_t get_theme_color(char c, terminal_color_t default_color);

bool load_theme(const char* theme_name);

#endif 