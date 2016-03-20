#include "test.h"

#include "print.h"
#include "buddy_allocator.h"
#include "slab_allocator.h"
#include "memory.h"

#define RUN_NUMBER 200

void test_print()
{
	int k = 105;
	printf("%d aba %i %u %o %x %c %s %p %lld %hd\n", 105, 205, -342, 10, 123123, 98344, "kkookk", &k, (1ll << 62), (1ll << 31));
 	char buffer[12];
 	int l = snprintf(buffer, 10, "%d %d %d %d\n", 1456, 1235, 1235, 2);
	printf("%d\n", l);
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
		printf("%p %p %p %p %p\n", p1, p2, p3, p4, p5);
		free_page(p4, 0);
		free_page(p5, 0);
		free_page(p3, 3);
		free_page(p2, 1);
		free_page(p1, 0);
	}
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
}

void test_all()
{
	test_print();
	test_buddy_allocator();
	test_slab_allocator();
}