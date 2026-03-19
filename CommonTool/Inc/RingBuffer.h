#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "main.h"
#define UART_BUF_SIZE 128

typedef struct {
    uint8_t buf[UART_BUF_SIZE];
    uint16_t write;
    uint16_t read;
} ring_buf_t;

/* 写入函数 */
void ring_write(ring_buf_t *rb, uint8_t data);

/* 读取函数 */
int ring_read(ring_buf_t *rb, uint8_t *data);

#endif /* __RINGBUFFER_H__ */