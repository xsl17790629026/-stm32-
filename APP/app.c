#include "freertos.h"
#include "task.h"
#include "app.h"
#include "string.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "led.h"
#include "stdio.h"
#include "stdlib.h"

#define UART_BUF_SIZE 128
#define FRAME_HEAD1 '#'
#define FRAME_HEAD2 '@'
#define FRAME_TAIL  '*'
#define MAX_VEHICLES 50

extern UART_HandleTypeDef huart1;
uint8_t rx_data;

int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffffff);
	
	return ch;
}

typedef struct{
    char plate_num[16];
    uint16_t balance;
}VehicleInfo_t;

VehicleInfo_t g_vehicle_db[MAX_VEHICLES];
int g_vehicle_count = 0;

typedef struct {
    uint8_t buf[UART_BUF_SIZE];
    uint16_t write;
    uint16_t read;
} ring_buf_t;

ring_buf_t uart_rb = {0};
uint8_t Serial_flag = 0;
uint8_t g_plate_number_buf[32];

// 通过车牌号查找车辆信息，返回索引，未找到返回-1
int Find_Vehicle(const char *plate)
{
    for (int i = 0; i < g_vehicle_count; i++) {
        if (strcmp(g_vehicle_db[i].plate_num, plate) == 0) {
            return i; // 找到了
        }
    }
    return -1; // 没有找到
}

//  添加新车辆信息, 返回索引，数据库满返回-1
int Add_Vehicle(const char *plate)
{
    if (g_vehicle_count >= MAX_VEHICLES) {
        return -1; // 数据库满
    }
    // 复制车牌
    strncpy(g_vehicle_db[g_vehicle_count].plate_num, plate, sizeof(g_vehicle_db[g_vehicle_count].plate_num) - 1);
    g_vehicle_db[g_vehicle_count].plate_num[sizeof(g_vehicle_db[g_vehicle_count].plate_num) - 1] = '\0'; // 确保字符串以null结尾
    // 新车默认余额为0
    g_vehicle_db[g_vehicle_count].balance = 0; // 初始化余额

    // // 如果是测试用的特权车牌，可以初始化送点钱方便测试
    // if(strcmp(plate, "B88888") == 0) {
    //      g_vehicle_db[g_vehicle_count].balance = 100;
    // }

    g_vehicle_count++;
    return g_vehicle_count - 1;
}

/* 写入函数 */
static inline void ring_write(ring_buf_t *rb, uint8_t data)
{
    uint16_t next = (rb->write + 1) % UART_BUF_SIZE;
    
    if (next != rb->read) {
        rb->buf[rb->write] = data;
        rb->write = next;
    }
}

/* 读取函数 */
static inline int ring_read(ring_buf_t *rb, uint8_t *data)
{
    if (rb->write == rb->read)
        return 0; // 缓冲区空
    *data = rb->buf[rb->read];
    rb->read = (rb->read + 1) % UART_BUF_SIZE;
    return 1;
}

/* 接收中断回调函数，接收数据放入环形缓冲区 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        ring_write(&uart_rb, rx_data);
        HAL_UART_Receive_IT(&huart1, &rx_data, 1); // 重新开启接收中断
    }
}

/* UART接收处理任务 */
void uart_task(void *arg)
{
    state_t state = WAIT_HEAD1;
    uint8_t data;
    uint8_t data_buf[32];
    uint8_t data_count = 0;

    HAL_UART_Receive_IT(&huart1, &rx_data, 1); // 重新开启接收中断

    while (1)
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
                        memcpy(g_plate_number_buf, data_buf, data_count + 1); // +1 复制空字符
                        Serial_flag = 1;
											
                        state = WAIT_HEAD1;
                    }

                    else
                        data_buf[data_count++] = data;
                    break;

            }
        }

        vTaskDelay(1);
    }
}

