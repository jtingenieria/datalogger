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
#include "configuration_manager.h"

static const char * TAG = "logger";

#define NUMBER_OF_STRINGS 6

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
#define GPIO_S2_STRING 10
#define GPIO_S3_STRING 11
#define GPIO_S4_STRING 12
#define GPIO_S5_STRING 15
#define GPIO_S6_STRING 16

#define MAX_DEVICES_S1_STRING 14
#define GPIO_S56_STRING 15
#define GPIO_SERIAL_PORT 16
#define MAX_DEVICES_S56_STRING 15

#define GPIO_S3_STRING 11
#define MAX_DEVICES_S3_STRING 1

static esp_packet_t * esp_now_packet;
RTC_DATA_ATTR static int esp_now_id = 0;

string_config_t strings_config[NUMBER_OF_STRINGS] = {0};

#define UART_DATA_QTY 5

static int assigned_number[NUMBER_OF_STRINGS];
static gpio_num_t gpio_numbers[] =
{
 GPIO_S1_STRING,
 GPIO_S2_STRING,
 GPIO_S3_STRING,
 GPIO_S4_STRING,
 GPIO_S5_STRING,
 GPIO_S6_STRING,
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
    int absolute_signal_position = 0;
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


    for(int i = 0; i <  NUMBER_OF_STRINGS; i++)
    {
    	switch(strings_config[i].sensor_type)
    	{
    		case SENSOR_DS18B20:
    			if(strings_config[i].quantity_of_signals > 0)
    			{
    				for (int j = 0 ; j < strings_config[i].quantity_of_signals; j++)
    				{
						sensor_data = ds18b20_manager_get_temp(assigned_number[i], j);
						sensor_data = strings_config[i].m[j] * sensor_data + strings_config[i].h[j];
						sprintf(data, "%.2f, ", sensor_data);
						strcat(data_line, data);
						esp_now_packet->float_data_p[absolute_signal_position] = sensor_data;
						absolute_signal_position++;
						ESP_LOGD(TAG, "Sensor %d %d: %.2f",i,j,sensor_data);
    				}
    			}
    			break;
    		case SENSOR_SERIAL:
                ESP_LOGD(TAG, "Serial sensor in %d with %d signals",i, strings_config[i].quantity_of_signals);
    			if(strings_config[i].quantity_of_signals > 0)
				{
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
    			            sprintf(data2,"%.2f, ", packet->float_data_p[i] );
    			            strcat(data, data2);
    			            esp_now_packet->float_data_p[absolute_signal_position] = packet->float_data_p[i];
    			            absolute_signal_position++;
    			        }

    			        for(int i = 0; i < (packet->float_data_qty - packet->received_data_qty); i++)
    			        {
    			            sprintf(data2,"%.2f, ", -255.0);
    			            strcat(data, data2);
    			            esp_now_packet->float_data_p[absolute_signal_position] = -255.0;
    			            absolute_signal_position++;
    			        }
    			       // strcat(data, "\r\n");
    			    }
    			    else
    			    {
    			        ESP_LOGD(TAG, "-1. qty = %d", packet->float_data_qty);
    			        for(int j = 0; j < packet->float_data_qty; j++)
    			        {
    			            sprintf(data2,"%.2f, ", -255.0);
    			            strcat(data, data2);
    			            esp_now_packet->float_data_p[absolute_signal_position] = -255.0;
    			            absolute_signal_position++;
    			        }

    			        //strcat(data, "\r\n");
    			        ESP_LOGV(TAG, "-1 done");
    			    }
    			    strcat(data_line, data);
    			    free(packet);

    			}
    			break;
    		default:
    			break;
    	}
    }



    esp_now_packet->float_data_qty = absolute_signal_position-1;
    esp_now_packet->id = esp_now_id ++;

    TickType_t xLastWakeTime;

//    if(packet->float_data_qty != packet->received_data_qty || packet->id == -1)
//    {
//    	gpio_set_level(GPIO_RED_LED, true);
//		vTaskDelay(pdMS_TO_TICKS(25));
//    }
//    else
//    {
//    	gpio_set_level(GPIO_GREEN_LED, true);
//		vTaskDelay(pdMS_TO_TICKS(25));
//    }
    xLastWakeTime = xTaskGetTickCount();

    gpio_set_level(GPIO_GREEN_LED, false);
	gpio_set_level(GPIO_RED_LED, false);

//	free(packet);
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
        ESP_LOGV(TAG, "Notify caller");
        xTaskNotifyIndexed(external_task_handle, 0, 0, eSetValueWithOverwrite);
    }
    else
    {
        ESP_LOGV(TAG, "Nothing to notify");
    }

    vTaskDelete(NULL);
}


void logger_init(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

	bool serial_logger_assigned = 0;

	int total_signal_qty = 0;

	configuration_manager_import_config(strings_config);

    for(int i = 0; i <  NUMBER_OF_STRINGS; i++)
    {
    	//ESP_LOGI(TAG, "Sensor %d type: %d",i+1, strings_config[i].sensor_type);
    	//ESP_LOGI(TAG, "Sensor %d qty: %d",i+1, strings_config[i].quantity_of_signals);
    	//ESP_LOGI(TAG, "Sensor %d delim: %d",i+1, strings_config[i].delimiter_char);
    	//ESP_LOGI(TAG, "Sensor %d end: %d",i+1, strings_config[i].end_char);

//    	for(int j = 0 ; j < strings_config[i].quantity_of_signals ; j++)
//    	{
//    		ESP_LOGI(TAG, "Sensor %d addr %d:  %016llX",i+1,j+1, strings_config[i].device_addr[j]);
//    	}

    }

    xTaskCreate(logger_task, "logger_task", 8192, NULL, 2, &logger_task_handle);
    ds18b20_manager_init(&logger_task_handle);

    for(int i = 0; i <  NUMBER_OF_STRINGS; i++)
    {
    	switch(strings_config[i].sensor_type)
    	{
    		case SENSOR_DS18B20:
    			if(strings_config[i].quantity_of_signals > 0)
    			{
    				ds18b20_manager_instance_string(gpio_numbers[i], strings_config[i].quantity_of_signals, &(assigned_number[i]));
    			    for(int j = 0; j <  strings_config[i].quantity_of_signals; j++)
    			    {
    			    	ds18b20_manager_set_address(assigned_number[i], j, strings_config[i].device_addr[j]);
    			    	ESP_LOGV(TAG, "Address[%d][%d] set as %016llX",i,j,strings_config[i].device_addr[j]);
    			    }
    			    ds18b20_manager_init_string(assigned_number[i]);
    			    total_signal_qty += strings_config[i].quantity_of_signals;
    			}
    			break;
    		case SENSOR_SERIAL:
    			if(strings_config[i].quantity_of_signals > 0)
				{
					if(!serial_logger_assigned)
					{
						serial_logger_assigned = true;
						serial_logger_init(strings_config[i].quantity_of_signals, gpio_numbers[i]);
						total_signal_qty += strings_config[i].quantity_of_signals;

					}
    			}
    			break;
    		default:
    			break;
    	}
    }

    ds18b20_manager_enable_task();

    size_t s = sizeof (esp_packet_t) - sizeof(float *) + sizeof(float) * (total_signal_qty);
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
