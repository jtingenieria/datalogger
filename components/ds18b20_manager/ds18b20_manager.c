#include <stdio.h>
#include "ds18b20_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "onewire_bus.h"
#include "ds18b20.h"

static const char *TAG = "ds18b20_manager";

#define DS18B20_MANAGER_MAX_STRINGS 6
#define DS18B20_MANAGER_MAX_DEVICES_PER_STRING 255

typedef struct
{
    onewire_bus_handle_t bus;
    onewire_bus_config_t bus_config;
    onewire_bus_rmt_config_t rmt_config;
    ds18b20_device_handle_t * ds18b20s;
    int gpio_number;
    int detected_devices;
    int max_devices;
    onewire_device_address_t * address_map;
    float * measured_temp;
    int string_number;
} ds18b20_string;

static ds18b20_string sensor_string[DS18B20_MANAGER_MAX_STRINGS];
static int sensor_string_count = 0;
static TaskHandle_t _task_to_block;

ds18b20_manager_err_t ds18b20_manager_init(TaskHandle_t task_to_block)
{
    _task_to_block = task_to_block;
    return DS18B20_MANAGER_ERR_OK;
}

ds18b20_manager_err_t ds18b20_manager_instance_string(int gpio_number,
                                                      int max_devices,
                                                      int *string_number)
{

    if (sensor_string_count == DS18B20_MANAGER_MAX_STRINGS)
    {
        return DS18B20_MANAGER_ERR_NO_MORE_STRING_AVAILABLE;
    }

    if (max_devices > DS18B20_MANAGER_MAX_DEVICES_PER_STRING)
    {
        return DS18B20_MANAGER_ERR_EXCESS_OF_DEVICES;
    }

    sensor_string[sensor_string_count].string_number = sensor_string_count;
    sensor_string[sensor_string_count].gpio_number = gpio_number;
    sensor_string[sensor_string_count].max_devices = max_devices;
    sensor_string[sensor_string_count].measured_temp = malloc(sizeof(float) * max_devices);
    sensor_string[sensor_string_count].ds18b20s = malloc(sizeof(ds18b20_device_handle_t) * max_devices);
    sensor_string[sensor_string_count].address_map = malloc(sizeof(onewire_device_address_t) * max_devices);

    if (sensor_string[sensor_string_count].ds18b20s == NULL || sensor_string[sensor_string_count].address_map == NULL || sensor_string[sensor_string_count].measured_temp == NULL)
    {
        return DS18B20_MANAGER_ERR_INSUFFICIENT_MEMORY;
    }

    if (string_number != NULL)
    {
        *string_number = sensor_string_count;
    }
    sensor_string_count++;
    return DS18B20_MANAGER_ERR_OK;
}

