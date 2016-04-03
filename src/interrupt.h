#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <stdint.h>
#include "ioport.h"

#define MASTER_COMMAND_REGISTER 0x20
#define MASTER_DATA_REGISTER 0x21
#define SLAVE_COMMAND_REGISTER 0xA0
#define SLAVE_DATA_REGISTER 0xA1

#define MASTER_START_NUMBER 0x20
#define SLAVE_START_NUMBER 0x28

#define FIRST_WORD (bit(0) + bit(4))
#define SLAVE_LINE_NUMBER 2
#define CONTROLLER_MODE 1

#define FULL_MASK (bit(8) - 1)

#define SIZE 256

#define SEGMENT_PRESENT_FLAG bit(7)
#define INTERRUPT_TYPE 14

#define EOI bit(5)

struct idt_ptr 
{
    uint16_t size;
    uint64_t base;
} __attribute__((packed));

struct interrupt_descriptor
{
    uint16_t offset_least;
    uint16_t segment_selector;
    uint8_t interrupt_stack_table;
    uint8_t flags;
    uint16_t offset_medium;
    uint32_t offset_most;
    uint32_t reserved;
} __attribute__((packed));

static struct interrupt_descriptor descriptors[SIZE];
static struct idt_ptr idt;

static inline void set_idt(const struct idt_ptr *ptr)
{ __asm__ volatile ("lidt (%0)" : : "a"(ptr)); }

static inline void sti()
{ __asm__ volatile ("sti"); }

static inline void cli()
{ __asm__ volatile ("cli"); }

void init_interrupt_controller();

void init_idt();

void empty_handler_w();
void pop_handler_w();

void set_interrupt_descriptor(uint8_t id, uint64_t offset, uint8_t flags); 

void send_eoi(uint8_t is_master);

#endif /*__INTERRUPT_H__*/
