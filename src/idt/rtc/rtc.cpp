#include "idt/rtc/rtc.h"
#include "cmos/io.h" // Modifica il percorso se necessario

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

int get_update_in_progress_flag() {
    outb(CMOS_ADDRESS, 0x0A);
    return (inb(CMOS_DATA) & 0x80);
}

uint8_t get_rtc_register(int reg) {
    outb(CMOS_ADDRESS, reg);
    return inb(CMOS_DATA);
}

bool is_leap_year(uint32_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

uint8_t get_days_in_month(uint8_t month, uint32_t year) {
    switch (month) {
        case 2: return is_leap_year(year) ? 29 : 28;
        case 4: case 6: case 9: case 11: return 30;
        default: return 31;
    }
}

void read_rtc(rtc_time_t* t) {
    uint8_t century;
    uint8_t last_second, last_minute, last_hour, last_day, last_month, last_year, last_century;
    uint8_t registerB;

    
    while (get_update_in_progress_flag());

    
    t->second = get_rtc_register(0x00);
    t->minute = get_rtc_register(0x02);
    t->hour   = get_rtc_register(0x04);
    t->day    = get_rtc_register(0x07);
    t->month  = get_rtc_register(0x08);
    t->year   = get_rtc_register(0x09);
    century   = get_rtc_register(0x32); 

    
    do {
        last_second = t->second;
        last_minute = t->minute;
        last_hour = t->hour;
        last_day = t->day;
        last_month = t->month;
        last_year = t->year;
        last_century = century;

        while (get_update_in_progress_flag());
        t->second = get_rtc_register(0x00);
        t->minute = get_rtc_register(0x02);
        t->hour   = get_rtc_register(0x04);
        t->day    = get_rtc_register(0x07);
        t->month  = get_rtc_register(0x08);
        t->year   = get_rtc_register(0x09);
        century   = get_rtc_register(0x32);
    } while ( (last_second != t->second) || (last_minute != t->minute) || (last_hour != t->hour) ||
              (last_day != t->day) || (last_month != t->month) || (last_year != t->year) ||
              (last_century != century) );

    registerB = get_rtc_register(0x0B);

    if (!(registerB & 0x04)) {
        t->second = (t->second & 0x0F) + ((t->second / 16) * 10);
        t->minute = (t->minute & 0x0F) + ((t->minute / 16) * 10);
        t->hour = ( (t->hour & 0x0F) + (((t->hour & 0x70) / 16) * 10) ) | (t->hour & 0x80);
        t->day = (t->day & 0x0F) + ((t->day / 16) * 10);
        t->month = (t->month & 0x0F) + ((t->month / 16) * 10);
        t->year = (t->year & 0x0F) + ((t->year / 16) * 10);
        century = (century & 0x0F) + ((century / 16) * 10);
    }

    if (!(registerB & 0x02) && (t->hour & 0x80)) {
        t->hour = ((t->hour & 0x7F) + 12) % 24;
    }

    t->year += century * 100;
}

int8_t system_timezone = 1; 

void apply_timezone(rtc_time_t* t) {
    int16_t new_hour = t->hour + system_timezone;

    if (new_hour >= 24) {
        new_hour -= 24;
        t->day++;
        
        if (t->day > get_days_in_month(t->month, t->year)) {
            t->day = 1;
            t->month++;
            
            if (t->month > 12) {
                t->month = 1;
                t->year++;
            }
        }
    } 
    else if (new_hour < 0) {
        new_hour += 24;
        t->day--;
        
        if (t->day == 0) {
            t->month--;
            
            if (t->month == 0) {
                t->month = 12;
                t->year--;
            }
            t->day = get_days_in_month(t->month, t->year);
        }
    }

    t->hour = (uint8_t)new_hour;
}