#ifndef __VEHICLE_INFO_UPDATE_H__
#define __VEHICLE_INFO_UPDATE_H__

#include "main.h"

int Find_Vehicle(const char *plate);
//  添加新车辆信息, 返回索引，数据库满返回-1
int Add_Vehicle(const char *plate);

#endif /* __VEHICLE_INFO_UPDATE_H__ */