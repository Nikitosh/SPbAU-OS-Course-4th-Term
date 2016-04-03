#include "lock.h"

#include "interrupt.h"
#include "utilities.h"                     

static uint64_t lock_nesting_depth;

void enable_interrupts() 
{
	sti();
}

void disable_interrupts() 
{
	cli();
}

void init_lock(struct spinlock *lock)
{
	lock->users = 0;
	lock->ticket = 0;	  	
}

void lock(struct spinlock *lock)
{
 	uint16_t ticket = __sync_fetch_and_add(&lock->users, 1);
 	while (lock->ticket != ticket)
 		barrier();
 	smp_mb();
}

void unlock(struct spinlock *lock)
{
 	smp_mb();
 	__sync_add_and_fetch(&lock->ticket, 1);
}

void lock_with_interrupts(struct spinlock *spinlock)
{
    disable_interrupts();
    lock(spinlock);
 	__sync_add_and_fetch(&lock_nesting_depth, 1);
}

void unlock_with_interrupts(struct spinlock *lock)
{
    unlock(lock);
 	uint64_t depth = __sync_sub_and_fetch(&lock_nesting_depth, 1);
 	if (depth == 0)
    	enable_interrupts();
}
