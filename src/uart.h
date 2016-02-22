#ifndef __UART_H__
#define __UART_H__

#include <stdint.h>
#include "ioport.h"

#define TRANSMITTER_HOLDING_BUFFER 0x3f8 + 0
#define LINE_CONTROL_REGISTER 0x3f8 + 3
#define LINE_STATUS_REGISTER 0x3f8 + 5

#define EIGHT_BIT_MODE (bit(0) + bit(1))
#define CAN_WRITE_BIT 5

void init_uart();

void putc(char c);

void puts(char *s);

#endif /* __UART_H__ */
