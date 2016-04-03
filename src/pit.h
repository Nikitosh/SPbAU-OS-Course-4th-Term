#ifndef __PIT_H__
#define __PIT_H__

#define PIT_CONTROL_PORT 0x43
#define PIT_DATA_PORT 0x40

#define RATE_GENERATOR (1 << 2)
#define DATA_FLAG ((1 << 4) | (1 << 5))

#define FREQUENCY_DIVIDER (bit(12))
#define FREQUENCY 1193180

void init_pit();
void pit_handler_w();
void pit_handler();

#endif /* __PIT_H__ */
