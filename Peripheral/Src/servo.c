#include "servo.h"

#define SERVO_MIN    50   // 0°
#define SERVO_MID   130   // 90°
#define SERVO_MAX   250   // 180°

extern TIM_HandleTypeDef htim2;

void servo_init()
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, SERVO_MIN); // 初始设为0°
}

void servo_open()
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, SERVO_MID); // 90°
}

void servo_close()
{
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, SERVO_MIN); // 0°
}