#include "utilities.h"

static unsigned long int next = 1;

int rand() 
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed)
{
    next = seed;
}

void* align(void* pointer, uint64_t alignment)
{
	uint64_t value = (uint64_t) pointer;
 	if (value % alignment)
 		value = value / alignment * alignment + alignment;
 	return (void*) value;
}

uint64_t div_up(uint64_t a, uint64_t b)
{
 	if (a % b)
 		return a / b + 1;
 	return a / b;
}

