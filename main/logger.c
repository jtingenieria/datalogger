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
#include "driver/gpio.h"

#include "ds18b20_manager.h"
#include "usb_sd_fs.h"
#include "serial_logger.h"
#include "esp_now_module.h"

static const char * TAG = "logger";

typedef struct
{
	time_t time;
	int float_data_qty;
	int id;
	size_t size_of_packet;
	float float_data_p[];
} esp_packet_t;

#define GPIO_RED_LED 5
#define GPIO_GREEN_LED 4

TaskHandle_t logger_task_handle = NULL;
TaskHandle_t external_task_handle = NULL;

#define GPIO_S1_STRING 9
#define MAX_DEVICES_S1_STRING 14
#define GPIO_S56_STRING 15
#define GPIO_SERIAL_PORT 16
#define MAX_DEVICES_S56_STRING 16

#define GPIO_S3_STRING 11
#define MAX_DEVICES_S3_STRING 4

static int assigned_number_S3;

static esp_packet_t * esp_now_packet;
RTC_DATA_ATTR static int esp_now_id = 0;

static onewire_device_address_t addr_map[MAX_DEVICES_S3_STRING] =
{
	0x0B8B6BB10164FF28,
	0x26B66CB10164FF28,
	0x66CB11B60164FF28,
	0x0
};

static float temperature_calibration[MAX_DEVICES_S3_STRING][2] =
{
  {1 , 0},
  {1,  0},
  {1 , 0},
  {1 , 0},
};


void set_esp_now_send_success(void)
{
    xTaskNotifyIndexed(logger_task_handle, 1, 1, eSetValueWithOverwrite);
}

void set_esp_now_send_fail(void)
{
    xTaskNotifyIndexed(logger_task_handle, 1, 0, eSetValueWithOverwrite);
}


