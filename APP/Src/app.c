#include "freertos.h"
#include "task.h"
#include "app.h"
#include "string.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "led.h"
#include "stdio.h"
#include "stdlib.h"
#include "servo.h"
#include "infrared.h"
#include "queue.h"
#include "semphr.h"
#include "flash_storage.h"
#include "RingBuffer.h"
#include "vehicle_info_update.h"
#include "interface_uart.h"
#include "interface_gpio.h"
#include "light_sensor.h"


extern UART_HandleTypeDef huart1;

QueueHandle_t xInfraredQueue;
SemaphoreHandle_t xVehicleSemaphore;
TaskHandle_t xInfraredTask = NULL;
volatile uint8_t vehicle_flag = 1;

uint8_t g_plate_number_buf[32];


int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffffff);
	return ch;
}


/**
 * @brief UART 接收任务
 * @details 循环读取 UART 接收缓冲区数据，解析后存储到全局车牌号缓冲区
 * @param arg 任务参数，未使用
 * @note 该函数为 FreeRTOS 任务，不应直接调用
 * @note 使用全局变量 g_plate_number_buf 存储接收到的车牌号数据
 */
void uart_task(void *arg)
{
    uart_rb_init();
    while (1)
    {
        uart_get_data(g_plate_number_buf);
        vTaskDelay(10);
    }
}

void led_key_task()
{
    while (1)
    {
        Process_System_Reset_Button();
        light_sensor_start();
        vTaskDelay(100);
    }
}
/**
 * @brief 舵机控制任务
 *
 * 根据红外传感器状态控制舵机开关，检测到车辆状态变化时控制道闸的开启和关闭
 * 通过队列接收红外传感器传来的车辆状态信息，根据车辆状态控制道闸和 LED 指示灯。
 *
 * @details
 * - 初始化舵机
 * - 循环等待红外传感器状态队列消息
 * - 当接收到无车辆状态时，关闭道闸，点亮红色 LED，熄灭绿色和黄色 LED，设置车辆标志位
 * - 当接收到状态 2 或有车辆状态时，打开道闸
 * - 每次操作后延时 90ms
 *
 * @note 该任务使用 FreeRTOS 队列机制进行消息等待，等待时间为 portMAX_DELAY
 * @note 道闸控制逻辑：无车时关闭，有车时开启
 */
void servo_task()
{
    uint8_t rx_infrared_state;
    servo_init();
    while (1)
    
        if (xQueueReceive(xInfraredQueue, &rx_infrared_state, portMAX_DELAY) == pdTRUE)
        {
            if (rx_infrared_state == NO_VEHICLE)
            {
                servo_close();
                Red_LED_On();
                Green_LED_Off();
                //Yellow_LED_Off();
                vehicle_flag = 1;
            }
            else if (rx_infrared_state == HAVE_VEHICLE)
            {
                servo_open();
            }
            else
            {
                servo_open();
            }
            vTaskDelay(90);
        }
}

