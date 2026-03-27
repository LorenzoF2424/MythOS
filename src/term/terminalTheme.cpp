#include "term/terminalTheme.h"

// Global theme table definition initialized to zero/false
theme_rule_t syntax_theme[256] = {0};

// ==========================================
// Theme Engine Management
// ==========================================

void clear_theme() {
    for (int i = 0; i < 256; i++) {
        syntax_theme[i].active = false;
    }
}

void set_theme_rule(const char* chars, uint32_t color) {
    // Early Return: Safety check against null pointers
    if (!chars) return;

    // Apply the custom color to every character in the provided string
    for (int i = 0; chars[i] != '\0'; i++) {
        unsigned char c = (unsigned char)chars[i];
        syntax_theme[c].active = true;
        syntax_theme[c].fg_color = color;
    }
}

void init_default_theme() {
    clear_theme();
    
    //  Soft Cyan
    set_theme_rule("0123456789", 0x8BE9FD);
    
    //  Soft Pink/Magenta
    set_theme_rule("\"'", 0xc26a36);
    
    
    //  Pastel Green
    set_theme_rule("+-*/:=<>", 0x50FA7B);
    
    //  Subdued Yellow
    set_theme_rule("()[]{};,", 0xF1FA8C);
}

terminal_color_t get_theme_color(char c, terminal_color_t default_color) {
    unsigned char uc = (unsigned char)c;
    
    // Early Return: If no custom rule exists for this character, use the standard color
    if (!syntax_theme[uc].active) return default_color;
    
    // Otherwise, apply the custom foreground color while keeping the standard background
    return {syntax_theme[uc].fg_color, default_color.bg};
}

bool load_theme(const char* theme_name) {
    // Early Return 1: Prevent null pointer dereference
    if (!theme_name) return false;

    // Command to disable the theme engine completely
    if (strcmp(theme_name, "disable") == 0 || strcmp(theme_name, "off") == 0) {
        // Assuming 'terminal' is accessible here. If not, you might need to 
        // return an enum or handle the toggle in the CLI directly.
        terminal.theme_enabled = false; 
        return true;
    }

    // Dracula Theme (Soft pastels)
    if (strcmp(theme_name, "dracula") == 0 || strcmp(theme_name, "default") == 0) {
        init_default_theme();
        terminal.theme_enabled = true;
        return true;
    }


    // Matrix/Hacker Theme (Various shades of bright and dark green)
    if (strcmp(theme_name, "matrix") == 0 || strcmp(theme_name, "hacker") == 0) {
        clear_theme();
        set_theme_rule("0123456789", 0x00FF00); // Bright green
        set_theme_rule("\"'", 0x00AA00);        // Mid green
        set_theme_rule("+-*/=<>", 0x55FF55);    // Light green
        set_theme_rule("()[]{};,", 0x008800);   // Dark green
        terminal.theme_enabled = true;
        return true;
    }

    // Ocean Theme (Deep blues and cyan)
    if (strcmp(theme_name, "ocean") == 0) {
        clear_theme();
        set_theme_rule("0123456789", 0x00FFFF); // Cyan
        set_theme_rule("\"'", 0x0088FF);        // Light Blue
        set_theme_rule("+-*/=<>", 0x00BFFF);    // Deep Sky Blue
        set_theme_rule("()[]{};,", 0x4682B4);   // Steel Blue
        terminal.theme_enabled = true;
        return true;
    }

    // Early Return 2: Theme name not recognized
    return false;
}