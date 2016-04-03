#ifndef __THREADS_H__
#define __THREADS_H__

#include "list.h"

#define THREAD_STACK_SIZE_LOG 12
#define THREAD_STACK_SIZE     (1 << THREAD_STACK_SIZE_LOG)
#define THREAD_QUANTUM        0.005
#define DIRECTION_FLAG        10

typedef enum 
{
	RUNNING, 
	FINISHED,
	RELEASED
} thread_state;

struct thread
{
	thread_state state;
	void *page_address;
	void *stack_pointer;
	void *return_value;
	struct list_head list_node;
};

void init_threads();
struct thread* thread_create(void *(*function_ptr) (void *), void *arg);
void thread_exit(void *value_ptr);
void thread_join(struct thread *t, void **value_ptr);
void thread_yield();
void thread_schedule(double delta);

#endif /* __THREADS_H__ */