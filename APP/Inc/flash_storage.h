#ifndef __FLASH_STORAGE_H
#define __FLASH_STORAGE_H

#include "main.h"
#include "app.h"

// 定义车辆信息结构体
typedef struct {
    char plate_num[16];
    uint16_t balance;
    uint8_t valid;  // 标记是否有效
} VehicleInfo_t;

// 最大车辆数量
#define MAX_VEHICLES 50

// Flash页大小（根据具体MCU调整）
#define FLASH_PAGE_SIZE 1024  // 例如STM32F103为1K，F4为2K

// 计算所需Flash空间
#define VEHICLE_DATA_SIZE sizeof(VehicleInfo_t) * MAX_VEHICLES
#define NUM_PAGES_NEEDED ((VEHICLE_DATA_SIZE + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE)
#define FLASH_STORAGE_START_ADDR 0x08070000  // 预留最后几页用于存储

// 函数声明
uint8_t flash_storage_init(void);
uint8_t save_vehicle_data_to_flash(void);
uint8_t load_vehicle_data_from_flash(void);
uint8_t erase_vehicle_data_in_flash(void);

// 外部变量声明
extern VehicleInfo_t g_vehicle_db[MAX_VEHICLES];
extern int g_vehicle_count;

#endif // __FLASH_STORAGE_H