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

#include "ds18b20_manager.h"
#include "usb_sd_fs.h"

static const char * TAG = "logger";

TaskHandle_t logger_task_handle = NULL;
TaskHandle_t external_task_handle = NULL;

#define GPIO_S1_STRING 9
#define MAX_DEVICES_S1_STRING 5
static int assigned_number_S1;

static void logger_task(void *pvParams)
{

    ESP_LOGD(TAG, "Task start wait");
    ulTaskNotifyTakeIndexed(1, pdTRUE, portMAX_DELAY);
    ESP_LOGD(TAG, "Task one done");
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    ESP_LOGD(TAG, "Task done wait");

    char data_line[256];
    char data[40];
    float sensor_data = 0;
    time_t now = 0;
    struct tm timeinfo =
    { 0 };

    memset(data_line, 0, sizeof(data_line));
    time(&now);

    localtime_r(&now, &timeinfo);
    strftime(data_line,
             sizeof(data_line),
             "%H:%M:%S, %d/%m/%Y, ",
             &timeinfo);

    for (int i = 0; i < MAX_DEVICES_S1_STRING; i++)
    {
        sensor_data = ds18b20_manager_get_temp(assigned_number_S1, i);
        if (i != MAX_DEVICES_S1_STRING - 1) sprintf(data, "%.2f, ", sensor_data);
        else sprintf(data, "%.2f", sensor_data);
        strcat(data_line, data);
        ESP_LOGD(TAG, "Sensor %d: %.2f",i,sensor_data);

    }
    ESP_LOGI(TAG, "%s", data_line);
    strcat(data_line, "\n");


    if(usb_sd_fs_write(data_line) == USB_SD_FS_OK)
    {
        //GREEN
    }
    else
    {
        //RED
    }


    if(external_task_handle != NULL)
    {
        ESP_LOGD(TAG, "Notify caller");
        xTaskNotifyIndexed(external_task_handle, 0, 0, eSetValueWithOverwrite);
    }
    else
    {
        ESP_LOGD(TAG, "Nothing to notify");
    }

    vTaskDelete(NULL);
}

void logger_init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    xTaskCreate(logger_task, "logger_task", 8192, NULL, 2, &logger_task_handle);
    ds18b20_manager_init(&logger_task_handle);

    ds18b20_manager_instance_string(GPIO_S1_STRING, MAX_DEVICES_S1_STRING, &assigned_number_S1);

    ds18b20_manager_set_address(assigned_number_S1, 0, 0x515810B60164FF28);

    ds18b20_manager_init_string(assigned_number_S1);



}

void logger_log(bool block_caller_task)
{
    if(block_caller_task)
    {
        ESP_LOGD(TAG, "Getting handle");
        external_task_handle = xTaskGetCurrentTaskHandle();

        xTaskNotifyIndexed(logger_task_handle, 1, 0, eSetValueWithOverwrite);

        ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    }
    else
    {
        xTaskNotifyIndexed(logger_task_handle, 1, 0, eSetValueWithOverwrite);
    }

}
