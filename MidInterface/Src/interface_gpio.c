#include "interface_gpio.h"
#include "vehicle_info_update.h"

void Process_System_Reset_Button(void)
{
    static uint32_t press_tick = 0;
    static uint8_t is_pressing = 0;

    // 1. 检测按键是否按下
    if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET) 
    {
        if (!is_pressing) 
        {
            is_pressing = 1;
            press_tick = HAL_GetTick(); // 记录按下开始时间
        }
        
        // 2. 判断按下时长是否超过 3 秒
        if (is_pressing && (HAL_GetTick() - press_tick > 3000)) 
        {
            // --- 执行清除动作 ---
            Clear_All_Vehicle_Data();
            
            // 等待按键松开，防止重复触发
            while(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET);
            is_pressing = 0;
        }
    }
    else 
    {
        is_pressing = 0; // 按键松开，重置状态
    }
}