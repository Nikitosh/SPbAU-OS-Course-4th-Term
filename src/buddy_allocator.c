#include "buddy_allocator.h"

#include <stdint.h>
#include <assert.h>
#include "list.h"
#include "memory_map.h"
#include "paging.h"
#include "bootstrap_allocator.h"
#include "utilities.h"

struct list_head *level_head;
int level_number;
int descriptors_size;

struct buddy_allocator_descriptor
{
	struct list_head list_node;
	uint32_t level;
	uint32_t is_free;
	struct slab *sl;
} __attribute((packed))__;

struct buddy_allocator_descriptor *descriptors;

int get_buddy(int id, int level)
{
 	return id ^ (1 << level);
}

void* allocate_page(int level)
{
 	for (int current_up_level = level; current_up_level < level_number; current_up_level++)
 	{
 	 	if (level_head[current_up_level].next != &level_head[current_up_level])
 	 	{
 	 		int id = LIST_ENTRY(level_head[current_up_level].prev, struct buddy_allocator_descriptor, list_node) - descriptors;
 	 	 	list_del(level_head[current_up_level].prev);
 	 	    for (int current_down_level = current_up_level - 1; current_down_level >= level; current_down_level--)
 	 	    {
 	 	    	int buddy_id = get_buddy(id, current_down_level);
 	 	    	descriptors[buddy_id].is_free = 1;
 	 	    	descriptors[buddy_id].level = current_down_level;
 	 	    	list_add_tail(&descriptors[buddy_id].list_node, &level_head[current_down_level]);	
 	 	   	} 
 	 	   	descriptors[id].is_free = 0;
 	 	   	descriptors[id].level = level;
 	 	   	return va(id * (uint64_t) SMALL_PAGE_SIZE);
 	 		break;
 	 	}
 	}
 	return 0;
}	

void* allocate_empty_page(int level)
{
	uint8_t *pointer = (uint8_t*) allocate_page(level);
	for (int i = 0; i < SMALL_PAGE_SIZE; i++)
		pointer[i] = 0;
	return (void*) pointer;
}

void free_page(void *address, int level)
{
	int id = pa(address) / SMALL_PAGE_SIZE;
	for (int current_level = level; current_level < level_number; current_level++)
	{
		int buddy_id = get_buddy(id, current_level);
		if (buddy_id >= descriptors_size || !descriptors[buddy_id].is_free || (int) descriptors[buddy_id].level != current_level)
		{
			list_add_tail(&descriptors[id].list_node, &level_head[current_level]);
			descriptors[id].is_free = 1;
			descriptors[id].level = current_level;
			break;
		}
		else
		{
			list_del(&descriptors[buddy_id].list_node);
			id = min(id, buddy_id);
		}
	}
}

void add_page(int id)
{
	free_page(va(id * SMALL_PAGE_SIZE), 0);
}

void init_buddy()
{
	uint64_t memory_size = get_memory_size();
	descriptors_size = memory_size / SMALL_PAGE_SIZE;
	level_number = 1;
	while ((1ll << level_number) < descriptors_size)
		level_number++;
	level_number++;
	
	uint64_t level_head_memory = level_number * sizeof(struct list_head);
	uint64_t descriptors_memory = descriptors_size * sizeof(struct buddy_allocator_descriptor);
   	bootstrap_init(level_head_memory + descriptors_memory);
   	level_head = (struct list_head*) (uint64_t*) bootstrap_allocate(level_head_memory);
	descriptors = (struct buddy_allocator_descriptor*) (uint64_t*) bootstrap_allocate(descriptors_memory);
   	for (int i = 0; i < level_number; i++)
		list_init(&level_head[i]);
   	for (int i = 0; i < descriptors_size; i++)
   	{
   		descriptors[i].is_free = 0;
   		list_init(&descriptors[i].list_node);
   	}
   	uint32_t memory_map_size = get_memory_map_size();
   	for (int i = 0; i < (int) memory_map_size; i++)
		if (memory_map_descriptors[i].type == AVAILABLE)
		{
		 	uint64_t left_bound = memory_map_descriptors[i].base_addr;
		 	if (left_bound % SMALL_PAGE_SIZE)
		 		left_bound = left_bound / SMALL_PAGE_SIZE * SMALL_PAGE_SIZE + SMALL_PAGE_SIZE;
			for (; left_bound + SMALL_PAGE_SIZE <= memory_map_descriptors[i].base_addr + memory_map_descriptors[i].length; left_bound += SMALL_PAGE_SIZE)
			{
				descriptors[left_bound / SMALL_PAGE_SIZE].is_free = 1;
				add_page(left_bound / SMALL_PAGE_SIZE);
			}
		}
}

void set_slab(void *address, struct slab *sl)
{
	descriptors[pa(address) / SMALL_PAGE_SIZE].sl = sl;
}

struct slab* get_slab(void *address)
{
 	return descriptors[pa(address) / SMALL_PAGE_SIZE].sl;
}