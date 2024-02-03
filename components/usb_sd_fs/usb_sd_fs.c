#include "usb_sd_fs.h"
#include "esp_system.h"
#include <stdio.h>
#include "stdint.h"
#include "tinyusb.h"
#include "tusb_msc_storage.h"
#include "diskio_impl.h"
#include "diskio_sdmmc.h"
#include "esp_check.h"
#include <errno.h>
#include <dirent.h>
#include "esp_ota_ops.h"        //included header file for esp_ota driver
#include <sys/stat.h>
#include "spi_flash_mmap.h"
#include "stdbool.h"

#define OTA_MAX_BYTES_PER_BATCH 4096

static const char *TAG = "usb_sd_fs";

uint8_t buf[OTA_MAX_BYTES_PER_BATCH+1];

static uint32_t total = 0;
static uint32_t pct = 0;
static uint32_t pct_send = 0;
static char message_data[10];
static esp_ota_handle_t update_handle = 0;

static sdmmc_card_t *card = NULL;
#define BASE_PATH "/data" // base path to mount the partition

static RTC_DATA_ATTR char file_name[128];

static TaskHandle_t external_task_handle = NULL;

static esp_err_t usb_sd_fs_init_sdmmc(sdmmc_card_t **card)
{
    esp_err_t ret = ESP_OK;
    bool host_init = false;
    sdmmc_card_t *sd_card;

    ESP_LOGI(TAG, "Initializing SDCard");

    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 40MHz for SDMMC)
    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = 40000;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // For SD Card, set bus width to use
#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    slot_config.width = 4;
#else
    slot_config.width = 1;
#endif  // CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4

    // On chips where the GPIOs used for SD card can be configured, set the user defined values
#ifdef CONFIG_SOC_SDMMC_USE_GPIO_MATRIX
    slot_config.clk = CONFIG_EXAMPLE_PIN_CLK;
    slot_config.cmd = CONFIG_EXAMPLE_PIN_CMD;
    slot_config.d0 = CONFIG_EXAMPLE_PIN_D0;
#ifdef CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4
    slot_config.d1 = CONFIG_EXAMPLE_PIN_D1;
    slot_config.d2 = CONFIG_EXAMPLE_PIN_D2;
    slot_config.d3 = CONFIG_EXAMPLE_PIN_D3;
#endif  // CONFIG_EXAMPLE_SDMMC_BUS_WIDTH_4

#endif  // CONFIG_SOC_SDMMC_USE_GPIO_MATRIX

    // Enable internal pullups on enabled pins. The internal pullups
    // are insufficient however, please make sure 10k external pullups are
    // connected on the bus. This is for debug / example purpose only.
    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    // not using ff_memalloc here, as allocation in internal RAM is preferred
    sd_card = (sdmmc_card_t *)malloc(sizeof(sdmmc_card_t));
    ESP_GOTO_ON_FALSE(sd_card, ESP_ERR_NO_MEM, clean, TAG, "could not allocate new sdmmc_card_t");

    ESP_GOTO_ON_ERROR((*host.init)(), clean, TAG, "Host Config Init fail");
    host_init = true;

    ESP_GOTO_ON_ERROR(sdmmc_host_init_slot(host.slot, (const sdmmc_slot_config_t *) &slot_config),
                      clean, TAG, "Host init slot fail");

    while (sdmmc_card_init(&host, sd_card)) {
        ESP_LOGE(TAG, "The detection pin of the slot is disconnected(Insert uSD card). Retrying...");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }

    // Card has been initialized, print its properties
    //sdmmc_card_print_info(stdout, sd_card);
    *card = sd_card;

    return ESP_OK;

