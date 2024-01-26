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
    DS18B20_MANAGER_ERR_NO_MORE_STRING_AVAILABLE,
    DS18B20_MANAGER_ERR_EXCESS_OF_DEVICES,
    DS18B20_MANAGER_ERR_INSUFFICIENT_MEMORY
};


#endif
