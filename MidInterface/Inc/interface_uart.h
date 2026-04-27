#ifndef __INTERFACE_UART_H__
#define __INTERFACE_UART_H__

#include "main.h"

extern uint8_t Serial_flag;

#define FRAME_HEAD1 '#'
#define FRAME_HEAD2 '@'
#define FRAME_TAIL  '*'


typedef enum {
    WAIT_HEAD1,
    WAIT_HEAD2,
    WAIT_DATA,
} state_t;

void uart_rb_init(void);
void uart_get_data(uint8_t *g_plate_number_buf);

#endif /* __INTERFACE_UART_H__ */