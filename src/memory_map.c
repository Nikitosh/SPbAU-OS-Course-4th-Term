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

void add_descriptor(uint32_t *index, uint64_t base_addr, uint64_t length, uint32_t type)
{
    if (length == 0)
        return;
    memory_map_descriptors[*index].base_addr = base_addr;   
    memory_map_descriptors[*index].length = length; 
    memory_map_descriptors[*index].type = type; 
    (*index)++;
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
    
    uint64_t kernel_left = (uint64_t) text_phys_begin;
    uint64_t kernel_right = (uint64_t) bss_phys_end;

    for (struct size_descriptor *pair = (struct size_descriptor*) (uintptr_t) memory_map_addr; 
        (unsigned long long) pair < memory_map_addr + memory_map_length; 
        pair = (struct size_descriptor*) (uintptr_t) ((uintptr_t) pair + pair->size + sizeof(pair->size)))
    {
        struct memory_map_descriptor *descriptor = &pair->descriptor;
        uint64_t descriptor_left = descriptor->base_addr;
        uint64_t descriptor_right = descriptor->base_addr + descriptor->length;
        if (descriptor_left >= kernel_right || kernel_left >= descriptor_right)
        {
            add_descriptor(&memory_map_size, descriptor_left, descriptor->length, descriptor->type);
            continue;
        }
        if (descriptor_left <= kernel_left && kernel_left < descriptor_right)
        {
            add_descriptor(&memory_map_size, descriptor_left, kernel_left - descriptor_left, descriptor->type);
            if (kernel_right < descriptor_right)
                add_descriptor(&memory_map_size, kernel_right, descriptor_right - kernel_right, descriptor->type);
        }
        if (descriptor_left >= kernel_left && descriptor_left < kernel_right && descriptor_right > kernel_right)
            add_descriptor(&memory_map_size, kernel_right, descriptor_right - kernel_right, descriptor->type);
    }
    add_descriptor(&memory_map_size, kernel_left, kernel_right - kernel_left, RESERVED);
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
            add_descriptor(&memory_map_size, descriptor->base_addr + descriptor->length, size, RESERVED);
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
