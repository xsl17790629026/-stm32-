#ifndef __APP_H__
#define __APP_H__

#include "main.h"
void app_init();

typedef enum {
    WAIT_HEAD1,
    WAIT_HEAD2,
    WAIT_DATA,
} state_t;

#endif // !__APP_H__