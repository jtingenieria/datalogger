#include <errno.h>
#include <dirent.h>

#include "esp_check.h"
#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_vfs_common.h"
#include <fcntl.h>


#include <stdio.h>
#include <string.h>
#include "esp_sleep.h"
#include "sdkconfig.h"
#include "driver/rtc_io.h"
#include "rom/rtc.h"
#include "nvs_flash.h"

#include "graphic_driver.h"
#include "usb_console.h"
#include "usb_sd_fs.h"
#include "logger.h"
#include "time_driver.h"
#include "wifi_cx.h"

#define GPIO_DP 20
#define GPIO_DN 19
#define GPIO_RED_LED 5
#define GPIO_GREEN_LED 4
#define GPIO_MODE_BTN 7
#define GPIO_OK_BTN 6

static const char *TAG = "datalogger";

static RTC_DATA_ATTR struct timeval sleep_enter_time;
static RTC_DATA_ATTR int boot_instruction = 0;


enum{
    BOOT_INSTRUCTION_NONE = 0,
    BOOT_INSTRUCTION_UPDATE_FIRMWARE,
};

enum{
    DATALOGGER_IS_NOT_LOGGING = 0,
    DATALOGGER_IS_LOGGING
};

static RTC_DATA_ATTR int datalogger_logging_status = DATALOGGER_IS_NOT_LOGGING;
static RTC_DATA_ATTR bool date_is_acquired = false;

TickType_t tick_bus_enabled;

static void enable_btn_wakeup(void)
{
    //printf("Enabling EXT0 wakeup on pin GPIO%d\n", ext_wakeup_pin_0);
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(GPIO_OK_BTN, 1));
    ESP_ERROR_CHECK(rtc_gpio_pullup_dis(GPIO_OK_BTN));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(GPIO_OK_BTN));
}

static void deep_sleep_register_rtc_timer_wakeup(void)
{
    const int wakeup_time_sec = 10;
    ESP_LOGD(TAG, "Going to sleep, %ds\n", wakeup_time_sec);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
}


#define USB_DETECT_GPIO 8
#define BUS_ENABLE_PIN 2

static bool is_usb_connected(void)
{

    return gpio_get_level(USB_DETECT_GPIO);
}

static bool start_btn_timer(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    if(gpio_get_level(GPIO_OK_BTN)){
        datalogger_logging_status = datalogger_logging_status ? 0 : 1;
    }
    ESP_LOGI(TAG, "Now logging is %d", datalogger_logging_status);
    return datalogger_logging_status;
//    while(gpio_get_level(ext_wakeup_pin_0)){
//        vTaskDelay(pdMS_TO_TICKS(50));
//    }
//    vTaskDelay(pdMS_TO_TICKS(50));
}

void display_sensors(void)
{
    vTaskDelay(pdMS_TO_TICKS(500));
    graphic_driver_show_text("Sensor 1: 51°C");
    vTaskDelay(pdMS_TO_TICKS(1000));
}

static void enable_bus(bool enabled)
{
    gpio_set_direction(BUS_ENABLE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(BUS_ENABLE_PIN, enabled);
    tick_bus_enabled = xTaskGetTickCount();
}

void graphics_init()
{
	vTaskDelayUntil( &tick_bus_enabled, pdMS_TO_TICKS(500) );
	graphic_driver_init();
}

void set_time_success(void)
{
    time_t now = 0;
    struct tm timeinfo =
    { 0 };
    char strftime_buf[128];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf,
             sizeof(strftime_buf),
             "%d_%m_%Y_%H_%M.csv",
             &timeinfo);
    ESP_LOGI(TAG, "Time obtained. Filename is: %s", strftime_buf);
    usb_sd_fs_set_file(strftime_buf);


    //xTaskNotifyIndexed(log_task_handle, 0, SC_WIFI_IS_ON, eSetValueWithOverwrite);
}

void set_filename_from_time(void)
{
    time_t now = 0;
    struct tm timeinfo =
    { 0 };
    char strftime_buf[128];
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf,
             sizeof(strftime_buf),
             "%d_%m_%Y_%H_%M.csv",
             &timeinfo);
    usb_sd_fs_set_file(strftime_buf);
}

