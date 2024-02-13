#include <stdio.h>
#include "stdbool.h"
#include "configuration_manager.h"
#include "cJSON.h"
#include "usb_sd_fs.h"
#include "esp_heap_caps.h"

const char * TAG = "configuration_manager";

#define NUMBER_OF_STRINGS 6
#define MAX_NAME_LEN 40

static bool reserve_name_memory(string_config_t * string_config, int number_of_names)
{
    string_config->sensor_names = malloc(number_of_names * sizeof(char *));
    if (string_config->sensor_names == NULL)
	{
		ESP_LOGE(TAG, "Failed to allocate sensor_names");
		return false;
	}

    for(int i = 0 ; i < number_of_names; i++)
    {
    	string_config->sensor_names[i] = malloc(MAX_NAME_LEN * sizeof(char));
    	if(string_config->sensor_names[i] == NULL)
    	{
    		ESP_LOGE(TAG, "Failed to allocate string_config->sensor_names[%d]", i);
    		return false;
    	}
    }

    string_config->m = malloc(number_of_names * sizeof(float));
    if (string_config->m == NULL)
	{
		ESP_LOGE(TAG, "Failed to allocate m");
		return false;
	}

    string_config->h = malloc(number_of_names * sizeof(float));
    if (string_config->h == NULL)
	{
		ESP_LOGE(TAG, "Failed to allocate h");
		return false;
	}

    return true;
}

static bool reserve_address_memory(string_config_t * string_config, int number_of_addresses)
{
    string_config->device_addr = malloc(number_of_addresses * sizeof(onewire_device_address_t));
    if (string_config->device_addr == NULL)
	{
		ESP_LOGE(TAG, "Failed to allocate %d device_addr", number_of_addresses);
		return false;
	}

    return true;
}



static bool str_eq(char * string_1, char * string_2)
{
    return (strcmp(string_1, string_2)==0);
}

#define DS18B20_NAME "DS18B20"
#define SERIAL_NAME "serial"


