#ifndef CLI_H
#define CLI_H
#include "lib/string.h"
#include "drivers/display/kprintf.h"
#include "drivers/sound/sound.h"
#include "drivers/cmos/timer/timer.h"
#include "drivers/input/keyboard/keyboard.h"
#include "drivers/input/keyboard/command.h"
#include "drivers/input/mouse/mouse.h"
#include "drivers/input/mouse/displayMouse.h"
#include "kernel/mem/pmm.h"
#include "kernel/mem/kheap.h"
#include "kernel/idt/gdt.h"
#include "kernel/idt/idt.h"
#include "kernel/idt/exceptions/tss.h"
#include "drivers/cmos/rtc/rtc.h"
#include "kernel/idt/spurious/spurious.h"
#include "kernel/sched/scheduler.h"
#include "fs/tmpfs.h"
#include "fs/mbr.h"
#include "fs/fat32/fat32.h"
#include "kernel/cpu/cpu.h"
 
extern bool draw_info;

extern void cli_main();
extern void info_bar_thread();
extern void draw_info_bar();
















#endif