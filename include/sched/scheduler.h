#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "mem/kheap.h"
#include "mem/pmm.h"

#define MAX_THREADS         16
#define STACK_SIZE          16384
#define TARGETED_LATENCY    6000000   /* 6ms in nanoseconds */
#define MIN_GRANULARITY     750000    /* 0.75ms in nanoseconds */
#define TIMER_PERIOD_NS     1000000   /* 1ms per tick at 1000Hz */

/* Default weight for priority 0 (nice=0) */
#define WEIGHT_DEFAULT      1024

enum ThreadState {
    THREAD_DEAD,
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_SLEEPING // <--- NUOVO STATO
};

typedef void* (*ThreadFunc)(void* arg);
typedef void  (*ThreadFuncSimple)();

struct Thread {
    uint64_t    rsp;
    uint64_t*   stack_base;
    uint32_t    id;
    ThreadState state;
    void*       arg;
    uint64_t    vruntime;   
    int32_t     priority;       
    uint32_t    weight;
    uint32_t    wake_up_tick;     
};

extern Thread  threads[MAX_THREADS];
extern Thread* current_thread;

void    init_scheduler();
Thread* thread_create(ThreadFunc entry, void* arg = nullptr);
Thread* thread_create(ThreadFuncSimple entry);
void    thread_exit();
uint32_t get_current_thread_id();
void    thread_sleep(uint32_t ms);

#endif