#include "uart.h"
#include "interrupt.h"
#include "pit.h"

void main(void)
{ 
	init_uart();
	init_interrupt_controller();
	init_idt();
	init_pit();
	sti();
	while (1); 
}
