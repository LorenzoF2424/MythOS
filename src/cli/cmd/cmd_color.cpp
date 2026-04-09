#include "cli/commands.h"


int8_t cmd_color(int argc, char **argv) {
    // Early return: not enough arguments provided
    if (argc < 2) return 2;

    switch (hash(argv[1])) {
        
        // ==========================================
        // Palette Type Command
        // ==========================================
        case hash("type"):
            // Early return: missing palette type value
            if (argc < 3) return 2;
            
            switch(htoi(argv[2])) {
                case 0: current_palette = DEFAULT; break;
                case 1: current_palette = SOFT; break;
                case 2: current_palette = SOLARIZED; break;
                case 3: current_palette = CUSTOM; break;
                default: return 2; // Early return: invalid palette type
            }
            return true;

        // ==========================================
        // RGB Command
        // ==========================================
        case hash("rgb"): {
            // Early return: missing background or foreground values
            if (argc < 4) return 2;
            
            uint32_t bg = htoi(argv[2]);
            uint32_t fg = htoi(argv[3]);
            
            // Early return: background and foreground cannot be the same
            if (bg == fg) return 2;
            
            // Check if targeting the info bar
            if (argc > 4 && hash(argv[4]) == hash("info")) {
                info_bar_window.change_color(terminal_color(fg, bg));
                return true;
            }
            
            // Apply to main terminal
            terminal.change_color(terminal_color(fg, bg));
            return true;
        }

        // ==========================================
        // Theme Command
        // ==========================================
        case hash("theme"):
            // Early return: missing theme name
            if (argc < 3) return 2;
            
            // Early return: failed to load the requested theme
            if (!load_theme(argv[2])) return 2;
            
            // Theme loaded successfully, apply the new base color
            terminal.change_color(terminal.color); 
            return true;

        // ==========================================
        // Standard VGA Attribute Command
        // ==========================================
        default: {
            int32_t raw_attr = htoi(argv[1]);
            
            // Early return: invalid attribute range
            if (raw_attr < 0 || raw_attr > 255) return 2;
            
            uint8_t attr = (uint8_t)raw_attr;
            
            // Check if targeting the info bar
            if (argc > 2 && hash(argv[2]) == hash("info")) {
                info_bar_window.color = terminal_color(vga_palette[current_palette][(attr >> 4) & 0x0F], vga_palette[current_palette][attr & 0x0F]);
                return true;
            }
            
            // Apply to main terminal
            terminal.change_color(terminal_color(vga_palette[current_palette][(attr >> 4) & 0x0F], vga_palette[current_palette][attr & 0x0F]));
            return true;
        }
    }
}

// ==========================================
// Module Registration
// ==========================================

void register_color_commands() {
    register_command("color", cmd_color, "Changes terminal colors or theme", "[attr] [info] / rgb [bg] [fg] [info] / theme [name] / type [0-3]");
}