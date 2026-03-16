#include "interface_uart.h"
#include "RingBuffer.h"
#include "stdio.h"
#include "string.h"

extern UART_HandleTypeDef huart1;

uint8_t rx_data;
uint8_t Serial_flag = 0;

int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffffff);
	return ch;
}

ring_buf_t uart_rb = {0};

void uart_rb_init(void)
{
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);  // 在初始化时使能 UART 中断接收
}

    state_t state = WAIT_HEAD1;
    uint8_t data;
    uint8_t data_buf[32];
    uint8_t data_count = 0;

void uart_get_data(uint8_t *g_plate_number_buf)
{
    if (ring_read(&uart_rb, &data))
        {
            switch (state)
            {
                case WAIT_HEAD1:
                    if (data == FRAME_HEAD1)
                    {
                        state = WAIT_HEAD2;
                    }		
                    break;

                case WAIT_HEAD2:
                    if (data == FRAME_HEAD2) 
                    {
                        data_count = 0;
                        state = WAIT_DATA;
                    } else 
                    {
                        state = (data == FRAME_HEAD1) ? WAIT_HEAD2 : WAIT_HEAD1;
                    }
                    break;

                case WAIT_DATA:
                    if (data == FRAME_TAIL)
                    { 
                        HAL_UART_Transmit(&huart1, data_buf, data_count, 100);
                        data_buf[data_count]='\0';
                        memcpy(g_plate_number_buf, data_buf, data_count + 1); // +1 包含字符串结束符
                        Serial_flag = 1;
											
                        state = WAIT_HEAD1;
                    }
                    // 在 uart_task 中处理 WAIT_DATA 状态
                    else
                    {
                        if(data_count < 31) // 防止数组越界
                        {
                            data_buf[data_count++] = data;
                        }
                        else
                        {
                            // 数据溢出，重置状态
                            state = WAIT_HEAD1;
                            data_count = 0;
                        }
                    }

                    break;

            }
        }
}


/* 串口接收完成回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1) {
        ring_write(&uart_rb, rx_data);
        HAL_UART_Receive_IT(&huart1, &rx_data, 1);  // 重新使能中断接收
    }
}