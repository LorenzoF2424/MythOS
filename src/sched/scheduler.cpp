#include "sched/scheduler.h"

extern volatile uint64_t ticks;
Thread  threads[MAX_THREADS];
Thread* current_thread = nullptr;

static uint32_t next_id      = 0;
static uint64_t tick_counter = 0;   /* counts ticks since last switch */

/* priority value to weight table (same as Linux kernel) */
static const uint32_t pvalue_to_weight[40] = {
    /* -20 to -1 */
    88761, 71755, 56483, 46273, 36291,
    29154, 23254, 18705, 14949, 11916,
     9548,  7620,  6100,  4904,  3906,
     3121,  2501,  1991,  1586,  1277,
    /* 0 to +19 */
     1024,   820,   655,   526,   423,
      335,   272,   215,   172,   137,
      110,    87,    70,    56,    45,
       36,    29,    23,    18,    15,
};

static uint32_t pvalue_to_w(int32_t pvalue) {
    if (pvalue < -20) pvalue = -20;
    if (pvalue >  19) pvalue =  19;
    return pvalue_to_weight[pvalue + 20];
}

/* Returns number of currently active (non-dead) threads */
static int count_active_threads() {
    int count = 0;
    for (int i = 0; i < MAX_THREADS; i++)
        if (threads[i].state != THREAD_DEAD) count++;
    return count;
}

/* Calculates the time slice for a thread in ticks based on CFS */
static uint64_t calc_timeslice(Thread* t) {
    int active = count_active_threads();
    if (active == 0) active = 1;

    /* Targeted latency divided among active threads, weighted by priority */
    uint64_t total_weight = 0;
    for (int i = 0; i < MAX_THREADS; i++)
        if (threads[i].state != THREAD_DEAD)
            total_weight += threads[i].weight;

    if (total_weight == 0) total_weight = 1;

    /* Time slice in nanoseconds for this thread */
    uint64_t slice_ns = (TARGETED_LATENCY * t->weight) / total_weight;

    /* Enforce minimum granularity */
    if (slice_ns < MIN_GRANULARITY) slice_ns = MIN_GRANULARITY;

    /* Convert to ticks (1 tick = TIMER_PERIOD_NS) */
    uint64_t slice_ticks = slice_ns / TIMER_PERIOD_NS;
    if (slice_ticks == 0) slice_ticks = 1;

    return slice_ticks;
}

void init_scheduler() {
    for (int i = 0; i < MAX_THREADS; i++) {
        threads[i].state      = THREAD_DEAD;
        threads[i].stack_base = nullptr;
        threads[i].vruntime   = 0;
        threads[i].priority   = 0;
        threads[i].weight     = WEIGHT_DEFAULT;
    }

    threads[0].id         = next_id++;
    threads[0].state      = THREAD_RUNNING;
    threads[0].stack_base = nullptr;
    threads[0].priority   = 0;
    threads[0].weight     = pvalue_to_w(0);

    current_thread = &threads[0];
}

Thread* thread_create(ThreadFunc entry, void* arg) {
    int slot = -1;
    for (int i = 1; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_DEAD) { slot = i; break; }
    }
    if (slot == -1) return nullptr;

    Thread* t = &threads[slot];
    t->stack_base = (uint64_t*)pmm_alloc_blocks(2);
    if (!t->stack_base) return nullptr;

    uint64_t* sp = (uint64_t*)((uint8_t*)t->stack_base + STACK_SIZE);

    uint16_t cs, ss;
    asm volatile("mov %%cs, %0" : "=r"(cs));
    asm volatile("mov %%ss, %0" : "=r"(ss));

    *(--sp) = (uint64_t)thread_exit;
    uint64_t* thread_rsp = sp;

    /* iretq frame */
    *(--sp) = (uint64_t)ss;
    *(--sp) = (uint64_t)thread_rsp;
    *(--sp) = 0x202;            /* RFLAGS: IF=1 */
    *(--sp) = (uint64_t)cs;
    *(--sp) = (uint64_t)entry;

    /* pushall order (fake register context):
     * i=0  -> rax
     * i=1  -> rbx
     * i=2  -> rcx
     * i=3  -> rdx
     * i=4  -> rsi
     * i=5  -> rdi  <- arg goes here
     * i=6  -> rbp
     * i=7  -> r8
     * i=8  -> r9
     * i=9  -> r10
     * i=10 -> r11
     * i=11 -> r12
     * i=12 -> r13
     * i=13 -> r14
     * i=14 -> r15
     */
    for (int i = 0; i < 15; i++) {
        if (i == 5) *(--sp) = (uint64_t)arg;
        else        *(--sp) = 0;
    }

    /* New thread starts with vruntime equal to the minimum among active threads
     * so it gets scheduled soon but doesn't starve others */
    uint64_t min_vruntime = UINT64_MAX;
    for (int i = 0; i < MAX_THREADS; i++)
        if (threads[i].state != THREAD_DEAD && threads[i].vruntime < min_vruntime)
            min_vruntime = threads[i].vruntime;
    if (min_vruntime == UINT64_MAX) min_vruntime = 0;

    t->rsp      = (uint64_t)sp;
    t->id       = next_id++;
    t->state    = THREAD_READY;
    t->arg      = arg;
    t->vruntime = min_vruntime;
    t->priority     = 0;
    t->weight   = pvalue_to_w(0);

    return t;
}

