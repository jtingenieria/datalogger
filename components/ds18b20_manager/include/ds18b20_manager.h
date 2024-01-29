#ifndef DS18B20_MANAGER_H
#define DS18B20_MANAGER_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_check.h"
#include "ds18b20.h"
#include "onewire_bus.h"


typedef int ds18b20_manager_err_t;

enum
{
    DS18B20_MANAGER_ERR_OK = 0,
    DS18B20_MANAGER_ERR_NOT_CREATED,
    DS18B20_MANAGER_ERR_NO_MORE_STRING_AVAILABLE,
    DS18B20_MANAGER_ERR_EXCESS_OF_DEVICES,
    DS18B20_MANAGER_ERR_INSUFFICIENT_MEMORY
};

ds18b20_manager_err_t ds18b20_manager_init(TaskHandle_t * task_to_block);


ds18b20_manager_err_t ds18b20_manager_instance_string(int gpio_number,
                                                      int max_devices,
                                                      int *string_number);

ds18b20_manager_err_t ds18b20_manager_init_string(int string_number);

float ds18b20_manager_get_temp(int string_number, int index_in_string);

void ds18b20_manager_set_address(int string_number, int index_in_string, onewire_device_address_t address);


#endif