void infrared_task()
{
    static uint8_t last_infrared_state = HAVE_VEHICLE;

    while (1)
    {
        if (xSemaphoreTake(xVehicleSemaphore, 0) == pdTRUE)
        {
            uint8_t cmd = HAVE_VEHICLE; // HAVE_VEHICLE 代表有车状态
            xQueueSend(xInfraredQueue, &cmd, 0);
            vTaskDelay(5000); // 
            while(Read_Infrared_State(CGQ2) == HAVE_VEHICLE)
            {
                vTaskDelay(50);
            }
            last_infrared_state = NO_VEHICLE; // 更新上一次状态
        }
        uint8_t current_infrared_state = Read_Infrared_State(CGQ1);
        if(current_infrared_state == NO_VEHICLE && last_infrared_state == HAVE_VEHICLE)
        {
            uint8_t cmd = NO_VEHICLE;
            xQueueSend(xInfraredQueue, &cmd, 0);
        }
        last_infrared_state = current_infrared_state; // 更新上一次状态
        // xQueueSend(xInfraredQueue, &infrared_state, 0);
        vTaskDelay(100); 
    }
}
void oled_task()
{
    static int current_balance = 0;
    static uint8_t Empty_place = 50;
    int vehicle_idx = -1;

    flash_storage_init();

    OLED_Init();
    OLED_Clear();

    OLED_ShowChinese(32, 0, "车库系统");
    OLED_ShowChinese(0, 16, "余额：");
    OLED_ShowChinese(70, 16, "空位：");
    OLED_ShowChinese(0, 33, "车牌号：");

    OLED_ShowNum(40, 16, 0, 3, OLED_8X16);            // 显示余额
    OLED_ShowNum(110, 16, Empty_place, 2, OLED_8X16); // 显示空位
    OLED_Update();

    // erase_vehicle_data_in_flash();

    char last_plate[16] = {0};        // 记录上一次的车牌号
    TickType_t last_process_time = 0; // 记录上一次处理时间
    uint32_t yellow_time = 0;
    while (1)
    {
        if (Serial_flag)
        {
            Serial_flag = 0;

            char *current_plate = (char *)g_plate_number_buf;

            if(strcmp(current_plate, "OPEN") == 0)
            {
                servo_open();
                Green_LED_On();
                Red_LED_Off();
                Yellow_LED_Off();
                vTaskSuspend(xInfraredTask); // 暂停红外任务，避免干扰
                continue;
            }
            if(strcmp(current_plate, "CLOSE") == 0)
            {
                servo_close();
                Red_LED_On();
                Green_LED_Off();
                Yellow_LED_Off();
                vTaskResume(xInfraredTask); // 恢复红外任务
                continue;
            }
            // ====防止重复处理====
            TickType_t current_time = xTaskGetTickCount();
            if (strcmp(current_plate, last_plate) == 0 && (current_time - last_process_time) < pdMS_TO_TICKS(5000))
            {
                // 如果两次识别的车牌相同且间隔时间小于 5 秒，则不处理
                continue;
            }
            // ====防止重复处理结束====
            char *cz_ptr = strstr(current_plate, "cz");

            int16_t vehicle_idx = -1;
            uint16_t add_money = 0;

            //==============充值处理 1：充值值指针（包含"cz"）===============
            if (cz_ptr != NULL)
            {
                // 获取充值金额值，格式为"CZ100"代表充值 100 元
                add_money = atoi(cz_ptr + 2); // 跳过"cz"后面的字符
                strcpy(last_plate, current_plate);
                last_process_time = xTaskGetTickCount();
                // 2. 获取车牌号
                // 将'c'位置强制改为'\0'，使得 recv_str 只剩下前面的车牌号
                *cz_ptr = '\0';
                // 3.添加或更新记录
                vehicle_idx = Find_Vehicle(current_plate);
                if (vehicle_idx == -1)
                {
                    vehicle_idx = Add_Vehicle(current_plate); // 如果不存在则添加
                }
                // 4. 充值金额
                if (vehicle_idx != -1)
                {
                    g_vehicle_db[vehicle_idx].balance += add_money;
                    // 显示充值成功的消息
                    OLED_ShowString(0, 48, "Recharge Success", OLED_8X16);
                }
                sync_all_vehicles_to_flash();
            }
            // ================== 扣费开门处理 2：识别车牌号指令 (不包含"cz") ==================
            else
            {
                vehicle_idx = Find_Vehicle(current_plate);

                // 自动添加记录
                if (vehicle_idx == -1)
                {
                    vehicle_idx = Add_Vehicle(current_plate);
                }

                // 放行车辆
                if (g_vehicle_db[vehicle_idx].balance > 0 && vehicle_flag == 1)
                {
                    vehicle_flag = 0;
                    // 放行车辆
                    if (Empty_place > 0)
                        Empty_place--;
                    g_vehicle_db[vehicle_idx].balance -= 10; // 扣 10 元停车费

                    strcpy(last_plate, current_plate);       // 记录上一次的车牌号
                    last_process_time = xTaskGetTickCount(); // 记录上一次的时间
                    uint8_t cmd = 2;
                    xQueueSend(xInfraredQueue, &cmd, 0);
                    Green_LED_On();
                    Red_LED_Off();
                    //Yellow_LED_Off();
                    OLED_ShowString(0, 48, "Access Granted  ", OLED_8X16);
                    xSemaphoreGive(xVehicleSemaphore);
                }
                else if (vehicle_flag == 0)
                {
                    continue;
                }
                else
                {
                    // 余额不足
                    Green_LED_Off();
                    Red_LED_Off();
                    Yellow_LED_On(); // 闪烁
                    yellow_time = xTaskGetTickCount();
                    OLED_ShowString(0, 48, "Need Recharge!  ", OLED_8X16);
                }
                sync_all_vehicles_to_flash();
            }

            // ================== 统计刷新显示 ==================
            if (vehicle_idx != -1)
            {
                OLED_ShowNum(40, 18, g_vehicle_db[vehicle_idx].balance, 3, OLED_8X16); // 显示车辆余额
                OLED_ShowNum(110, 18, Empty_place, 2, OLED_8X16);
                OLED_ShowString(60, 33, "                ", OLED_8X16);                  // 清除车牌号
                OLED_ShowString(60, 33, g_vehicle_db[vehicle_idx].plate_num, OLED_8X16); // 显示车牌号
                OLED_Update();
            }
        }


        if (yellow_time != 0 && (xTaskGetTickCount() - yellow_time) > pdMS_TO_TICKS(5000))
        {
            Yellow_LED_Off();
            yellow_time = 0;
        }
        vTaskDelay(10);
    }
}

// void flash_storage_task()
// {
//     flash_storage_init();
//     while(1)
//     {
//         if(g_flash_write_addr >= FLASH_STORAGE_END_ADDR)
//         {
//             erase_vehicle_data_in_flash();
//         }
//         vTaskDelay(100); // 每 10 秒保存一次数据到 Flash
//     }
// }

/* 应用程序入口启动初始化 */
void app_init(void)
{
    xInfraredQueue = xQueueCreate(10, sizeof(uint8_t));
    xVehicleSemaphore = xSemaphoreCreateBinary();
		
    // HAL_UART_Transmit(&huart1, (uint8_t *)"hello\r\n", 7, 0xFFFF);
    // // printf("flash init start\r\n");
    // printf("flash_vehicles is erased\r\n");

    xTaskCreate(led_key_task, "led_key_task", 128, NULL, 1, NULL);
    xTaskCreate(uart_task, "uart_task", 128, NULL, 3, NULL);
    xTaskCreate(oled_task, "oled_task", 512, NULL, 1, NULL);
    xTaskCreate(servo_task, "servo_task", 128, NULL, 2, NULL);
    xTaskCreate(infrared_task, "infrared_task", 128, NULL, 1, &xInfraredTask);
    //xTaskCreate(flash_storage_task, "flash_storage_task", 128, NULL, 1, NULL);
    vTaskStartScheduler();
}