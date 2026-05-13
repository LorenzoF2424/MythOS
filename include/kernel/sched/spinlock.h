#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

extern uint32_t get_current_thread_id(); 

struct Spinlock {
    volatile uint32_t locked = 0;
    volatile uint32_t owner = 0xFFFFFFFF; 
    volatile uint32_t count = 0;          
};

inline void spinlock_acquire(Spinlock* lock) {

    uint32_t me = get_current_thread_id();


    if (lock->locked && lock->owner == me) {
        lock->count++;
        return;
    }

    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        asm volatile("pause"); 
    }
    
   
    lock->owner = me;
    lock->count = 1;
}

inline void spinlock_release(Spinlock* lock) {
    
    if (lock->count > 1) {
        lock->count--;
        return; 
    }

    
    lock->owner = 0xFFFFFFFF;
    lock->count = 0;
    __sync_lock_release(&lock->locked);
}



#endif