Thread* thread_create(ThreadFuncSimple entry) {
    return thread_create((ThreadFunc)[](void* arg) -> void* {
        ThreadFuncSimple f = (ThreadFuncSimple)arg;
        f();
        return nullptr;
    }, (void*)entry);
}

void thread_exit() {
    current_thread->state = THREAD_DEAD;
    pmm_free_blocks(current_thread->stack_base, 2);
    while (true) asm volatile("hlt");
}

extern "C" uint64_t scheduler_switch(uint64_t old_rsp) {
    if (current_thread == nullptr) return old_rsp; // Safety check

    // ==========================================
    // 1. THE WAKE-UP CALL: Check sleeping threads
    // ==========================================
    // This check is performed at EVERY interrupt tick.
    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_SLEEPING) {
            // If the global time has reached the thread's wake-up time, make it READY
            if (ticks >= threads[i].wake_up_tick) {
                threads[i].state = THREAD_READY;
            }
        }
    }

    // ==========================================
    // 2. TIMESLICE CHECK & STATE CHANGE
    // ==========================================
    bool force_switch = false;

    // If the current thread went to sleep voluntarily (or is dead), 
    // we force a context switch immediately, bypassing the timeslice.
    if (current_thread->state != THREAD_RUNNING) {
        force_switch = true;
    } else {
        // Otherwise, check if it has exhausted its allocated time slice
        tick_counter++;
        uint64_t slice = calc_timeslice(current_thread);
        
        if (tick_counter >= slice) {
            force_switch = true;
        }
    }

    // If we don't need to switch threads, continue the current execution
    if (!force_switch) {
        return old_rsp;
    }

    // ==========================================
    // 3. VRUNTIME AND CONTEXT UPDATE
    // ==========================================
    /* Update vruntime of current thread.
     * We use the actually elapsed ticks (tick_counter). If the thread
     * yielded the CPU early (e.g., by sleeping), its vruntime will grow less, 
     * effectively rewarding it with higher priority later!
     */
    uint32_t ticks_elapsed = (tick_counter > 0) ? tick_counter : 1; 

    current_thread->vruntime += (ticks_elapsed * TIMER_PERIOD_NS * WEIGHT_DEFAULT) 
                                / current_thread->weight;

    tick_counter = 0; // Reset the counter for the next thread

    /* Save current RSP */
    current_thread->rsp = old_rsp;
    
    // If the thread was running normally, demote it to READY.
    // (This if statement is vital: if the state was SLEEPING, we must not overwrite it with READY!)
    if (current_thread->state == THREAD_RUNNING) {
        current_thread->state = THREAD_READY;
    }

    // ==========================================
    // 4. PICK THE NEXT THREAD (CFS Core)
    // ==========================================
    /* Pick the READY thread with the smallest vruntime */
    Thread* next     = nullptr;
    uint64_t min_vrt = UINT64_MAX;

    for (int i = 0; i < MAX_THREADS; i++) {
        if (threads[i].state == THREAD_READY && threads[i].vruntime < min_vrt) {
            min_vrt = threads[i].vruntime;
            next    = &threads[i];
        }
    }

    if (next == nullptr) {
        /* No READY thread available. 
         * If the current thread still wants to run, give it the CPU back. */
        if (current_thread->state == THREAD_READY) {
            current_thread->state = THREAD_RUNNING;
            return old_rsp;
        }
        
        /* If all threads (including the current one) are sleeping,
         * we simply return old_rsp. The CPU will execute 'hlt' until 
         * the timer ticks enough times to wake at least one thread up. */
        return old_rsp; 
    }

    // ==========================================
    // 5. EXECUTION
    // ==========================================
    next->state    = THREAD_RUNNING;
    current_thread = next;

    return next->rsp;
}
uint32_t get_current_thread_id() {
    if (current_thread != nullptr) return current_thread->id;
    return 0xFFFFFFFF; 
}

void thread_sleep(uint32_t ms) {
    
    current_thread->wake_up_tick = ticks + ms;
    
    current_thread->state = THREAD_SLEEPING;
    
    
    while (current_thread->state == THREAD_SLEEPING) {
        asm volatile("hlt");
    }
}