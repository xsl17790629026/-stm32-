#include "servo.h"

#define SERVO_MIN    50   // 0°
#define SERVO_MID   130   // 90°
#define SERVO_MAX   250   // 180°

extern TIM_HandleTypeDef htim2;

// 增加一个变量来记忆舵机当前的位置
static volatile int current_duty = SERVO_MIN; 

void servo_init()
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, SERVO_MIN); 
    current_duty = SERVO_MIN; // 初始化记忆
}

void servo_open()
{
    // 防御性编程：如果已经处于开门位置或更大，直接退出，不执行循环
    if (current_duty >= SERVO_MID) {
        return; 
    }

    // 从【当前位置】开始转，而不是死板地从 SERVO_MIN 开始
    for(int duty = current_duty; duty <= SERVO_MID; duty += 5)
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
        vTaskDelay(20);
        current_duty = duty; // 实时更新当前位置记忆
    }
}

void servo_close()
{
    // 防御性编程：如果已经处于关门位置或更小，直接退出
    if (current_duty <= SERVO_MIN) {
        return; 
    }

    // 从【当前位置】开始转
    for(int duty = current_duty; duty >= SERVO_MIN; duty -= 5)
    {
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty);
        vTaskDelay(20);
        current_duty = duty; // 实时更新当前位置记忆
    }
}