#include "flash_storage.h"
#include "string.h"

// 全局变量，替换app.c中的定义
VehicleInfo_t g_vehicle_db[MAX_VEHICLES];
int g_vehicle_count = 0;

/**
 * @brief 初始化Flash存储
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t flash_storage_init(void)
{
    // 加载存储的数据
    return load_vehicle_data_from_flash();
}

/**
 * @brief 保存车辆数据到Flash
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t save_vehicle_data_to_flash(void)
{
    uint32_t flash_address = FLASH_STORAGE_START_ADDR;
    uint32_t page_error = 0;
    FLASH_EraseInitTypeDef erase_init_struct;
    
    // 解锁Flash
    HAL_FLASH_Unlock();
    
    // 擦除Flash页
    erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init_struct.PageAddress = FLASH_STORAGE_START_ADDR;
    erase_init_struct.NbPages = NUM_PAGES_NEEDED;
    
    if(HAL_FLASHEx_Erase(&erase_init_struct, &page_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1; // 擦除失败
    }
    
    // 写入数据
    for(int i = 0; i < MAX_VEHICLES; i++)
    {
        uint32_t *data_ptr = (uint32_t*)&g_vehicle_db[i];
        int words_count = sizeof(VehicleInfo_t) / 4;
        
        for(int j = 0; j < words_count; j++)
        {
            if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, flash_address, data_ptr[j]) != HAL_OK)
            {
                HAL_FLASH_Lock();
                return 1; // 写入失败
            }
            flash_address += 4; // 每次写入4字节
        }
    }
    
    // 锁定Flash
    HAL_FLASH_Lock();
    
    return 0; // 成功
}

/**
 * @brief 从Flash加载车辆数据
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t load_vehicle_data_from_flash(void)
{
    uint32_t flash_address = FLASH_STORAGE_START_ADDR;
    uint8_t count = 0;
    
    // 从Flash读取数据
    for(int i = 0; i < MAX_VEHICLES; i++)
    {
        memcpy(&g_vehicle_db[i], (void*)flash_address, sizeof(VehicleInfo_t));
        flash_address += sizeof(VehicleInfo_t);
        
        // 统计有效的车辆记录
        if(g_vehicle_db[i].valid)
        {
            count++;
        }
    }
    
    g_vehicle_count = count;
    
    return 0; // 成功
}

/**
 * @brief 擦除Flash中的车辆数据
 * @retval uint8_t 0-成功 1-失败
 */
uint8_t erase_vehicle_data_in_flash(void)
{
    uint32_t page_error = 0;
    FLASH_EraseInitTypeDef erase_init_struct;
    
    // 解锁Flash
    HAL_FLASH_Unlock();
    
    // 擦除Flash页
    erase_init_struct.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init_struct.PageAddress = FLASH_STORAGE_START_ADDR;
    erase_init_struct.NbPages = NUM_PAGES_NEEDED;
    
    if(HAL_FLASHEx_Erase(&erase_init_struct, &page_error) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1; // 擦除失败
    }
    
    // 锁定Flash
    HAL_FLASH_Lock();
    
    // 清空RAM中的数据
    memset(g_vehicle_db, 0, sizeof(g_vehicle_db));
    g_vehicle_count = 0;
    
    return 0; // 成功
}