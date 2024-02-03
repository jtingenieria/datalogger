#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include "onewire_types.h"
#include "esp_err.h"
#include "esp_log.h"
#include "string.h"

typedef enum
{
    SENSOR_NONE = 0,
    SENSOR_DS18B20,
    SENSOR_SERIAL
} sensor_type_t;

typedef struct
{
    sensor_type_t sensor_type;
    int quantity_of_signals;
    char delimiter_char;
    char end_char;
    onewire_device_address_t * device_addr;
} string_config_t;

esp_err_t configuration_manager_import_config(string_config_t * strings_config);

#endif
