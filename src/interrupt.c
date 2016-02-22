#include "interrupt.h"
#include "ioport.h"
#include "memory.h"
#include "uart.h"

void init_interrupt_controller()
{
	out8(MASTER_COMMAND_REGISTER, FIRST_WORD);
	out8(MASTER_DATA_REGISTER, MASTER_START_NUMBER);
	out8(MASTER_DATA_REGISTER, bit(SLAVE_LINE_NUMBER));
	out8(MASTER_DATA_REGISTER, CONTROLLER_MODE);
	
	out8(SLAVE_COMMAND_REGISTER, FIRST_WORD);
	out8(SLAVE_DATA_REGISTER, SLAVE_START_NUMBER);
	out8(SLAVE_DATA_REGISTER, SLAVE_LINE_NUMBER);
	out8(SLAVE_DATA_REGISTER, CONTROLLER_MODE);
}

void init_idt()
{
	idt.base = (uint64_t) &descriptors;
	idt.size = sizeof(descriptors) - 1;
	for (int i = 0; i < SIZE; i++)
		set_interrupt_descriptor(i, (uint64_t) &empty_handler_w, SEGMENT_PRESENT_FLAG | INTERRUPT_TYPE);
    set_idt(&idt);
}

void set_interrupt_descriptor(uint8_t id, uint64_t offset, uint8_t flags)
{
	descriptors[id].offset_most = get_bits(offset, 32, 64);
	descriptors[id].offset_medium = get_bits(offset, 16, 32);
	descriptors[id].offset_least = get_bits(offset, 0, 16);
	descriptors[id].flags = flags;
	descriptors[id].segment_selector = KERNEL_CODE;
	descriptors[id].interrupt_stack_table = 0;
	descriptors[id].reserved = 0;
}

void send_eoi(uint8_t is_master)
{
	if (!is_master)
		out8(SLAVE_COMMAND_REGISTER, EOI);
	out8(MASTER_COMMAND_REGISTER, EOI);
}
