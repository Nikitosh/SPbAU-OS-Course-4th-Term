#include "threads.h"

#include "lock.h"
#include "buddy_allocator.h"
#include "slab_allocator.h"
#include "memory.h"

#include "print.h"

static struct slab_allocator *thread_struct_allocator;
static struct spinlock threads_lock;
static struct list_head *volatile threads_list;
static struct thread *volatile current_thread;
static volatile double elapsed_time = 0;

void init_threads() 
{
    thread_struct_allocator = (struct slab_allocator*) create_slab_allocator(sizeof(struct thread), 1);
    current_thread = (struct thread*) slab_allocator_allocate(thread_struct_allocator);
    current_thread->state = RUNNING;
    current_thread->stack_pointer = NULL;
    threads_list = &current_thread->list_node;
    list_init(threads_list);
    init_lock(&threads_lock);
}

void function_wrapper();
                               
struct thread* thread_create(void *(*function_ptr) (void *), void *arg)
{
    lock_with_interrupts(&threads_lock);

    struct thread *t = slab_allocator_allocate(thread_struct_allocator);        
    t->page_address = allocate_page(THREAD_STACK_SIZE_LOG / SMALL_PAGE_BITS);
    uint64_t *stack_pointer = (uint64_t*) (((uint8_t*) t->page_address) + THREAD_STACK_SIZE);
    *(--stack_pointer) = (uint64_t) function_ptr;
    *(--stack_pointer) = (uint64_t) arg;
    *(--stack_pointer) = (uint64_t) &threads_lock;
    *(--stack_pointer) = (uint64_t) &function_wrapper;
    
    uint64_t rflags;
    __asm__ (
    "pushfq;" 
    "pop %%rax;" 
    "mov %%rax, %0" :  :"m" (rflags) 
    );
    uint8_t status_bit_numbers[] = {0, 2, 4, 6, 7, 11};
    for (int i = 0; i < 6; i++)
        rflags &= ~(1 << status_bit_numbers[i]);
    rflags &= ~(1 << DIRECTION_FLAG);
    *(--stack_pointer) = rflags;

    for (int i = 0; i < 6; i++)
        *(--stack_pointer) = 0;
    t->stack_pointer = stack_pointer;
    t->state = RUNNING;
    
    list_init(&t->list_node);
    list_add_tail(&t->list_node, threads_list);
    
    unlock_with_interrupts(&threads_lock);
    return t;
}

void thread_exit(void *value_ptr)
{
    lock_with_interrupts(&threads_lock);
    current_thread->state = FINISHED;
    current_thread->return_value = value_ptr;
    unlock_with_interrupts(&threads_lock);
    thread_yield();
}

void thread_join(struct thread *t, void **value_ptr)
{
    while (t->state != RELEASED)
        thread_yield(); 
    *value_ptr = t->return_value;

    lock_with_interrupts(&threads_lock);
    list_del(&t->list_node);                            
    unlock_with_interrupts(&threads_lock);  

    slab_allocator_free(t);
}

void thread_schedule(double delta)
{
    lock_with_interrupts(&threads_lock);
    elapsed_time += delta;
    if (elapsed_time >= THREAD_QUANTUM)
    {
        unlock_with_interrupts(&threads_lock);
        thread_yield(); 
        return;
    }   
    unlock_with_interrupts(&threads_lock);  
}

void switch_threads(void **old_sp, void *new_sp);

void thread_yield()
{
    lock_with_interrupts(&threads_lock);
    struct thread *old_thread = current_thread;
    current_thread = LIST_ENTRY(current_thread->list_node.next, struct thread, list_node);
    while (current_thread->state == FINISHED || current_thread->state == RELEASED)
    {
        if (current_thread->state == FINISHED)
        {
            current_thread->state = RELEASED;
            free_page(current_thread->page_address, THREAD_STACK_SIZE_LOG);
        }
        current_thread = LIST_ENTRY(current_thread->list_node.next, struct thread, list_node);
    }
    elapsed_time = 0;
    if (current_thread != old_thread)
        switch_threads(&(old_thread->stack_pointer), current_thread->stack_pointer);
    unlock_with_interrupts(&threads_lock);
}

