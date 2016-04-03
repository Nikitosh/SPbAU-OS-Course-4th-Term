#ifndef __MEMORY_MAP_H__
#define __MEMORY_MAP_H__

#include <stdint.h>

#define MEMORY_MAP_BIT 6
#define MEMORY_MAP_LENGTH_OFFSET 44
#define MEMORY_MAP_ADDR_OFFSET 48
#define MEMORY_MAP_SIZE 32
#define RESERVED 0
#define AVAILABLE 1

struct memory_map_descriptor
{
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} __attribute__((packed));

extern struct memory_map_descriptor memory_map_descriptors[MEMORY_MAP_SIZE];

void get_memory_map();
uint64_t memory_map_allocate(uint64_t size);
uint32_t get_memory_map_size();
uint64_t get_memory_size();
void print_memory_map();

#endif /* __MEMORY_MAP_H__ */

