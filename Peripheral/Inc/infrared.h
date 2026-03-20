#ifndef __INFRARED_H
#define __INFRARED_H

#include "main.h"

typedef enum{
    CGQ1 = 0,
    CGQ2 = 1,
}CGQ_TYPE;
uint8_t Read_Infrared_State(CGQ_TYPE type);

#endif