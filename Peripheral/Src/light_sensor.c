#include "light_sensor.h"
#include "LED.h"

void light_sensor_start()
{
    if(HAL_GPIO_ReadPin(GM_GPIO_Port, GM_Pin)==GPIO_PIN_SET)
    {
        light_Off();
    }   
    else
    {
        light_On();
    }
}