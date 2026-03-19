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

// STM32F103 Flash页大小
#define m_FLASH_PAGE_SIZE 1024

// 数据区大小
#define VEHICLE_DATA_SIZE (sizeof(VehicleInfo_t) * MAX_VEHICLES)

// 需要的页数
#define NUM_PAGES_NEEDED ((VEHICLE_DATA_SIZE + m_FLASH_PAGE_SIZE - 1) / m_FLASH_PAGE_SIZE)

// Flash存储区域
#define FLASH_STORAGE_START_ADDR 0x0800FC00
#define FLASH_STORAGE_END_ADDR   0x0800FFFF

// 函数声明
uint8_t flash_storage_init(void);
uint8_t save_one_vehicle_to_flash(VehicleInfo_t *vehicle);
uint8_t sync_all_vehicles_to_flash(void);
uint8_t load_vehicle_data_from_flash(uint32_t *flash_address);
uint8_t erase_vehicle_data_in_flash(void);

// 外部变量声明
extern volatile VehicleInfo_t g_vehicle_db[MAX_VEHICLES];
extern volatile uint8_t g_vehicle_count;
extern volatile uint32_t g_flash_write_addr;
#endif // __FLASH_STORAGE_H