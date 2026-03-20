#include "vehicle_info_update.h"
#include "flash_storage.h"
#include "app.h"
#include "oled.h"

// 通过车牌号查找车辆信息，返回索引，未找到返回-1
int Find_Vehicle(const char *plate)
{
    for (int i = 0; i < g_vehicle_count; i++) {
        if (strcmp(g_vehicle_db[i].plate_num, plate) == 0) {
            return i; // 找到了
        }
    }
    return -1; // 没有找到
}

//  添加新车辆信息, 返回索引，数据库满返回-1
int Add_Vehicle(const char *plate)
{
    if (g_vehicle_count >= MAX_VEHICLES) {
        return -1; // 数据库满
    }
    // 复制车牌
    strncpy(g_vehicle_db[g_vehicle_count].plate_num, plate, sizeof(g_vehicle_db[g_vehicle_count].plate_num) - 1);
    g_vehicle_db[g_vehicle_count].plate_num[sizeof(g_vehicle_db[g_vehicle_count].plate_num) - 1] = '\0'; // 确保字符串以null结尾
    // 新车默认余额为0
    g_vehicle_db[g_vehicle_count].balance = 0; // 初始化余额

    g_vehicle_count++;
    return g_vehicle_count - 1;
}

void Clear_All_Vehicle_Data(void)
{
    erase_vehicle_data_in_flash();
    memset((void*)g_vehicle_db, 0, sizeof(g_vehicle_db));
    g_vehicle_count = 0;
	
		OLED_Clear();

		OLED_ShowChinese(32, 0, "车库系统");
		OLED_ShowChinese(0, 16, "余额：");
		OLED_ShowChinese(70, 16, "空位：");
		OLED_ShowChinese(0, 33, "车牌号：");

		OLED_ShowNum(40, 16, 0, 3, OLED_8X16);            // 显示余额
		OLED_ShowNum(110, 16, 50, 2, OLED_8X16); // 显示空位
		OLED_Update();
}