void led_task()
{
	while(1)
	{
		HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
		vTaskDelay(500);
	}
}
void oled_task()
{
	static int current_balance = 0;
	static uint8_t Empty_place =50;
  int vehicle_idx = -1;

	OLED_Init();
	OLED_Clear();

    // 初始化界面
	OLED_ShowChinese(32, 0, "车库系统");
	OLED_ShowChinese(0, 16, "余额：");
	OLED_ShowChinese(70, 16, "空位：");
	OLED_ShowChinese(0, 33, "车牌号：");

	OLED_ShowNum(40, 16, 0, 3, OLED_8X16); //余额
	OLED_ShowNum(110, 16, Empty_place, 2, OLED_8X16); //空位
	OLED_Update();

	while(1)
	{
       if(Serial_flag)
       {
            Serial_flag = 0;
            char *current_plate = (char *)g_plate_number_buf;
            char *cz_ptr = strstr(current_plate, "cz");

            int16_t vehicle_idx = -1;
            uint16_t add_money = 0;

            //==============分支1：充值指令（包含"cz"）===============
            if (cz_ptr != NULL)
            {
                // 提取充值金额，假设格式为 "CZ100" 表示充值100单位
                add_money = atoi(cz_ptr + 2); // 跳过 "cz" 两个字符

                // 2. 提取车牌
                // 将 'c' 的位置强制改为 '\0'，这样 recv_str 就只剩前半段车牌了
                *cz_ptr = '\0';
                // 3.查找或注册车牌
                vehicle_idx = Find_Vehicle(current_plate);
                if (vehicle_idx == -1) 
                {
                    vehicle_idx = Add_Vehicle(current_plate); // 如果是新车充值，先注册
                }
                // 4. 充值
                if (vehicle_idx != -1) 
                {
                    g_vehicle_db[vehicle_idx].balance += add_money;
                    // 显示充值成功信息
										OLED_ShowString(0, 48, "                ", OLED_8X16); // 清除旧指令
                    OLED_ShowString(0, 48, "Recharge Success", OLED_8X16);
                    // 充值后如果余额 > 0，顺便把灯变绿 (可选)
                    // if(g_vehicle_db[vehicle_idx].balance > 0) 
                    // {
                    //     Green_LED_On();
                    //     Red_LED_Off();
                    //     Yellow_LED_Off();
                    //     g_vehicle_db[vehicle_idx].balance -= 10; // 充值后自动扣10单位进门费
                    // }
                }    
            }
            // ================== 分支2：正常识别指令 (不含 "cz") ==================
            else 
            {
                vehicle_idx = Find_Vehicle(current_plate);
                
                // 新车自动注册
                if (vehicle_idx == -1) {
                    vehicle_idx = Add_Vehicle(current_plate);
                }

                // 检查余额
                if(g_vehicle_db[vehicle_idx].balance > 0) 
                {
                    // 正常进门
                    if(Empty_place > 0) Empty_place--;
                    g_vehicle_db[vehicle_idx].balance -= 10; // 扣10单位进门费
                    Green_LED_On();
                    Red_LED_Off();
                    Yellow_LED_Off();
										OLED_ShowString(0, 48, "                ", OLED_8X16); // 清除旧指令
                    OLED_ShowString(0, 48, "Access Granted ", OLED_8X16);	
                }
                else 
                {
                    // 余额不足
                    Green_LED_Off();
                    Red_LED_Off(); 
                    Yellow_LED_On(); // 亮黄灯
										OLED_ShowString(0, 48, "                ", OLED_8X16); // 清除旧指令
										OLED_ShowString(0, 48, "Need Recharge! ", OLED_8X16);
                }
            }

            // ================== 统一刷新显示 ==================
            if(vehicle_idx != -1)
            {
                OLED_ShowNum(40, 18, g_vehicle_db[vehicle_idx].balance, 3, OLED_8X16); // 显示最新余额
                OLED_ShowNum(110, 18, Empty_place, 2, OLED_8X16);
                OLED_ShowString(60, 33, "                ", OLED_8X16); // 清除旧车牌
                OLED_ShowString(60, 33, g_vehicle_db[vehicle_idx].plate_num, OLED_8X16); // 显示车牌
                OLED_Update();
            }   
       } 
       vTaskDelay(10);
	}
}
/* 应用初始化函数 */
void app_init(void)
{
// // 可以在这里预存一些车牌数据，方便测试
//     Add_Vehicle("TEST01"); // 余额默认0
    
//     // 手动给特权车加钱
//     int idx = Add_Vehicle("B88888"); 
//     g_vehicle_db[idx].balance = 500; // 初始余额500

    xTaskCreate(uart_task, "uart_task", 256, NULL, 2, NULL);
    xTaskCreate(led_task, "led_task", 256, NULL, 1, NULL);
    xTaskCreate(oled_task, "oled_task", 256, NULL, 1, NULL);
    vTaskStartScheduler();
}
