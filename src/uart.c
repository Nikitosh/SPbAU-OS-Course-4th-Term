#include "uart.h"
#include "ioport.h"

void init_uart()
{
	out8(LINE_CONTROL_REGISTER, EIGHT_BIT_MODE);
	out8(INTERRUPT_ENABLE_REGISTER, INTERRUPTS_DISABLED);
}

void putc(char c)
{
	while (!get_bit(in8(LINE_STATUS_REGISTER), CAN_WRITE_BIT))
		continue;
	out8(TRANSMITTER_HOLDING_BUFFER, c);
}

void puts(char *s)
{
	while (*s != 0)
	{
		putc(*s);
		s++;
	}
}