ds18b20_manager_err_t ds18b20_manager_init_string(int string_number)
{

    sensor_string[string_number].detected_devices = 0;
    sensor_string[string_number].rmt_config.max_rx_bytes = 10; // 1byte ROM command + 8byte ROM number + 1byte device command
    sensor_string[string_number].bus_config.bus_gpio_num = sensor_string[string_number].gpio_number;


    ESP_ERROR_CHECK(onewire_new_bus_rmt(&(sensor_string[string_number].bus_config),
                                        &(sensor_string[string_number].rmt_config),
                                        &(sensor_string[string_number].bus)));
    ESP_LOGI(TAG,
             "1-Wire bus installed on GPIO%d",
             sensor_string[string_number].gpio_number);

    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;
    esp_err_t search_result = ESP_OK;

    // create 1-wire device iterator, which is used for device search
    ESP_ERROR_CHECK(onewire_new_device_iter(sensor_string[string_number].bus, &iter));
    ESP_LOGI(TAG, "Device iterator created, start searching...");
    do
    {
        search_result = onewire_device_iter_get_next(iter,
                                                     &next_onewire_device);
        if (search_result == ESP_OK)
        { // found a new device, let's check if we can upgrade it to a DS18B20
            ds18b20_config_t ds_cfg =
            { };
            if (ds18b20_new_device(&next_onewire_device,
                                   &ds_cfg,
                                   &sensor_string[string_number].ds18b20s[sensor_string[string_number].detected_devices])
                == ESP_OK)
            {
//                ESP_LOGI(TAG, "Found a DS18B20[%d], address: %016llX", sensor_string[string_number].detected_devices, next_onewire_device.address);
                sensor_string[string_number].ds18b20s[sensor_string[string_number].detected_devices]->user_id = -1;

                for (int j = 0; j < sensor_string[string_number].max_devices; j++)
                {
                    if (sensor_string[string_number].address_map[j] == sensor_string[string_number].ds18b20s[sensor_string[string_number].detected_devices]->addr)
                    {
                      ESP_LOGI(   TAG,
                                  "Matched DS18B20[%d], address: %016llX to position %d",
                                  sensor_string[string_number].detected_devices,
                                  next_onewire_device.address,
                                  j);
                        sensor_string[string_number].ds18b20s[sensor_string[string_number].detected_devices]->user_id = j;
                        continue;
                    }

                }

                if (sensor_string[string_number].ds18b20s[sensor_string[string_number].detected_devices]->user_id == -1)
                {
                    ESP_LOGW(TAG,
                             "Unmatched DS18B20[%d], address: %016llX",
                             sensor_string[string_number].detected_devices,
                             next_onewire_device.address);
                }

                sensor_string[string_number].detected_devices++;

                if (sensor_string[string_number].detected_devices >= sensor_string[string_number].max_devices)
                {
                    ESP_LOGI(TAG,
                             "Max DS18B20 number reached, stop searching...");
                    break;
                }
            }
            else
            {
                  ESP_LOGI(   TAG,
                              "Found an unknown device, address: %016llX",
                              next_onewire_device.address);
            }
        }
    } while (search_result != ESP_ERR_NOT_FOUND);
    ESP_ERROR_CHECK(onewire_del_device_iter(iter));
    ESP_LOGI(TAG,
             "Searching done, %d DS18B20 device(s) found",
             sensor_string[string_number].detected_devices);

    // set resolution for all DS18B20s
//    for (int j = 0; j < sensor_string[string_number].detected_devices; j++)
//    {
//        // set resolution
//
//        esp_err_t err = ESP_OK;
//        err = ds18b20_set_resolution(sensor_string[string_number].ds18b20s[j],
//                                     DS18B20_RESOLUTION_12B);
//        if (err != ESP_OK) ESP_LOGE(TAG, "Error in set_resolution %d", err);
//
//    }

    return DS18B20_MANAGER_ERR_OK;
}


void ds18b20_manager_task(void * params)
{

    float temperature = 0;

    for(;;)
    {
        for(int j = 0; j < sensor_string_count ; j++)
        {
            if(sensor_string[j].ds18b20s[0] == NULL) continue;
            ds18b20_trigger_temperature_conversion_for_all(sensor_string[j].ds18b20s[0]);
        }
        ESP_LOGI(TAG, "Triggered conversions");

        vTaskDelay(pdMS_TO_TICKS(800));

        for (int j = 0; j < sensor_string_count; j++)
        {
            for (int i = 0; i < sensor_string[j].detected_devices; i++)
            {
                esp_err_t err = ESP_OK;

                err = ds18b20_get_temperature(  sensor_string[j].ds18b20s[i],
                                                &temperature);

                if (err != ESP_OK)
                {
                    ESP_LOGE(TAG, "Error in get_temperature %d", err);
                    if (sensor_string[j].ds18b20s[i]->user_id != -1) sensor_string[j].measured_temp[sensor_string[j].ds18b20s[i]->user_id] = -254;
                }

                else if (sensor_string[j].ds18b20s[i]->user_id != -1) sensor_string[j].measured_temp[sensor_string[j].ds18b20s[i]->user_id] = temperature;
            }
        }
        ESP_LOGI(TAG, "Done with conversions");
        xTaskNotifyIndexed(_task_to_block, 2, 0, eSetValueWithOverwrite);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


float ds18b20_manager_get_temp(int string_number, int index_in_string)
{

    for (int i = 0; i < sensor_string[string_number].detected_devices; i++)
    {

        if (index_in_string == sensor_string[string_number].ds18b20s[i]->user_id)
        {
            return sensor_string[string_number].measured_temp[index_in_string];
        }

    }

    return -255;
}
