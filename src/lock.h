#ifndef __LOCK_H__
#define __LOCK_H__

#include <stdint.h>

struct spinlock
{
    uint16_t users;
    uint16_t ticket;
};

void init_lock(struct spinlock *lock);
void lock(struct spinlock *lock);
void unlock(struct spinlock *lock);
void lock_with_interrupts(struct spinlock *lock);
void unlock_with_interrupts(struct spinlock *lock);

#endif /* __LOCK_H__ */