#ifndef __STRING_H__
#define __STRING_H__

#include <stdint.h>

int strncmp(const char *str1, const char *str2, uint32_t length);
char* strncpy(char *destination, const char *source, uint32_t length);
uint32_t strlen(const char *str);

#endif /* __STRING_H__ */