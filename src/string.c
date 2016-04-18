#include "string.h"

int strncmp(const char *str1, const char *str2, uint32_t length)
{
	for (int i = 0; i < (int) length; i++)
		if (str1[i] != str2[i])
			return str1[i] - str2[i];
	return 0;
}

char* strncpy(char *destination, const char *source, uint32_t length)
{
	for (int i = 0; i < (int) length; i++)
		destination[i] = source[i];
	return destination;
}

uint32_t strlen(const char *str)
{
	uint32_t length = 0;
	while (*str != 0)
	{
		str++;
		length++;
	}
	return length;	
}
