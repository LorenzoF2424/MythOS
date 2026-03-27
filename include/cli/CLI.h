#ifndef CLI_H
#define CLI_H
#include "gnu_utils/string.h"
#include "term/kprintf.h"
#include "idt/sound/sound.h"
#include "idt/timer/timer.h"
#include "idt/keyboard/keyboard.h"
#include "idt/keyboard/command.h"
#include "idt/mouse/mouse.h"
#include "idt/mouse/displayMouse.h"
#include "mem/pmm.h"
#include "mem/vmm.h"
#include "mem/kheap.h"
#include "idt/gdt.h"
#include "idt/idt.h"
#include "idt/exceptions/tss.h"
#include "idt/rtc/rtc.h"
#include "idt/spurious/spurious.h"
#include "sched/scheduler.h"

 
extern bool draw_info;

extern void cli_main();
extern void info_bar_thread();
extern void draw_info_bar();
















#endif