#include "uart.h"
#include "interrupt.h"
#include "pit.h"
#include "print.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"
#include "slab_allocator.h"
#include "test.h"

void main(void)
{ 
	init_uart();
	init_interrupt_controller();
	init_idt();
	
	get_memory_map();
	print_memory_map();
	init_buddy();
	init_paging();
	init_slab_allocator();

	test_all();

	while (1); 
}
