#include "print.h"
#include "uart.h"

#define MAXN 70

size_t strlen(const char *s)
{
	size_t len = 0;
	while (*s)
	{
	 	len++;
	 	s++;
	}
	return len;
}

void print_symb(char **buf, char c, size_t *print_length, size_t n)
{
	(*print_length)++;
	if ((*buf) != NULL && *print_length == n)
		return;
	if (*buf == NULL)
 		putc(c);
 	else
 		*(*buf)++ = c;
}

int printf(const char *format, ...) 
{
	va_list args;
	va_start(args, format);
	int result = vprintf(format, args);
	va_end(args);
	return result;
}

int vprintf(const char *format, va_list args)
{
    return vsnprintf(NULL, 0, format, args);
}

int snprintf(char *s, size_t n, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	int result = vsnprintf(s, n, format, args);
	va_end(args);
	return result;
}

void print_sign_and_null(char **buf, long long *x, size_t *print_length, size_t n)
{
	if (*x < 0)
	{
		print_symb(buf, '-', print_length, n);
		*x *= -1;
	}
	if (*x == 0)
		print_symb(buf, '0', print_length, n);
}
                      
void print_dec_unsigned(char **buf, unsigned long long x, size_t *print_length, int base, size_t need_length, size_t n)
{
    char buffer[MAXN];
	int pos = 0;
	if (x == 0)
		print_symb(buf, '0', print_length, n);
	while (x > 0)
	{
		int digit = x % base;
		if (digit < 10)
        	buffer[pos++] = '0' + digit;
        else
        	buffer[pos++] = 'a' + digit - 10;
		x /= base;
	}
	while (pos < (int) need_length)
		buffer[pos++] = '0';
	for (int i = pos - 1; i >= 0; i--)
		print_symb(buf, buffer[i], print_length, n);
} 

void print_number(char **buf, long long x, size_t *print_length, int base, size_t need_length, size_t n)
{
	char buffer[MAXN];
	int pos = 0;
	print_sign_and_null(buf, &x, print_length, n);
	while (x > 0)
	{
		int digit = x % base;
		if (digit < 10)
        	buffer[pos++] = '0' + digit;
        else
        	buffer[pos++] = 'a' + digit - 10;
		x /= base;
	}
	while (pos < (int) need_length)
		buffer[pos++] = '0';
	for (int i = pos - 1; i >= 0; i--)
		print_symb(buf, buffer[i], print_length, n);
}

int vsnprintf(char *s, size_t n, const char *format, va_list args)
{
	size_t print_length = 0;
	size_t it = 0;
	size_t len = strlen(format);
	while (it < len)
	{
	 	if (format[it] == '%')
	 	{
	 		it++;
	 		unsigned long long x = 0;
	 		int was = 0;
	 		if (format[it] == 'h')
	 		{
	 			was = 1;
	 			it++;
	 			if (format[it] == 'h')
	 			{
	 				it++;
	 				x = (char) va_arg(args, int);
	 			}
	 			else
	 				x = (short) va_arg(args, int);
	 		}
	 		else
	 		{
	 			if (format[it] == 'l')
	 			{
	 				was = 1;
	 				it++;
	 				if (format[it] == 'l')
	 				{
	 				 	it++;
	 				 	x = va_arg(args, long long);
	 				}
	 				else
	 					x = va_arg(args, long);
	 			}
	 			else if (format[it] == 'z')
	 			{
	 				was = 1;
	 			 	it++;
	 			 	x = va_arg(args, size_t);
	 			}
	 		}
	 		if (format[it] == 's')
	 		{
	 		    it++;
	 		    was = 1;
	 			char *t = va_arg(args, char*);
	 			int len = strlen(t);
	 			for (int i = 0; i < len; i++)
	 				print_symb(&s, t[i], &print_length, n);
	 			continue;	
	 		}
	 		if (format[it] == 'p')
	 		{
	 			it++;
	 			was = 1;
	 			uint64_t p = (uint64_t) va_arg(args, void*);
	 			print_dec_unsigned(&s, p, &print_length, 16, 16, n);
	 			continue;
	 		}
	 		if (format[it] == 'u')
	 		{
	 			it++;
	 			if (was)
	 				print_dec_unsigned(&s, x, &print_length, 10, 0, n);
	 			else
	 			{
	 				was = 1;
		 			unsigned int p = va_arg(args, unsigned int);
		 			print_dec_unsigned(&s, p, &print_length, 10, 0, n);		
	 			}
	 			continue;
	 		}
	 		if (!was)
	 			x = va_arg(args, int);
	 		if (format[it] == 'd' || format[it] == 'i')
	 		{
	 			it++;
	 			print_number(&s, x, &print_length, 10, 0, n);
	 			continue;
	 		}
	 		if (format[it] == 'x')
	 		{
	 			it++;
	 			print_number(&s, x, &print_length, 16, 0, n);
	 			continue;
	 		}	 		 	
	 		if (format[it] == 'o')
	 		{
	 			it++;
	 			print_number(&s, x, &print_length, 8, 0, n);
	 			continue;
	 		}	 		 	
	 		if (format[it] == 'c')
	 		{
	 			it++;
	 			print_symb(&s, x, &print_length, n);
	 			continue;
	 		}	 		 	
	 	}
	 	else
	 	{
	 		print_symb(&s, format[it], &print_length, n);
	 		it++;
		}
	}
	return print_length;
}

void print_test()
{
	int k = 105;
	printf("%d aba %i %u %o %x %c %s %p %lld %hd\n", 105, 205, -342, 10, 123123, 98344, "kkookk", &k, (1ll << 62), (1ll << 31));
 	char buffer[12];
 	int l = snprintf(buffer, 10, "%d %d %d %d\n", 1456, 1235, 1235, 2);
	printf("%d\n", l);
}

