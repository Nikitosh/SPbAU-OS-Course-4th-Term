#include "test.h"

#include "print.h"
#include "buddy_allocator.h"
#include "slab_allocator.h"
#include "memory.h"
#include "threads.h"
#include "lock.h"

#define RUN_NUMBER    200
#define THREAD_NUMBER 30

void test_print()
{
    int k = 105;
    printf("%d aba %i %u %o %x %c %s %p %lld %hd\n", 105, 205, -342, 10, 123123, 98344, "kkookk", &k, (1ll << 62), (1ll << 31));
    char buffer[12];
    int l = snprintf(buffer, 10, "%d %d %d %d\n", 1456, 1235, 1235, 2);
    printf("%d\n", l);
    printf("print test: OK\n");
}

void test_buddy_allocator()
{
    for (int i = 0; i < RUN_NUMBER; i++)
    {
        void *p1 = allocate_empty_page(0);
        void *p2 = allocate_empty_page(1);
        void *p3 = allocate_empty_page(3);
        void *p4 = allocate_empty_page(0);
        void *p5 = allocate_empty_page(0);
        printf("%d %p %p %p %p %p\n", i, p1, p2, p3, p4, p5);
        free_page(p4, 0);
        free_page(p5, 0);
        free_page(p3, 3);
        free_page(p2, 1);
        free_page(p1, 0);
    }
    printf("buddy_allocator test: OK\n");
}

void *pointers_small[RUN_NUMBER];
void *pointers_big[RUN_NUMBER];

void test_slab_allocator()
{
    struct slab_allocator *allocator_small = create_slab_allocator(8, 1);
    struct slab_allocator *allocator_big = create_slab_allocator(SMALL_PAGE_SIZE / 8, SMALL_PAGE_SIZE / 2);

    for (int i = 0; i < RUN_NUMBER; i++)
    {
        pointers_small[i] = slab_allocator_allocate(allocator_small);
        pointers_big[i] = slab_allocator_allocate(allocator_big);
        printf("%p %p\n", pointers_small[i], pointers_big[i]);
    }
    for (int i = 0; i < RUN_NUMBER; i++)
    {
        slab_allocator_free(pointers_small[i]);
        slab_allocator_free(pointers_big[i]);
    }   
    printf("slab_allocator test: OK\n");
}

struct spinlock print_lock;

void* print(void* arg)
{
    if (rand() % 2 == 1)
        thread_yield();
    lock(&print_lock);
    int value = (int) (uint64_t) arg;
    printf("Print: %d\n", value);   
    unlock(&print_lock);
    return arg;
}

void test_thread_create()
{
    init_lock(&print_lock);
    for (int i = 0; i < THREAD_NUMBER; i++)
        thread_create(print, (void*) (uint64_t) i);
    lock(&print_lock);
    printf("thread_create test: OK\n");
    unlock(&print_lock);
}

int value = 0;

void* function1(void *arg)
{
    thread_yield();
    value = 1;
    return arg;
}

void* function2(void *arg)
{
    value = 2;
    return arg;
}

void test_thread_yield()
{
    struct thread *thread1 = thread_create(function1, NULL);    
    struct thread *thread2 = thread_create(function2, NULL);    
    thread_join(thread1, NULL);
    thread_join(thread2, NULL);
    if (value != 1)
        printf("yield test: FAILED!\n");
    else
        printf("yield test: OK\n");
}

struct thread *thread_pointer[THREAD_NUMBER];
int result[THREAD_NUMBER];

void* return_arg(void *arg)
{
    return arg;
}

void test_thread_join()
{
    init_lock(&print_lock);
    for (int i = 0; i < THREAD_NUMBER; i++)
        thread_pointer[i] = thread_create(return_arg, (void*) (uint64_t) i);
    int ok = 1;
    for (int i = 0; i < THREAD_NUMBER; i++)
    {
        thread_join(thread_pointer[i], (void**) &result[i]);
        if (result[i] != i) 
        {
            printf("join test: FAILED!\n");
            ok = 0;
        }
    }
    if (ok)
        printf("join test: OK\n");
}

void test_threads()
{
    test_thread_create();
    test_thread_yield();
    test_thread_join();
}

void test_all()
{
    //test_print();
    //test_buddy_allocator();
    //test_slab_allocator();
    test_threads();
}