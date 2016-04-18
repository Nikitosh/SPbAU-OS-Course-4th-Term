#include "uart.h"
#include "interrupt.h"
#include "pit.h"
#include "print.h"
#include "memory_map.h"
#include "buddy_allocator.h"
#include "paging.h"
#include "slab_allocator.h"
#include "test.h"
#include "threads.h"
#include "lock.h"
#include "paging.h"
#include "file_system.h"
#include "initramfs.h"

void main(void)
{ 
    init_uart();
    init_interrupt_controller();
    init_idt();
    
    get_memory_map();
    init_initramfs();
    init_buddy();
    init_paging();
    init_slab_allocator();
    
    init_file_system();
    load_initramfs_module();
    print_file_system();
	
    init_pit(); 
    init_threads();
    sti();
    
    test_all();
    
    while (1);
}
