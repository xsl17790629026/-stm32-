#include "infrared.h"

/**
 * @brief 读取红外传感器的状态
 * 
 * 该函数用于读取指定红外传感器的当前状态，判断是否有车辆检测到。
 * 
 * @param type 红外传感器类型，可选值为：
 *             - CGQ1: 第一个红外传感器（读取CGQ3引脚）
 *             - CGQ2: 第二个红外传感器（读取CGQ2引脚）
 * 
 * @return uint8_t 传感器状态值：
 *         - 非0值：检测到车辆
 *         - 0：无车辆或无效参数
 * 
 * @note 当传入的type参数既不是CGQ1也不是CGQ2时，默认返回0（无车状态）
 */
uint8_t Read_Infrared_State(CGQ_TYPE type)
{
    if (type == CGQ1) {
        return HAL_GPIO_ReadPin(CGQ3_GPIO_Port, CGQ3_Pin);
    } else if (type == CGQ2) {
        return HAL_GPIO_ReadPin(CGQ2_GPIO_Port, CGQ2_Pin);
    }
    return 0; // 默认返回无车状态
}