static void logger_task(void *pvParams)
{

    ESP_LOGD(TAG, "Task start wait");
    ulTaskNotifyTakeIndexed(1, pdTRUE, portMAX_DELAY);
    ESP_LOGD(TAG, "Task one done");
    ulTaskNotifyTakeIndexed(0, pdTRUE, portMAX_DELAY);
    ESP_LOGD(TAG, "Task done wait");

    uint32_t esp_now_delivery = 0;
    char data_line[256];
    char data[40], data2[20];
    float sensor_data = 0;
    time_t now = 0;
    struct tm timeinfo =
    { 0 };

    memset(data_line, 0, sizeof(data_line));
    time(&now);
    esp_now_packet->time = now;
    localtime_r(&now, &timeinfo);
    strftime(data_line,
             sizeof(data_line),
             "%H:%M:%S, %d/%m/%Y, ",
             &timeinfo);



    for (int i = 0; i < MAX_DEVICES_S3_STRING; i++)
    {
        sensor_data = ds18b20_manager_get_temp(assigned_number_S3, i);
        sensor_data = sensor_data*temperature_calibration[i][0] + temperature_calibration[i][1];
        if (i != MAX_DEVICES_S3_STRING - 1) sprintf(data, "%.2f, ", sensor_data);
        else sprintf(data, "%.2f", sensor_data);
        strcat(data_line, data);
        esp_now_packet->float_data_p[i] = sensor_data;
        ESP_LOGD(TAG, "Sensor %d: %.2f",i,sensor_data);

    }

    memset(data, 0, sizeof(data));
    memset(data2, 0, sizeof(data2));

    serial_packet_t * packet;
    serial_logger_get_data(1000, &packet);


    if(packet->id != -1)
    {
    	ESP_LOGD(TAG, "-1. qty = %d", packet->float_data_qty);
    	ESP_LOGD(TAG, "-1. rx = %d", packet->received_data_qty);
        for(int i = 0; i < packet->received_data_qty; i++)
        {
            sprintf(data2,", %.2f", packet->float_data_p[i] );
            strcat(data, data2);
            esp_now_packet->float_data_p[MAX_DEVICES_S3_STRING + i] = packet->float_data_p[i];
        }

        for(int i = 0; i < (packet->float_data_qty - packet->received_data_qty); i++)
        {
            sprintf(data2,", %.2f", -255.0);
            strcat(data, data2);
            esp_now_packet->float_data_p[MAX_DEVICES_S3_STRING + packet->received_data_qty + i] = -255.0;
        }
       // strcat(data, "\r\n");
    }
    else
    {
        ESP_LOGD(TAG, "-1. qty = %d", packet->float_data_qty);
        for(int j = 0; j < packet->float_data_qty; j++)
        {
            sprintf(data2,", %.2f", -255.0);
            strcat(data, data2);
            esp_now_packet->float_data_p[MAX_DEVICES_S3_STRING + j] = -255.0;
        }

        //strcat(data, "\r\n");
        ESP_LOGV(TAG, "-1 done");
    }
    strcat(data_line, data);

    esp_now_packet->float_data_qty = packet->float_data_qty + MAX_DEVICES_S3_STRING;
    esp_now_packet->id = esp_now_id ++;

    TickType_t xLastWakeTime;

    if(packet->float_data_qty != packet->received_data_qty || packet->id == -1)
    {
    	gpio_set_level(GPIO_RED_LED, true);
		vTaskDelay(pdMS_TO_TICKS(25));
    }
    else
    {
    	gpio_set_level(GPIO_GREEN_LED, true);
		vTaskDelay(pdMS_TO_TICKS(25));
    }
    xLastWakeTime = xTaskGetTickCount();

    gpio_set_level(GPIO_GREEN_LED, false);
	gpio_set_level(GPIO_RED_LED, false);

	free(packet);
    ESP_LOGI(TAG, "%s", data_line);
    strcat(data_line, "\r\n");

    if(usb_sd_fs_write(data_line) == USB_SD_FS_OK)
    {
        //GREEN
    	vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(100) );
    	gpio_set_level(GPIO_GREEN_LED, true);
		vTaskDelay(pdMS_TO_TICKS(25));
    }
    else
    {
        //RED
    	vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(100) );
    	gpio_set_level(GPIO_RED_LED, true);
		vTaskDelay(pdMS_TO_TICKS(25));
    }

    gpio_set_level(GPIO_GREEN_LED, false);
    gpio_set_level(GPIO_RED_LED, false);

    xLastWakeTime = xTaskGetTickCount();

    for (int i = 0; i < 5; i++)
    {
        esp_now_module_send(esp_now_packet, esp_now_packet->size_of_packet);
        esp_now_delivery = ulTaskNotifyTakeIndexed(1,
                                                   pdTRUE,
                                                   pdMS_TO_TICKS(2000));
        ESP_LOGD(TAG, "Delivery notify is %lu", esp_now_delivery);
        if (esp_now_delivery) break;
        vTaskDelay(pdMS_TO_TICKS(25));
    }

    vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(100) );

    if (esp_now_delivery)
    {
        gpio_set_level(GPIO_GREEN_LED, true);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    else
    {
        gpio_set_level(GPIO_RED_LED, true);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    gpio_set_level(GPIO_RED_LED, false);
    gpio_set_level(GPIO_GREEN_LED, false);

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

#define UART_DATA_QTY 5

void logger_init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    xTaskCreate(logger_task, "logger_task", 8192, NULL, 2, &logger_task_handle);
    ds18b20_manager_init(&logger_task_handle);

    ds18b20_manager_instance_string(GPIO_S3_STRING, MAX_DEVICES_S3_STRING, &assigned_number_S3);

    for(int i = 0; i <  MAX_DEVICES_S3_STRING; i++)
    {
    	ds18b20_manager_set_address(assigned_number_S3, i, addr_map[i]);
    }

    ds18b20_manager_init_string(assigned_number_S3);

    serial_logger_init(UART_DATA_QTY, GPIO_SERIAL_PORT);

    size_t s = sizeof (esp_packet_t) - sizeof(float *) + sizeof(float) * (MAX_DEVICES_S3_STRING + UART_DATA_QTY);
    esp_now_packet = malloc(s);
    esp_now_packet->size_of_packet = s;

    esp_now_module_set_success_cb(set_esp_now_send_success);
	esp_now_module_set_fail_cb(set_esp_now_send_fail);
	esp_now_module_init();

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
