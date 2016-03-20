#include "utilities.h"

void* align(void* pointer, uint64_t alignment)
{
	uint64_t value = (uint64_t) pointer;
 	if (value % alignment)
 		value = value / alignment * alignment + alignment;
 	return (void*) value;
}
