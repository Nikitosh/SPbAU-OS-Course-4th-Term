#include "utilities.h"

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
