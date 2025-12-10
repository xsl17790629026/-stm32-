#include "infrared.h"

uint8_t Read_Infrared_State()
{
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
}