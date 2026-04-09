#include "cli/commands.h"
#include "idt/rtc/rtc.h"


// ==========================================
// Time Command Implementation
// ==========================================

int8_t cmd_time(int argc, char **argv) {
    // Early return: not enough arguments provided
    if (argc < 2) return 2;

    // Read the Real-Time Clock and apply the current system timezone
    rtc_time_t t;
    read_rtc(&t);
    apply_timezone(&t);

    switch (hash(argv[1])) {
        
        // ==========================================
        // Print Full Date and Time
        // ==========================================
        case hash("all"): 
            kprintf("%02d/%02d/%d - %02d:%02d:%02d\n", t.day, t.month, t.year, t.hour, t.minute, t.second); 
            return true;
        
        // ==========================================
        // Print Date Only
        // ==========================================
        case hash("date"):    
            kprintf("%02d/%02d/%d\n", t.day, t.month, t.year); 
            return true;
        
        // ==========================================
        // Print Clock Only
        // ==========================================
        case hash("clock"):   
            kprintf("%02d:%02d\n", t.hour, t.minute); 
            return true;
        
        // ==========================================
        // Timezone Management
        // ==========================================
        case hash("zone"): {
            // Early return: if no new timezone is provided, just print the current one
            if (argc < 3) {
                kprintf("Current timezone: UTC %s%d\n", (system_timezone >= 0) ? "+" : "", system_timezone);
                return true;
            }
            
            int8_t new_tz = (int8_t)atoi(argv[2]);
            
            // Early return: validate timezone boundaries (-12 to +14)
            if (new_tz < -12 || new_tz > 14) {
                kprintf("Error: Invalid timezone. Must be between -12 and +14.\n");
                return 2; 
            }
            
            // Apply the valid timezone
            system_timezone = new_tz;
            kprintf("Timezone updated to UTC %s%d\n", (system_timezone >= 0) ? "+" : "", system_timezone);
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

void register_time_commands() {
    register_command(
        "time", 
        cmd_time, 
        "Displays or configures current date and time", 
        "[option]\n\toptions: all, date, clock, zone [offset]"
    );
}