#include "uart.h"
#include "interrupt.h"
#include "pit.h"
#include "print.h"

void main(void)
{ 
	init_uart();
	init_interrupt_controller();
	init_idt();
	init_pit();
 	print_test();
	sti();
	while (1); 
}
