#include <stdio.h>
#include "stdbool.h"
#include "configuration_manager.h"
#include "cJSON.h"
#include "usb_sd_fs.h"


const char * TAG = "configuration_manager";

#define NUMBER_OF_STRINGS 6

static bool reserve_address_memory(string_config_t * string_config, int number_of_addresses)
{
   string_config->device_addr = malloc(number_of_addresses * sizeof(onewire_device_address_t));
   return (string_config->device_addr != NULL);
}

static void free_address_memory(string_config_t * string_config)
{
    free(string_config->device_addr);
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

                if(str_eq(sensor_type->valuestring, DS18B20_NAME))
                {
                    strings_config[i].sensor_type = SENSOR_DS18B20;

                    quantity_of_signals = cJSON_GetObjectItemCaseSensitive(string_json, "QuantityOfSignals");
                    if (cJSON_IsString(quantity_of_signals) && (quantity_of_signals->valuestring != NULL))
                    {
                        strings_config[i].quantity_of_signals =  atoi(quantity_of_signals->valuestring);
                        ESP_LOGD(TAG, "strings_config[%d].quantity_of_signals = %d", i, strings_config[i].quantity_of_signals);

                        reserve_address_memory(&(strings_config[i]),strings_config[i].quantity_of_signals);

                        device_addresses = cJSON_GetObjectItemCaseSensitive(string_json, "DeviceAddresses");
                        int j = 0;
                        cJSON_ArrayForEach(device_address, device_addresses)
                        {
                            char *end;
                            strings_config[i].device_addr[j] = strtoull(device_address->valuestring, &end, 16);
                            //sscanf(device_address->valuestring, "%" SCNx64, &(strings_config[i].device_addr[j]));
                            ESP_LOGD(TAG, "Address[%d][%d] = %016llX", i,j, strings_config[i].device_addr[j]);
                            j++;
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
                    quantity_of_signals = cJSON_GetObjectItemCaseSensitive(string_json, "QuantityOfSignals");
                    if (cJSON_IsString(quantity_of_signals) && (quantity_of_signals->valuestring != NULL))
                    {
                        strings_config[i].quantity_of_signals =  atoi(quantity_of_signals->valuestring);
                        ESP_LOGD(TAG, "strings_config[%d].quantity_of_signals = %d", i, strings_config[i].quantity_of_signals);
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

	//esp_log_level_set(TAG, ESP_LOG_INFO);

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