clean:
    if (host_init) {
        if (host.flags & SDMMC_HOST_FLAG_DEINIT_ARG) {
            host.deinit_p(host.slot);
        } else {
            (*host.deinit)();
        }
    }
    if (sd_card) {
        free(sd_card);
        sd_card = NULL;
    }
    return ret;
}
//tusb_msc_storage.c
//  const MKFS_PARM opt = {(BYTE)!!!FM_ANY!!!!, 0, 0, 0, alloc_unit_size};
// mount the partition and show all the files in BASE_PATH
static void _mount(void)
{
    ESP_LOGI(TAG, "Mount storage...");
    ESP_ERROR_CHECK(tinyusb_msc_storage_mount(BASE_PATH));


    // List all the files in this directory
    ESP_LOGI(TAG, "\nls command output:");
    struct dirent *d;
    DIR *dh = opendir(BASE_PATH);
    if (!dh) {
        if (errno == ENOENT) {
            //If the directory is not found
            ESP_LOGE(TAG, "Directory doesn't exist %s", BASE_PATH);
        } else {
            //If the directory is not readable then throw error and exit
            ESP_LOGE(TAG, "Unable to read directory %s", BASE_PATH);
        }
        return;
    }
    //While the next entry is not readable we will print directory files
//    while ((d = readdir(dh)) != NULL) {
//        printf("%s\n", d->d_name);
//    }
    return;
}


void usb_sd_fs_init_card(void)
{
    ESP_LOGI(TAG, "Initializing storage...");
    ESP_ERROR_CHECK(usb_sd_fs_init_sdmmc(&card));

    const tinyusb_msc_sdmmc_config_t config_sdmmc = {
          .card = card
      };
      ESP_ERROR_CHECK(tinyusb_msc_storage_init_sdmmc(&config_sdmmc));

      //mounted in the app by default
      _mount();

}

void usb_sd_fs_init_usb(void)
{
      ESP_LOGI(TAG, "USB Composite initialization");
      const tinyusb_config_t tusb_cfg = {
          .device_descriptor = NULL,
          .string_descriptor = NULL,
          .string_descriptor_count = 0,
          .external_phy = false,
          .configuration_descriptor = NULL,
          .self_powered = true,
          .vbus_monitor_io = 8
      };
      ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
}

void usb_sd_fs_deinit_usb(void)
{
    tinyusb_driver_uninstall();
}

bool usb_sd_fs_is_storage_in_use_by_host(void)
{
    return tinyusb_msc_storage_in_use_by_usb_host();
}

static usb_sd_fs_err_t usb_sd_fs_write_file(char *path,
                                                    char *data)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, "a+");
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return USB_SD_FS_OPEN_FAILED;
    }
    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    return USB_SD_FS_OK;
}

void usb_sd_fs_set_file(char *file)
{
    sprintf(file_name, "%s/%s", BASE_PATH, file);
    ESP_LOGI(TAG, "Full filename is: %s", file_name);
}

char * usb_sd_fs_get_full_filename(void)
{
    return file_name;
}

bool usb_sd_fs_is_filename_set(void)
{
    return (strncmp(file_name, BASE_PATH, strlen(BASE_PATH)) == 0);
}

usb_sd_fs_err_t usb_sd_fs_write(char *data)
{
    usb_sd_fs_err_t err = USB_SD_FS_OTHER_ERR;

    if (strlen(data) != 0)
    {
        err = usb_sd_fs_write_file(file_name, data);
    }
    return err;

}

usb_sd_fs_err_t usb_sd_fs_read_file(char * file_name,char **data_out , long * read_size)
{

	//esp_log_level_set(TAG, ESP_LOG_INFO);

	char full_path[128];
	sprintf(full_path, "%s/%s", BASE_PATH, file_name);
	FILE *file;
	file = fopen (full_path, "r" );
	if(file == NULL)
	{
		ESP_LOGW(TAG, "File open failed");
		return USB_SD_FS_OPEN_FAILED;
	}
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);

	ESP_LOGI(TAG, "File size is %ld", (long)fsize);

	fseek(file, 0, SEEK_SET);

	*data_out = malloc(fsize + 1);
	memset(*data_out, 0, fsize + 1);
	if(*data_out == NULL)
	{
		ESP_LOGW(TAG, "Malloc failed");
		return USB_SD_FS_MALLOC_FAILED;
	}
	*read_size = fread(*data_out,sizeof(char) , fsize, file);

	ESP_LOGI(TAG, "Read size is %ld. Data out is %p",(long)*read_size, *data_out);

	fclose(file);
	return USB_SD_FS_OK;
}

