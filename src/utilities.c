#include "utilities.h"
#include "memory.h"

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

uint32_t convert_from_hex(const char *str, int len)
{                                
	uint32_t result = 0;
	for (int i = 0; i < len; i++)
	{
		int digit = str[i] - '0';
		if (str[i] >= 'A' && str[i] <= 'F')
			digit = 10 + str[i] - 'A';
		result = 16 * result + digit;
	}
	return result;
}
                
uint32_t get_size_level(uint64_t size)
{
	size = div_up(size, SMALL_PAGE_SIZE);
	uint32_t result = 0;
	while (size > 1)
	{
		size = (size + 1) / 2;
		result++;
	}	
	return result;
}
