#include "memory_map.h"

#include "utilities.h"
#include "print.h"
#include "uart.h"
#include "utilities.h"

extern const uint32_t mboot_info;

extern char text_phys_begin[];
extern char bss_phys_end[];

struct size_descriptor 
{
    uint32_t size;
    struct memory_map_descriptor descriptor;
} __attribute__((packed));

struct memory_map_descriptor memory_map_descriptors[MEMORY_MAP_SIZE];
uint32_t memory_map_size = 0;

void add_descriptor(struct memory_map_descriptor *descriptors, uint32_t *index, uint64_t base_addr, uint64_t length, uint32_t type)
{
    if (length == 0)
        return;
    descriptors[*index].base_addr = base_addr;   
    descriptors[*index].length = length; 
    descriptors[*index].type = type; 
    (*index)++;
}              

void mark_as_reserved(uint64_t left, uint64_t right)
{
	struct memory_map_descriptor memory_map_descriptors_temp[MEMORY_MAP_SIZE];
	uint32_t memory_map_size_temp = 0;
	for (int i = 0; i < (int) memory_map_size; i++)
	{
		uint64_t descriptor_left = memory_map_descriptors[i].base_addr;
		uint64_t descriptor_right = memory_map_descriptors[i].base_addr + memory_map_descriptors[i].length;
		if (descriptor_left >= right || left >= descriptor_right)
		{
			memory_map_descriptors_temp[memory_map_size_temp++] = memory_map_descriptors[i];
			continue;
	   	}
	   	if (left <= descriptor_left && right >= descriptor_right)
	   		continue;
	   	if (descriptor_left < left)
	   	{
	   		add_descriptor(memory_map_descriptors_temp, &memory_map_size_temp, 
	   			descriptor_left, left - descriptor_left, memory_map_descriptors[i].type);
	   	}
	   	if (descriptor_right > right)
	   	{
	   		add_descriptor(memory_map_descriptors_temp, &memory_map_size_temp, 
	   			right, descriptor_right - right, memory_map_descriptors[i].type);
	   	}
	}
	memory_map_size = memory_map_size_temp;
	for (int i = 0; i < (int) memory_map_size; i++)
		memory_map_descriptors[i] = memory_map_descriptors_temp[i];
}

void get_memory_map()
{
    uint32_t flags = * (uint32_t*) (uintptr_t) mboot_info;
    if (!get_bit(flags, MEMORY_MAP_BIT))
    {
        puts("Memory map is unavailable");
        return;
    }   
    uint32_t memory_map_length = *(uint32_t*) (uintptr_t) (mboot_info + MEMORY_MAP_LENGTH_OFFSET);
    uint32_t memory_map_addr = *(uint32_t*) (uintptr_t) (mboot_info + MEMORY_MAP_ADDR_OFFSET);
    
	for (struct size_descriptor *pair = (struct size_descriptor*) (uintptr_t) memory_map_addr; 
        (unsigned long long) pair < memory_map_addr + memory_map_length; 
        pair = (struct size_descriptor*) (uintptr_t) ((uintptr_t) pair + pair->size + sizeof(pair->size)))
    {
        struct memory_map_descriptor *descriptor = &pair->descriptor;
        add_descriptor(memory_map_descriptors, &memory_map_size, descriptor->base_addr, descriptor->length, descriptor->type);
    }

    uint64_t kernel_left = (uint64_t) text_phys_begin;
    uint64_t kernel_right = (uint64_t) bss_phys_end;
	mark_as_reserved(kernel_left, kernel_right);
}

uint32_t get_memory_map_size()
{
    return memory_map_size;
}

uint64_t memory_map_allocate(uint64_t size)
{
    for (int i = 0; i < (int) memory_map_size; i++)
    {
        struct memory_map_descriptor *descriptor = &memory_map_descriptors[i];
        if (descriptor->type == AVAILABLE && descriptor->length >= size)
        {
            descriptor->length -= size;
            add_descriptor(memory_map_descriptors, &memory_map_size, descriptor->base_addr + descriptor->length, size, RESERVED);
            return descriptor->base_addr + descriptor->length;
        }
    }
    return 0;
}
                     	
uint64_t get_memory_size()
{
    uint64_t memory_size = 0;
    for (int i = 0; i < (int) memory_map_size; i++)
        memory_size = max(memory_size, memory_map_descriptors[i].base_addr + memory_map_descriptors[i].length);
    return memory_size;
}

void print_memory_map()
{
    for (int i = 0; i < (int) memory_map_size; i++)
    {
        struct memory_map_descriptor *descriptor = &memory_map_descriptors[i];
        printf("%llx-%llx: %u\n", descriptor->base_addr, descriptor->base_addr + descriptor->length, descriptor->type);
    }
}
