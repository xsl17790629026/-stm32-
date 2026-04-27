// #include "interface_bule.h"

// #include "RingBuffer.h"
// #include "stdio.h"
// #include "string.h"

// extern UART_HandleTypeDef huart1;

// volatile uint8_t rx_data;
// uint8_t Bule_flag = 0;


// ring_buf_t bule_rb = {
//     .buf = {0},
//     .write = 0,
//     .read = 0
// };

// void bule_rb_init(void)
// {
//     HAL_UART_Receive_IT(&huart1, &rx_data, 1);  // 开启UART中断接收
// }

//     static state_t state = WAIT_HEAD1;
//     uint8_t data;
//     uint8_t data_buf[32];
//     uint8_t data_count = 0;

// void bule_get_data(uint8_t *g_plate_number_buf)
// {

//     if (ring_read(&bule_rb, &data))
//     {
//         switch (state)
//         {
//             case WAIT_HEAD1:
//                 if (data == BULE_HEAD1)
//                 {
//                     state = WAIT_HEAD2;
//                 }		
//                 break;

//             case WAIT_HEAD2:
//                 if (data == BULE_HEAD2) 
//                 {
//                     data_count = 0;
//                     state = WAIT_DATA;
//                 } 
//                 else 
//                 {
//                     state = (data == BULE_HEAD1) ? WAIT_HEAD2 : WAIT_HEAD1;
//                 }
//                 break;

//             case WAIT_DATA:
//                 if (data == BULE_TAIL)
//                 { 
                    
//                     printf("Received data: %s\r\n", data_buf);
//                     // HAL_UART_Transmit(&huart1, data_buf, data_count, 100);
//                     data_buf[data_count] = '\0';

//                     memcpy(g_plate_number_buf, data_buf, data_count + 1); // +1 用于拷贝字符串结束符 '\0'
//                     Bule_flag = 1;
										
//                     state = WAIT_HEAD1;
//                 }
//                 // 如果没有接收到结束符，则继续接收数据
//                 else
//                 {
//                     if (data_count < 31) // 防止缓冲区溢出
//                     {
//                         data_buf[data_count++] = data;
//                     }
//                     else
//                     {
//                         // 数据过长，丢弃当前帧，重新等待帧头
//                         state = WAIT_HEAD1;
//                         data_count = 0;
//                     }
//                 }
//                 break;
//         }
//     }
// }


// /* 串口接收完成中断回调函数 */
// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//     if (huart == &huart1) {
//         ring_write(&bule_rb, rx_data);
//         HAL_UART_Receive_IT(&huart1, &rx_data, 1);  // 重新开启下一次接收中断
//     }
// }