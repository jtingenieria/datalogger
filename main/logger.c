/*
 * logger.c
 *
 *  Created on: 3 ene. 2024
 *      Author: juant
 */


#include "logger.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <string.h>

static RTC_DATA_ATTR char file_name[128];

#define MOUNT_POINT "/sdcard"


static void logger_task(void *pvParams)
{



    vTaskDelete(NULL);
}

void logger_init(void)
{

}

void logger_log(void)
{
    //xTaskNotifyIndexed(log_task_handle, 0, 0, eSetValueWithOverwrite);
}
