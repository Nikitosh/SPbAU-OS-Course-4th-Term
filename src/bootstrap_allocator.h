#ifndef __BOOTSTRAP_ALLOCATOR_H__
#define __BOOTSTRAP_ALLOCATOR_H__

#include <stdint.h>

void bootstrap_init(uint64_t size);
void* bootstrap_allocate(uint64_t size);
void* bootstrap_allocate_with_alignment(uint64_t size, uint64_t alignment);

#endif /* __BOOTSTRAP_ALLOCATOR_H__ */