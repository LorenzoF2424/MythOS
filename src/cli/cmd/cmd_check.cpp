#include "cli/commands.h"

// ==========================================
// Helper Functions (Private to this file)
// ==========================================

int8_t cmd_format_memory_info(uint32_t value_in_bytes, uint32_t format_hash) {
    switch(format_hash) {
        case hash("b"):
            kprintf("%d B", value_in_bytes);
            return true;
        case hash("kb"):
            kprintf("%.4f KB", (double)value_in_bytes / 1024.0);
            return true;
        case hash("gb"):
            kprintf("%.4f GB", (double)value_in_bytes / (1024.0 * 1024.0 * 1024.0));
            return true;
        default: // Default to MB
            kprintf("%.4f MB", (double)value_in_bytes / (1024.0 * 1024.0));
            return true;
    }
}

// ==========================================
// Check Command Implementation
// ==========================================

int8_t cmd_check(int argc, char **argv) {
    // Early return: not enough parameters
    if (argc < 2) return 2; 

    switch (hash(argv[1])) {
        
        // ==========================================
        // Memory (RAM) Checks
        // ==========================================
        case hash("ram"): {
            uint32_t total_mb = get_total_memory_mb();
            uint32_t total_bytes = total_mb * 1024 * 1024;
            uint32_t used_bytes = pmm_get_allocated() + heap_get_allocated();
            uint32_t available_bytes = total_bytes - used_bytes;
            
            // Get format from argv[3] if it exists, else default to 0
            uint32_t print_format = (argc > 3) ? hash(argv[3]) : 0; 
            uint32_t sub_cmd = (argc > 2) ? hash(argv[2]) : 0;

            switch (sub_cmd) {
                case hash("all"):
                    kprintf("System RAM: %d MB\n", total_mb);
                    kprintf("Used RAM: %f MB\n", (double)used_bytes / (1024.0 * 1024.0));
                    kprintf("Available RAM: %f MB\n", (double)available_bytes / (1024.0 * 1024.0));
                    return true;
                
                case hash("av"):
                    cmd_format_memory_info(available_bytes, print_format);
                    kprintf("\n"); 
                    return true;
                
                case hash("adv"):
                    kprintf("Available RAM:\t ");
                    cmd_format_memory_info(available_bytes, print_format);
                    kprintf(" (");
                    cmd_format_memory_info(used_bytes, print_format); 
                    kprintf(" used)\n"); 
                    return true;
                
                case hash("us"):
                    cmd_format_memory_info(used_bytes, print_format);
                    kprintf("\n"); 
                    return true;
                
                default:
                    kprintf("Total System RAM: %d MB\n", total_mb);
                    return true;
            }
        } 

        // ==========================================
        // CPU & Registers Checks
        // ==========================================
        case hash("cs"): {
            uint16_t cs;
            asm volatile ("mov %%cs, %0" : "=r"(cs));
            kprintf("CS = %x\n", cs);        
            return true;
        }

        case hash("stack"): {
            uint64_t rsp;
            asm volatile("mov %%rsp, %0" : "=r"(rsp));
            kprintf("RSP = %lx/%ld\n", rsp, rsp);      
            return true;
        }

        case hash("cpu"): {
            uint32_t ebx, ecx, edx, unused;
            asm volatile("cpuid" : "=a"(unused), "=b"(ebx), "=d"(edx), "=c"(ecx) : "a"(0));
            
            char vendor[13];
            *(uint32_t*)(vendor) = ebx;
            *(uint32_t*)(vendor + 4) = edx;
            *(uint32_t*)(vendor + 8) = ecx;
            vendor[12] = '\0';
            
            kprintf("CPU Vendor ID: %s\n", vendor);
            return true;
        }

        // ==========================================
        // Hardware & Input Checks
        // ==========================================
        case hash("mouse"): {
            kprintf("=== MOUSE STATUS ===\n");
            kprintf("Position : X = %d | Y = %d\n", mouse.x, mouse.y);
            kprintf("Scroll   : Z = %d\n", mouse_scroll);
            return true;
        }

        // ==========================================
        // Utilities (ASCII Table)
        // ==========================================
        case hash("ascii"): {
            kprintf("ASCII Table (Code Page 437) - 8 Columns Layout:\n");

            // Top border (8 columns, each 8 characters wide)
            kprintf("\xDA"); 
            for (int c = 0; c < 8; c++) {
                kprintf("\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4"); 
                if (c < 7) kprintf("\xC2");         
            }
            kprintf("\xBF\n"); 

            // Print the 32 rows
            for (int r = 0; r < 32; r++) {
                kprintf("\xB3"); 
                
                for (int c = 0; c < 8; c++) {
                    int i = (c * 32) + r;
                    
                    // Filter out control characters that would break the terminal layout
                    char display_c = (i == 0 || i == '\b' || i == '\t' || i == '\n' || i == '\r') ? '.' : (unsigned char)i;
                    kprintf(" %3d: %c \xB3", i, display_c);
                }
                kprintf("\n");
            }

            // Bottom border
            kprintf("\xC0"); 
            for (int c = 0; c < 8; c++) {
                kprintf("\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4"); 
                if (c < 7) kprintf("\xC1");         
            }
            kprintf("\xD9\n"); 

            return true;
        }

        // ==========================================
        // System Logs
        // ==========================================
        case hash("logs"): {
            for (size_t i = 0; i < log_index; i++) {
                kprintf("%s\n", klogs[i]);
            }
            return true;
        }

        // ==========================================
        // Invalid Sub-command
        // ==========================================
        default: 
            return 2;
    }
}

// ==========================================
// Module Registration
// ==========================================

void register_check_commands() {
    register_command(
        "check", 
        cmd_check, 
        "Checks system info or prints tables", 
        "[option] [format]\n\toptions: ram [av/us/adv/all], cs, stack, cpu, mouse, ascii, logs\n\tformat (for ram): b, kb, mb, gb"
    );
}