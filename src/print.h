#ifndef __PRINT_H__
#define __PRINT_H__

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

int printf(const char *format, ...);
int vprintf(const char *format, va_list arg);
int snprintf(char *s, size_t n, const char *format, ...);
int vsnprintf(char *s, size_t n, const char *format, va_list arg);

void print_test();

#endif /* __PRINT_H__ */