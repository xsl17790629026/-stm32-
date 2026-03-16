#include "RingBuffer.h"

void ring_write(ring_buf_t *rb, uint8_t data)  
{
    uint16_t next = (rb->write + 1) % UART_BUF_SIZE;
    
    if (next != rb->read) {
        rb->buf[rb->write] = data;
        rb->write = next;
    }
}

int ring_read(ring_buf_t *rb, uint8_t *data)  
{
    if (rb->write == rb->read)
    {
        return 0; 
    }   
    *data = rb->buf[rb->read];
    rb->read = (rb->read + 1) % UART_BUF_SIZE;
    return 1;
}