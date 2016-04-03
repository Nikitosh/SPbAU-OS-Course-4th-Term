#include "slab_allocator.h"                                    

#include "buddy_allocator.h"
#include "memory.h"
#include "utilities.h"
#include "lock.h"

struct small_slab_descriptor
{
    struct list_head list_node;
};

struct big_slab_descriptor
{
    struct list_head list_node;
    void *address;
};

static struct slab_allocator big_slab_descriptor_allocator;
static struct slab_allocator slab_allocator_struct_allocator;
static struct spinlock slab_allocator_lock;

void* slab_allocator_allocate_no_lock(struct slab_allocator *allocator);
void slab_allocator_free_no_lock(void *address);

struct slab* init_slab(uint32_t is_small)
{
    uint8_t *address = (uint8_t*) allocate_empty_page(0);
    struct slab *sl = (struct slab*) (address + SMALL_PAGE_SIZE - sizeof(struct slab));
    sl->address = address;
    sl->is_small = is_small;
    list_init(&sl->descriptor_head);
    list_init(&sl->list_node);
    set_slab(sl->address, sl);
    return sl;  
}

struct slab* create_small_slab(uint64_t size, uint64_t alignment)
{
    struct slab *sl = init_slab(SMALL);
    uint8_t *address = (uint8_t*) align(sl->address, alignment);
    uint64_t block_size = sizeof(struct small_slab_descriptor) + size;
    for (; (uint64_t) (address + block_size) <= (uint64_t) sl; address = (uint8_t*) align(address + block_size, alignment))
    {
        struct small_slab_descriptor *descriptor = (struct small_slab_descriptor*) address;
        list_init(&descriptor->list_node);
        list_add(&descriptor->list_node, &sl->descriptor_head); 
    }
    return sl;
}

void* small_slab_allocate(struct slab *sl)
{
    if (list_empty(&sl->descriptor_head))
        return NULL;
    struct small_slab_descriptor *descriptor = LIST_ENTRY(sl->descriptor_head.next, struct small_slab_descriptor, list_node);
    list_del(&descriptor->list_node);
    return ((uint8_t*) descriptor) + sizeof(struct small_slab_descriptor);
}

void small_slab_free(struct slab *sl, void *address)
{
    struct small_slab_descriptor *descriptor = (struct small_slab_descriptor*) ((uint8_t*) address - sizeof(struct small_slab_descriptor));
    list_add(&descriptor->list_node, &sl->descriptor_head); 
}

struct slab* create_big_slab(uint64_t size, uint64_t alignment)
{
    struct slab *sl = init_slab(BIG);
    uint8_t *address = (uint8_t*) align(sl->address, alignment);
    for (; (uint64_t) (address + size) <= (uint64_t) sl; address = (uint8_t*) align(address + size, alignment))
    {
        struct big_slab_descriptor *descriptor = slab_allocator_allocate_no_lock(&big_slab_descriptor_allocator);
        descriptor->address = address;
        list_init(&descriptor->list_node);
        list_add(&descriptor->list_node, &sl->descriptor_head); 
    }
    return sl;
}

void* big_slab_allocate(struct slab *sl)
{
    if (list_empty(&sl->descriptor_head))
        return NULL;
    struct big_slab_descriptor *descriptor = LIST_ENTRY(sl->descriptor_head.next, struct big_slab_descriptor, list_node);
    list_del(&descriptor->list_node);
    slab_allocator_free_no_lock(descriptor);
    return descriptor->address;
}

void big_slab_free(struct slab *sl, void *address)
{
    struct big_slab_descriptor *descriptor = (struct big_slab_descriptor*) slab_allocator_allocate_no_lock(&big_slab_descriptor_allocator);
    descriptor->address = address;
    list_init(&descriptor->list_node);
    list_add(&descriptor->list_node, &sl->descriptor_head);
}

struct slab* create_slab(uint64_t size, uint64_t alignment)
{
    if (size >= BIG_SLAB_OBJECT_SIZE)
        return create_big_slab(size, alignment);
    return create_small_slab(size, alignment);
}

void* slab_allocate(struct slab *sl)
{
    if (sl->is_small == SMALL)
        return small_slab_allocate(sl);
    return big_slab_allocate(sl);   
}

void slab_free(struct slab *sl, void *address)
{
    if (sl->is_small == SMALL)
        small_slab_free(sl, address);
    else
        big_slab_free(sl, address);     
}

struct slab_allocator* create_slab_allocator(uint64_t size, uint64_t alignment)
{
    struct slab_allocator *allocator = (struct slab_allocator*) slab_allocator_allocate(&slab_allocator_struct_allocator);
    allocator->size = size;
    allocator->alignment = alignment;
    list_init(&allocator->slab_head);
    return allocator;
}

struct slab* create_new_slab_for_slab_allocator(struct slab_allocator *allocator)
{
    struct slab *sl = create_slab(allocator->size, allocator->alignment);
    list_add(&sl->list_node, &allocator->slab_head);
    return sl;
}

void* allocate_in_new_slab(struct slab_allocator *allocator)
{
    struct slab *sl = create_new_slab_for_slab_allocator(allocator);
    return slab_allocate(sl);
}

void* slab_allocator_allocate(struct slab_allocator *allocator)
{
    lock(&slab_allocator_lock);
    void *result = slab_allocator_allocate_no_lock(allocator);
    unlock(&slab_allocator_lock);    
    return result;
}

void* slab_allocator_allocate_no_lock(struct slab_allocator *allocator)
{
    if (list_empty(&allocator->slab_head))
        return allocate_in_new_slab(allocator);
    struct list_head *head = &allocator->slab_head;
    for (struct list_head *node = head->next; node != head; node = node->next)
    {
        struct slab *sl = LIST_ENTRY(node, struct slab, list_node);
        if (!list_empty(&sl->descriptor_head))
            return slab_allocate(sl);
    }
    return allocate_in_new_slab(allocator);
}

void slab_allocator_free(void *address)
{
    lock(&slab_allocator_lock);
    slab_allocator_free_no_lock(address);
    unlock(&slab_allocator_lock);    
}

void slab_allocator_free_no_lock(void *address)
{                                    
    slab_free(get_slab(address), address);
}

void init_slab_allocator()
{
    big_slab_descriptor_allocator.size = sizeof(struct big_slab_descriptor);
    big_slab_descriptor_allocator.alignment = 1;
    list_init(&big_slab_descriptor_allocator.slab_head);
    slab_allocator_struct_allocator.size = sizeof(struct slab_allocator);
    slab_allocator_struct_allocator.alignment = 1;
    list_init(&slab_allocator_struct_allocator.slab_head);
    init_lock(&slab_allocator_lock);
}                   
                             