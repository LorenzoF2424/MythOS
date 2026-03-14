#ifndef RTC_H
#define RTC_H

#include <stdint.h>
#include "cmos/io.h"

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint32_t year;
} rtc_time_t;

extern int8_t system_timezone;
void read_rtc(rtc_time_t* time);
void apply_timezone(rtc_time_t* time);

#endif