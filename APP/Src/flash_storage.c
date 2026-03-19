#include "flash_storage.h"
#include "string.h"
#include "usart.h"
#include "stdio.h"

// 全局变量，替换 app.c 中的全局变量
volatile VehicleInfo_t g_vehicle_db[MAX_VEHICLES];
volatile uint8_t g_vehicle_count = 0;
volatile uint32_t g_flash_write_addr = FLASH_STORAGE_START_ADDR;


/**
 * @brief 初始化Flash存储
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t flash_storage_init(void)
{
    uint32_t flash_addr = FLASH_STORAGE_START_ADDR;
    uint8_t count = 0;
    VehicleInfo_t temp_info; // 建立一个临时缓冲区，增加安全性

    for(uint8_t i = 0; i < MAX_VEHICLES; i++)
    {
        // 1. 边界检查
        if(flash_addr + sizeof(VehicleInfo_t) > FLASH_STORAGE_END_ADDR)
        {
            break;
        }

        // 2. 将数据从 Flash 拷贝到 RAM 缓冲区进行处理（避开对齐问题）
        memcpy(&temp_info, (void*)flash_addr, sizeof(VehicleInfo_t));

        // 3. 判断是否到达 Flash 空白区 (STM32 擦除后默认为 0xFF)
        if(temp_info.valid == 0xFF)
        {
            break; 
        }

        // 4. 判断是否为有效业务数据
        if(temp_info.valid == 1)
        {
            if(count < MAX_VEHICLES) // 再次确认全局数组不越界
            {
                memcpy(&g_vehicle_db[count], &temp_info, sizeof(VehicleInfo_t));
                count++;
            }
        }

        // 5. 地址偏移
        flash_addr += sizeof(VehicleInfo_t);
    }

    g_vehicle_count = count;
    g_flash_write_addr = flash_addr; // 下一次写入的起始地址

    return 0;
}

/**
 * @brief 保存车辆数据到 Flash
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t save_one_vehicle_to_flash(VehicleInfo_t *vehicle)
{
    if(g_vehicle_count >= MAX_VEHICLES) return 1;

    vehicle->valid = 1;

    // 1. 使用局部变量，防止操作失败导致全局地址污染
    uint32_t current_addr = g_flash_write_addr; 
    
    // 2. 准备缓冲区，解决非4字节对齐的非法内存读取问题
    // 确保写入 Flash 的总是一个完整的 Word 数组
    uint32_t write_buf[sizeof(VehicleInfo_t) / 4 + 1] = {0};
    memcpy(write_buf, vehicle, sizeof(VehicleInfo_t));
    uint32_t words = (sizeof(VehicleInfo_t) + 3) / 4; 

    HAL_FLASH_Unlock();
    __disable_irq();

    for(uint32_t i = 0; i < words; i++)
    {
        // 检查 Flash 是否为空 (0xFFFFFFFF)
        if(*(uint32_t*)current_addr != 0xFFFFFFFF)
        {
            goto ERROR_EXIT; // 发现不为空，说明没擦除干净或空间被占
        }

        if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, current_addr, write_buf[i]) != HAL_OK)
        {
            goto ERROR_EXIT;
        }
        current_addr += 4;
    }

    __enable_irq();
    HAL_FLASH_Lock();

    // 3. 只有全部写入成功，才更新全局状态
    g_flash_write_addr = current_addr; 
    memcpy(&g_vehicle_db[g_vehicle_count], vehicle, sizeof(VehicleInfo_t));
    g_vehicle_count++;

    return 0;

ERROR_EXIT:
    __enable_irq();
    HAL_FLASH_Lock();
    return 1;
}

uint8_t sync_all_vehicles_to_flash(void)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError;
    HAL_StatusTypeDef status;

    // 1. 解锁 Flash
    HAL_FLASH_Unlock();

    // 2. 擦除目标页 (0x0800FC00 所在页)
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = FLASH_STORAGE_START_ADDR;
    EraseInitStruct.NbPages     = 1; // 1KB 足以存放 50 多个 20字节的结构体

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1; // 擦除失败
    }

    // 3. 循环写入整个数组
    uint32_t current_addr = FLASH_STORAGE_START_ADDR;
    
    // 关中断防止写入过程被干扰
    __disable_irq();

    for (uint8_t i = 0; i < g_vehicle_count; i++) 
    {
        // 标记为有效
        g_vehicle_db[i].valid = 1;
        
        // 将结构体拆分为 Word 写入 (20字节 = 5个 Word)
        uint32_t *pData = (uint32_t*)&g_vehicle_db[i];
        for (uint8_t j = 0; j < (sizeof(VehicleInfo_t)+3) / 4; j++) 
        {
            status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, current_addr, pData[j]);
            if (status != HAL_OK) break;
            current_addr += 4;
        }
        if (status != HAL_OK) break;
    }

    __enable_irq();
    HAL_FLASH_Lock();

    return (status == HAL_OK) ? 0 : 1;
}

// uint8_t save_one_vehicle_to_flash(VehicleInfo_t *vehicle)
// {
//     if(g_vehicle_count >= MAX_VEHICLES)
//     {
//         return 1;
//     }

//     vehicle->valid = 1;

//     __disable_irq();// 关闭中断，确保写入过程不被打断

//     HAL_FLASH_Unlock();

//     uint32_t *data = (uint32_t*)vehicle;
//     uint32_t words = sizeof(VehicleInfo_t) / 4;

//     if(sizeof(VehicleInfo_t) % 4 != 0)
//     {
//         words++;
//     }

//     for(uint32_t i = 0; i < words; i++)
//     {
//         if(*(uint32_t*)g_flash_write_addr != 0xFFFFFFFF)
//         {
//             HAL_FLASH_Lock();
//             __enable_irq();
//             return 1;
//         }

//         if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
//                              g_flash_write_addr,
//                              data[i]) != HAL_OK)
//         {
//             HAL_FLASH_Lock();
//             return 1;
//         }

//         g_flash_write_addr += 4;
//     }

//     HAL_FLASH_Lock();

//     __enable_irq();// 恢复中断

//     memcpy(&g_vehicle_db[g_vehicle_count], vehicle, sizeof(VehicleInfo_t));
//     g_vehicle_count++;

//     return 0;
// }

/**
 * @brief 从 Flash 读取车辆数据
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t load_vehicle_data_from_flash(uint32_t *flash_address)
{
    *flash_address = FLASH_STORAGE_START_ADDR;
    uint8_t count = 0;
    
    // 从 Flash 读取数据
    for(uint8_t i = 0; i < MAX_VEHICLES; i++)
    {
        memcpy(&g_vehicle_db[i], (void*)*flash_address, sizeof(VehicleInfo_t));
        *flash_address += sizeof(VehicleInfo_t);
        
        // 统计有效记录数
        if(g_vehicle_db[i].valid)
        {
            count++;
        }
    }
    
    g_vehicle_count = count;
    
    return 0; // 成功
}

/**
 * @brief 清除 Flash 中的某一辆车辆的数据（简单版本）
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t delete_vehicle(uint8_t index)
{
    if(index >= g_vehicle_count)
        return 1;

    g_vehicle_db[index].valid = 0;

    uint32_t addr = FLASH_STORAGE_START_ADDR + index * sizeof(VehicleInfo_t);

    HAL_FLASH_Unlock();

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, 0);

    HAL_FLASH_Lock();

    return 0;
}

/**
 * @brief 清除 Flash 中的车辆数据
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t erase_vehicle_data_in_flash(void)
{
    uint32_t page_error = 0;
    FLASH_EraseInitTypeDef erase_init_struct;
    
    // 解锁 Flash
    HAL_FLASH_Unlock();
    
    // 擦除 Flash 页
    erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init_struct.PageAddress = FLASH_STORAGE_START_ADDR;
    erase_init_struct.NbPages = NUM_PAGES_NEEDED;
    
    if(HAL_FLASHEx_Erase(&erase_init_struct, &page_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1; // 擦除失败
    }
    
    // 锁定 Flash
    HAL_FLASH_Lock();
    
    // 清除 RAM 中的数据
    memset(g_vehicle_db, 0, sizeof(g_vehicle_db));
    g_vehicle_count = 0;
    
    return 0; // 成功
}