#include "bootstrap_allocator.h"

#include "memory_map.h"
#include "memory.h"
#include "paging.h"

void *address;

void bootstrap_init(uint64_t size)
{
	address = va(memory_map_allocate(size));
}

void* bootstrap_allocate(uint64_t size)
{
	void *old_address = address;
	address = (void*) ((uint8_t*) address + size);
	return old_address;	                                        	
}