enum
{
    OTA_UPDATE_OK = ESP_OK,
    OTA_OPEN_FAILED,
    OTA_STAT_FAILED,
    OTA_BEGIN_FAILED,

};

char ota_bin_file_path[50] = BASE_PATH"/datalogger.bin";

static void ota_func_task(void *pvParameter)
{
    FILE *ota_bin_file;
    int finished = 0;


    struct stat entry_stat;
    uint32_t file_size_number = 0;
    char file_size[16];



        printf("OTA Bin File Path = %s \n", ota_bin_file_path);

        ota_bin_file = fopen (ota_bin_file_path, "rb" );

        if (ota_bin_file == NULL) {
            ESP_LOGE(TAG,"Failed to open file for reading \n");
            xTaskNotifyIndexed(external_task_handle, 0, OTA_OPEN_FAILED, eSetValueWithOverwrite);
            vTaskDelete(NULL);
        }else{
            ESP_LOGI(TAG,"Opened OTA BIN file for reading \n");
            //stat();
            if (stat(ota_bin_file_path, &entry_stat) == -1) {
                    ESP_LOGE(TAG,"Failed to stat %s \n", ota_bin_file_path);
                    xTaskNotifyIndexed(external_task_handle, 0, OTA_STAT_FAILED, eSetValueWithOverwrite);
                    vTaskDelete(NULL);
                }
                file_size_number = (uint32_t) entry_stat.st_size;
                ESP_LOGI(TAG,"File Size is %"PRIu32 "\n", file_size_number);

                sprintf(file_size, "%ld", entry_stat.st_size);
                ESP_LOGI(TAG,"Found %s : (%s bytes) \n", ota_bin_file_path, file_size);
        }

        ESP_LOGI(TAG,"starting main task firmware update \n");

        vTaskDelay(pdMS_TO_TICKS(500));

        const esp_partition_t *configured = esp_ota_get_boot_partition();
        const esp_partition_t *running = esp_ota_get_running_partition();

        if (configured != running) {
            printf("Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x \n",(unsigned int)configured->address, (unsigned int)running->address);
            ESP_LOGW(TAG,"This can happen if either the OTA boot data or preferred boot image become corrupted somehow.\n");
        }

        ESP_LOGI(TAG,"Running partition type %u sub-type %u (offset 0x%08x) \n",running->type, running->subtype,  (unsigned int)running->address);

        const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
        assert(update_partition != NULL);

        ESP_LOGI(TAG,"Writing to partition subtype %u at offset 0x%x \n", update_partition->subtype,  (unsigned int)update_partition->address);

        //esp_err_t err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);

        esp_err_t err = esp_ota_begin(update_partition, file_size_number, &update_handle);

        ESP_LOGI(TAG, "esp_begin result = %d", err);
        ESP_LOGD(TAG,"Update Handle is %d \n", (int)update_handle);

        if (err != ESP_OK) {
            ESP_LOGE(TAG,"esp_ota_begin failed, error=%d \n", err);
            ESP_LOGE(TAG,"esp_ota_begin failed\n");
            xTaskNotifyIndexed(external_task_handle, 0, OTA_BEGIN_FAILED, eSetValueWithOverwrite);
            vTaskDelete(NULL);
        }

        printf("esp_ota_begin succeeded \n-----------------------\n");
                    while(!finished)
                    {
                            unsigned int retn = fread(buf, 1, OTA_MAX_BYTES_PER_BATCH, ota_bin_file);
                            printf("First Byte of the Read Data Chunk is %X \n", buf[0]);

                            if(retn != OTA_MAX_BYTES_PER_BATCH)
                            {
                                printf("Finished reading, Last Chunk size %d \n",retn);

                                err = esp_ota_write(update_handle, buf, retn);

                                if (err == ESP_ERR_INVALID_ARG) {
                                    printf("error: esp_ota_write failed! err=0x%x\n", err);
                                    vTaskDelete(NULL);
                                } else if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
                                    printf("error: First byte of image contains invalid app image magic byte\n");
                                } else if (err == ESP_ERR_FLASH_OP_FAIL) {
                                    printf("error: Flash write IO Operaion failed\n");
                                } else if (err == ESP_ERR_FLASH_OP_TIMEOUT) {
                                    printf("error: Flash write failed due to TimeOut\n");
                                } else if (err == ESP_ERR_OTA_SELECT_INFO_INVALID) {
                                    printf("error: OTA data partition has invalid contents\n");
                                } else if (err == ESP_OK) {
                                    printf("Wrote %d Bytes to OTA Partition \n", retn);
                                }


                                ESP_LOGI(TAG, "Ota result = %d", err);

                                total += retn;
                                pct = total * 100 / file_size_number;

                                itoa(pct, message_data, 10);
                                printf("Progress %s %% \n", message_data);
                                printf("Total Bytes Read from the OTA BIN File is %"PRIu32 " \n", total);

                                //vTaskDelay(5000/portTICK_PERIOD_MS);
                                printf("Update Handle is %d \n", (int)update_handle);
                                err = esp_ota_end(update_handle);

                                if (err != ESP_OK) {
                                    if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
                                        ESP_LOGE(TAG, "Image validation failed, image is corrupted");
                                    } else {
                                        ESP_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
                                    }
                                    vTaskDelete(NULL);
                                }

                                //vTaskDelay(5000/portTICK_PERIOD_MS);
                                printf("OTA Update has Ended \n");
                                err = esp_ota_set_boot_partition(update_partition);
                                if (err != ESP_OK) {
                                    ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
                                }
                                vTaskDelay(pdMS_TO_TICKS(250));
                                ESP_LOGI(TAG, "Prepare to restart system!");
                                finished = 1;
                                xTaskNotifyIndexed(external_task_handle, 0, OTA_UPDATE_OK, eSetValueWithOverwrite);
                                break;
                            }
                            else
                            {

                                //gpio_set_level(WiFi_LED, toggle);
                                //toggle = !toggle;

                                err = esp_ota_write(update_handle, buf, retn);

                                if (err == ESP_ERR_INVALID_ARG) {
                                    printf("error: esp_ota_write failed! err=0x%x\n", err);
                                    vTaskDelete(NULL);
                                } else if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
                                    printf("error: First byte of image contains invalid app image magic byte\n");
                                } else if (err == ESP_ERR_FLASH_OP_FAIL) {
                                    printf("error: Flash write IO Operaion failed\n");
                                } else if (err == ESP_ERR_FLASH_OP_TIMEOUT) {
                                    printf("error: Flash write failed due to TimeOut\n");
                                } else if (err == ESP_ERR_OTA_SELECT_INFO_INVALID) {
                                    printf("error: OTA data partition has invalid contents\n");
                                } else if (err == ESP_OK) {
                                    printf("Wrote %d Bytes to OTA Partition \n", retn);
                                }

                                total += OTA_MAX_BYTES_PER_BATCH;

                                pct = total * 100 / file_size_number; //calculated percentage

                                if(pct != pct_send ) //i.e. if the rounded off percentage is updated
                                {
                                    pct_send = pct;
                                    itoa(pct, message_data, 10);
                                    printf("Progress %s %% \n", message_data);
                                }
                            }
                    }//Reading of data from PenDrive complete
    //return ESP_OK;
    vTaskDelete(NULL);
}   //end of OTA function


esp_err_t usb_sd_fs_ota_start(void)
{
    external_task_handle = xTaskGetCurrentTaskHandle();
    xTaskCreate(&ota_func_task, "ota_func_task", 8192, NULL, 5, NULL);
    return ESP_OK;
}

esp_err_t usb_sd_fs_ota_wait_for_result(void)
{
    uint32_t result = ulTaskNotifyTakeIndexed(0, pdTRUE, pdMS_TO_TICKS(120000));
    if(result == ESP_OK)
    {
        remove(ota_bin_file_path);
        return ESP_OK;
    }
    else return ESP_FAIL;
}
