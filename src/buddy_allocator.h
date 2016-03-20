#ifndef __BUDDY_H__
#define __BUDDY_H__

#include "slab_allocator.h"

void *allocate_page(int level);
void *allocate_empty_page(int level);
void free_page(void *address, int level);
void init_buddy();
void set_slab(void *address, struct slab *sl);
struct slab* get_slab(void *address);

#endif /* __BUDDY_H__ */