static void init_nvs()
{
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

static void init_logging(bool force_rename)
{
    graphic_driver_show_recording(true);
    datalogger_logging_status = DATALOGGER_IS_LOGGING;
    vTaskDelay(pdMS_TO_TICKS(1000));
    if(!usb_sd_fs_is_filename_set() || force_rename)
    {
        set_filename_from_time();
    }
    esp_sleep_enable_timer_wakeup(0);
    esp_deep_sleep_start();
}

static void init_usb(void)
{
    ESP_LOGI(TAG, "Connect USB");
    init_nvs();
    wifi_init_sta();
    time_driver_initialize();
    usb_sd_fs_init_card();
    usb_sd_fs_init_usb();
//                usb_console_init();
    graphics_init();
    //graphic_driver_usb_connected();
    graphic_driver_show_usb();
}



void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    gpio_set_direction(USB_DETECT_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_MODE_BTN, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_DN, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_DP, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_RED_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_GREEN_LED, GPIO_MODE_OUTPUT);

    if(esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) enable_bus(true);
    //vTaskDelay(pdMS_TO_TICKS(100));
    enable_btn_wakeup();

    switch(boot_instruction)
    {
        case BOOT_INSTRUCTION_UPDATE_FIRMWARE:
                ESP_LOGI(TAG, "Update mode");
                graphics_init();
                graphic_driver_show_text("Starting update");
                usb_sd_fs_init_card();
                usb_sd_fs_ota_start();
                esp_err_t err = usb_sd_fs_ota_wait_for_result();
                ESP_LOGI(TAG, "Update done, result %d", (int)err);
                if(err==ESP_OK)
                {
                    graphic_driver_show_text("Update ok\nRebooting...");
                }
                else
                {
                    graphic_driver_show_text("Update failed\nRebooting...");
                }
                boot_instruction = BOOT_INSTRUCTION_NONE;
                vTaskDelay(pdMS_TO_TICKS(1500));
                esp_sleep_enable_timer_wakeup(0);
                esp_deep_sleep_start();
            break;

        case BOOT_INSTRUCTION_NONE:
            switch (esp_sleep_get_wakeup_cause())
                {
                    case ESP_SLEEP_WAKEUP_TIMER:
            //            printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
                        if(datalogger_logging_status == DATALOGGER_IS_LOGGING)
                        {
                            ESP_LOGD(TAG, "About to log");
                            time_driver_initialize_from_sleep();
                            init_nvs();
                            usb_sd_fs_init_card();
                            logger_init();
                            logger_log(true);
                            ESP_LOGD(TAG, "Log done");
                            deep_sleep_register_rtc_timer_wakeup();
                            esp_deep_sleep_start();
                        }
                        else
                        {
                            if(is_usb_connected())
                            {
                                init_usb();
                            }
                            else
                            {
                                ESP_LOGI(TAG, "Not logging");
                                esp_deep_sleep_start();
                            }

                        }
                        break;

                    case ESP_SLEEP_WAKEUP_EXT0:
                        if(datalogger_logging_status == DATALOGGER_IS_LOGGING)
                        {
                            ESP_LOGI(TAG, "Want to see sensors");
                            graphics_init();
                            start_btn_timer();
                            if(datalogger_logging_status == DATALOGGER_IS_LOGGING)
                            {
                               display_sensors();
                               deep_sleep_register_rtc_timer_wakeup();
                               esp_deep_sleep_start();
                            }
                            else
                            {
                                graphic_driver_show_recording(false);
                                while(gpio_get_level(GPIO_OK_BTN))
                                {
                                    vTaskDelay(pdMS_TO_TICKS(50));
                                }
                                //vTaskDelay(pdMS_TO_TICKS(50));
                                //graphic_driver_show_text("Stopped recording data");
                                vTaskDelay(pdMS_TO_TICKS(1000));
                                if(is_usb_connected())
                                {
                                    esp_sleep_enable_timer_wakeup(0);
                                }
                                else
                                {

                                }
                                esp_deep_sleep_start();
                            }

                        }
                        else if(is_usb_connected())
                        {
                            init_usb();
                        }
                        else
                        {

                            ESP_LOGI(TAG, "Init logging");
                            //vTaskDelay(pdMS_TO_TICKS(250));
                            graphics_init();
                            time_driver_initialize_from_sleep();
                            //vTaskDelay(pdMS_TO_TICKS(1000));
                            //graphic_driver_show_text("Recording data.");
                            init_logging(true);
                        }
                        break;

                    case ESP_SLEEP_WAKEUP_GPIO:
                    case ESP_SLEEP_WAKEUP_EXT1:
                    case ESP_SLEEP_WAKEUP_UNDEFINED:
                    default:
                        rtc_suppress_rom_log();
                        esp_deep_sleep_start();
                }
                vTaskDelay(pdMS_TO_TICKS(2500));

                while(usb_sd_fs_is_storage_in_use_by_host() && is_usb_connected())
                {
                    if(gpio_get_level(GPIO_OK_BTN))
                    {
                        graphic_driver_clear();
                        vTaskDelay(pdMS_TO_TICKS(200));
                        init_logging(false);
                    }

                    else if(gpio_get_level(GPIO_MODE_BTN))
                    {
                        boot_instruction = BOOT_INSTRUCTION_UPDATE_FIRMWARE;
                        esp_sleep_enable_timer_wakeup(0);
                        esp_deep_sleep_start();
                    }
                    vTaskDelay(pdMS_TO_TICKS(50));
                }

                ESP_LOGI(TAG, "Going to sleep from main");
                esp_deep_sleep_start();
                break;
    }




}