static int configuration_manager_parse_string(char ** const config, string_config_t * strings_config)
{
    const cJSON *delimiter_char = NULL;
    const cJSON *end_char = NULL;
    const cJSON *device_address = NULL;
    const cJSON *device_addresses = NULL;
    const cJSON *device_names = NULL;
    const cJSON *device_name = NULL;
    const cJSON *device_hs = NULL;
    const cJSON *device_ms = NULL;
    const cJSON *device_h = NULL;
    const cJSON *device_m = NULL;
    const cJSON *quantity_of_signals = NULL;
    const cJSON *sensor_type = NULL;
    const cJSON *string_json = NULL;
    int status = 0;
    cJSON *config_json = cJSON_Parse(*config);
    if (config_json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        status = 0;
        return status;
    }

    //printf(cJSON_Print(config_json));

    memset(strings_config, 0, sizeof(string_config_t));

    char string_name[24];
    for (int i = 0 ; i < NUMBER_OF_STRINGS ; i++)
    {
        sprintf(string_name, "String%d",i+1);
        string_json = cJSON_GetObjectItemCaseSensitive(config_json, string_name);
        if (string_json == NULL)
        {
        	ESP_LOGW(TAG, "string_json is null");
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL)
            {
                fprintf(stderr, "Error before: %s\n", error_ptr);
            }
            status = 0;
            return status;
        }
        else
        {
            sensor_type = cJSON_GetObjectItemCaseSensitive(string_json, "SensorType");
            if (cJSON_IsString(sensor_type) && (sensor_type->valuestring != NULL))
            {
                ESP_LOGD(TAG, "Sensor type [%d] \"%s\"",i, sensor_type->valuestring);

                if(str_eq(sensor_type->valuestring, DS18B20_NAME) || str_eq(sensor_type->valuestring, SERIAL_NAME))
                {
					quantity_of_signals = cJSON_GetObjectItemCaseSensitive(string_json, "QuantityOfSignals");
					if (cJSON_IsString(quantity_of_signals) && (quantity_of_signals->valuestring != NULL))
					{
						strings_config[i].quantity_of_signals =  atoi(quantity_of_signals->valuestring);
						ESP_LOGD(TAG, "strings_config[%d].quantity_of_signals = %d", i, strings_config[i].quantity_of_signals);

						if(strings_config[i].quantity_of_signals > 0)
						{
							if(reserve_name_memory(&(strings_config[i]),strings_config[i].quantity_of_signals) != true)
							{
								ESP_LOGE(TAG,"Could not allocate name memory. Largest block: %lu",(long unsigned int) heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
							}
							else
							{
								device_names = cJSON_GetObjectItemCaseSensitive(string_json, "Names");
								int j = 0;
								cJSON_ArrayForEach(device_name, device_names)
								{
									strncpy(strings_config[i].sensor_names[j], device_name->valuestring, MAX_NAME_LEN);
									ESP_LOGD(TAG, "Name[%d][%d] = %s", i,j, strings_config[i].sensor_names[j]);
									j++;
								}

								device_ms = cJSON_GetObjectItemCaseSensitive(string_json, "M");
								j = 0;
								cJSON_ArrayForEach(device_m, device_ms)
								{
									strings_config[i].m[j] = atof(device_m->valuestring);
									ESP_LOGD(TAG, "M[%d][%d] = %.5f", i,j,  strings_config[i].m[j]);
									j++;
								}

								device_hs = cJSON_GetObjectItemCaseSensitive(string_json, "H");
								j = 0;
								cJSON_ArrayForEach(device_h, device_hs)
								{
									strings_config[i].h[j] = atof(device_h->valuestring);
									ESP_LOGD(TAG, "H[%d][%d] = %.5f", i,j,  strings_config[i].h[j]);
									j++;
								}
							}
						}
					}
                }

                if(str_eq(sensor_type->valuestring, DS18B20_NAME))
                {
                    strings_config[i].sensor_type = SENSOR_DS18B20;
                    if(strings_config[i].quantity_of_signals > 0)
                    {
                    	if(reserve_address_memory(&(strings_config[i]),strings_config[i].quantity_of_signals) != true)
						{
							ESP_LOGE(TAG,"Could not allocate address memory. Largest block: %lu",(long unsigned int) heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
						}
						else
						{
							device_addresses = cJSON_GetObjectItemCaseSensitive(string_json, "Addresses");
							int j = 0;
							cJSON_ArrayForEach(device_address, device_addresses)
							{
								char *end;
								strings_config[i].device_addr[j] = strtoull(device_address->valuestring, &end, 16);
								ESP_LOGD(TAG, "Address[%d][%d] = %016llX", i,j, strings_config[i].device_addr[j]);
								j++;
							}
						}
					}
                }
                else if (str_eq(sensor_type->valuestring, SERIAL_NAME))
                {
                    strings_config[i].sensor_type = SENSOR_SERIAL;
                    delimiter_char = cJSON_GetObjectItemCaseSensitive(string_json, "SeparationCharacter");
                    if (cJSON_IsString(delimiter_char) && (delimiter_char->valuestring != NULL))
                    {
                        strings_config[i].delimiter_char = delimiter_char->valuestring[0];
                        ESP_LOGD(TAG, "Delimiter [%d]: %c",i,strings_config[i].delimiter_char);
                    }
                    end_char = cJSON_GetObjectItemCaseSensitive(string_json, "EndOfMessageCharacter");
                    if (cJSON_IsString(end_char) && (end_char->valuestring != NULL))
                    {
                    	if (str_eq(end_char->valuestring, "\\n"))
                    	{
                    		 strings_config[i].end_char = '\n';
                    	}
                    	else if (str_eq(end_char->valuestring, "\\r"))
                    	{
                    		strings_config[i].end_char = '\r';
                    	}
                    	else
                    	{
                    		strings_config[i].end_char = end_char->valuestring[0];
                    		ESP_LOGD(TAG, "End char [%d] may be: %s",i,end_char->valuestring);
                    	}

                        ESP_LOGD(TAG, "End char [%d]: %d",i,(int)strings_config[i].end_char);
                    }


                }
                else
                {
                    strings_config[i].sensor_type = SENSOR_NONE;
                }

            }
        }
    }

    cJSON_Delete(config_json);
    return status;
}

#define CONFIURATION_FILE_NAME "parameters.json"

esp_err_t configuration_manager_import_config(string_config_t * strings_config)
{
	char * file_data = NULL;
	long read_size = 0;

	usb_sd_fs_err_t err;

	//esp_log_level_set(TAG, ESP_LOG_DEBUG);

	err = usb_sd_fs_read_file(CONFIURATION_FILE_NAME, &file_data, &read_size);

	if(err != USB_SD_FS_OK)
	{
		ESP_LOGE(TAG, "Error reading file %d", err);
		if(file_data != NULL)
		{
			free(file_data);
		}
		return ESP_FAIL;
	}
	//if(file_data!=NULL) printf("%s\n", file_data);
	//else ESP_LOGW(TAG, "File is null");
	ESP_LOGI(TAG, "Parsing file");
	configuration_manager_parse_string(&file_data, strings_config);

	free(file_data);

	return ESP_OK;

}
