#ifndef __SLAB_ALLOCATOR_H__
#define __SLAB_ALLOCATOR_H__

#include <stdint.h>
#include "list.h"

#define BIG_SLAB_OBJECT_SIZE (SMALL_PAGE_SIZE / 8)
#define SMALL 1
#define BIG 0

struct slab
{
    struct list_head list_node;
    struct list_head descriptor_head;
    void *address;
    uint32_t is_small;
};

struct slab_allocator
{
    struct list_head slab_head;
    uint64_t size;
    uint64_t alignment;
};

void init_slab_allocator();
struct slab_allocator* create_slab_allocator(uint64_t size, uint64_t alignment);
void* slab_allocator_allocate(struct slab_allocator *allocator);
void slab_allocator_free(void *address);

#endif /* __SLAB_ALLOCATOR_H__ */