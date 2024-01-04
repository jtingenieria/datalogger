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

#include "graphic_driver.h"
#include "usb_console.h"
#include "usb_sd_fs.h"
#include "logger.h"

static const char *TAG = "datalogger";

static RTC_DATA_ATTR struct timeval sleep_enter_time;

enum{
    DATALOGGER_IS_NOT_LOGGING = 0,
    DATALOGGER_IS_LOGGING
};

static RTC_DATA_ATTR int datalogger_logging_status = DATALOGGER_IS_NOT_LOGGING;
static RTC_DATA_ATTR bool date_is_acquired = false;


const int ext_wakeup_pin_0 = 6;

static void enable_btn_wakeup(void)
{
    printf("Enabling EXT0 wakeup on pin GPIO%d\n", ext_wakeup_pin_0);
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(ext_wakeup_pin_0, 1));
    ESP_ERROR_CHECK(rtc_gpio_pullup_dis(ext_wakeup_pin_0));
    ESP_ERROR_CHECK(rtc_gpio_pulldown_en(ext_wakeup_pin_0));
}

static void deep_sleep_register_rtc_timer_wakeup(void)
{
    const int wakeup_time_sec = 10;
    printf("Enabling timer wakeup, %ds\n", wakeup_time_sec);
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(wakeup_time_sec * 1000000));
}


#define USB_DETECT_GPIO 8
#define BUS_ENABLE_PIN 2

static bool is_usb_connected(void)
{

    return gpio_get_level(USB_DETECT_GPIO);
}

static void start_btn_timer(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    if(gpio_get_level(ext_wakeup_pin_0)){
        datalogger_logging_status = datalogger_logging_status ? 0 : 1;
    }
    ESP_LOGI(TAG, "Now logging is %d", datalogger_logging_status);
    while(gpio_get_level(ext_wakeup_pin_0)){
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelay(pdMS_TO_TICKS(50));
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
}

static void acquire_time(void)
{

}

void app_main(void)
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    gpio_set_direction(USB_DETECT_GPIO, GPIO_MODE_INPUT);

    enable_bus(true);
    vTaskDelay(pdMS_TO_TICKS(50));
    enable_btn_wakeup();

    gpio_set_direction(19, GPIO_MODE_INPUT);
    gpio_set_direction(20, GPIO_MODE_INPUT);

    switch (esp_sleep_get_wakeup_cause())
    {
        case ESP_SLEEP_WAKEUP_TIMER:
//            printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
            if(datalogger_logging_status == DATALOGGER_IS_LOGGING)
            {
                ESP_LOGI(TAG, "About to log");
                usb_sd_fs_init_card();
                logger_init();
                logger_log();
                ESP_LOGI(TAG, "Log done");
                deep_sleep_register_rtc_timer_wakeup();
                esp_deep_sleep_start();
            }
            else
            {
                ESP_LOGI(TAG, "Not logging");
                esp_deep_sleep_start();
            }
            break;

        case ESP_SLEEP_WAKEUP_EXT0:
            if(datalogger_logging_status == DATALOGGER_IS_LOGGING)
            {
                ESP_LOGI(TAG, "Want to see sensors");
                graphic_driver_init();
                start_btn_timer();
                if(datalogger_logging_status == DATALOGGER_IS_LOGGING)
                {
                   display_sensors();
                   deep_sleep_register_rtc_timer_wakeup();
                   esp_deep_sleep_start();
                }
                else
                {
                    graphic_driver_show_text("Stopped recording data");
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    //esp_sleep_enable_timer_wakeup(0);
                    esp_deep_sleep_start();
                }

            }
            else if(is_usb_connected())
            {
                ESP_LOGI(TAG, "Connect USB");
                usb_sd_fs_init_card();
                usb_sd_fs_init_usb();
//                usb_console_init();
                graphic_driver_init();
                graphic_driver_usb_connected();
            }
            else
            {
                ESP_LOGI(TAG, "Init logging");
                graphic_driver_init();
                vTaskDelay(pdMS_TO_TICKS(1000));
                graphic_driver_show_text("Recording data.");
                datalogger_logging_status = DATALOGGER_IS_LOGGING;
                acquire_time();
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_sleep_enable_timer_wakeup(0);
                esp_deep_sleep_start();
            }
            break;

        case ESP_SLEEP_WAKEUP_GPIO:
        case ESP_SLEEP_WAKEUP_EXT1:
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            esp_deep_sleep_start();
    }
    vTaskDelay(pdMS_TO_TICKS(2500));

    while(usb_sd_fs_is_storage_in_use_by_host() && is_usb_connected()){
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "Going to sleep from main");
    esp_deep_sleep_start();


}
