#include "pit.h"
#include "interrupt.h"
#include "ioport.h"
#include "uart.h"
#include "utilities.h"
#include "threads.h"

void init_pit()
{
    out8(PIT_CONTROL_PORT, RATE_GENERATOR | DATA_FLAG);
    out8(PIT_DATA_PORT, get_bits(FREQUENCY_DIVIDER, 0, 8));
    out8(PIT_DATA_PORT, get_bits(FREQUENCY_DIVIDER, 8, 16));
    set_interrupt_descriptor(MASTER_START_NUMBER, (uint64_t) &pit_handler_w, SEGMENT_PRESENT_FLAG | INTERRUPT_TYPE);
}

void pit_handler()
{
    send_eoi(1);
    thread_schedule((double) FREQUENCY_DIVIDER / FREQUENCY);
}
