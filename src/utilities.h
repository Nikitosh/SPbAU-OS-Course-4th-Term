#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdint.h>

#define bit(x) (1ll << (x))
#define get_bits(x, l, r) (((x) >> (l)) & (bit((r) - (l)) - 1)) //[l, r)
#define get_bit(x, b) get_bits(x, b, (b) + 1)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define barrier() __asm__ __volatile__("": : :"memory")
#define smp_mb() __sync_synchronize()

void srand(unsigned int seed);
int rand();
void* align(void* pointer, uint64_t alignment);
uint64_t div_up(uint64_t a, uint64_t b);
uint32_t convert_from_hex(const char *str, int len);
uint32_t get_size_level(uint64_t size);

#endif /* __UTILITIES_H__ */
