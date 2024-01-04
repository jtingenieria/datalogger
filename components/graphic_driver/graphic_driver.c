/*
 * graphic_driver.c
 *
 *  Created on: 17 oct. 2023
 *      Author: juant
 */
#include <sys/param.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#include "graphic_driver.h"
#include "lv_roller_file_explorer.h"
#include "lv_app_helper_functions.h"


/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"

static const char *TAG="graphic_driver";

#define LV_TICK_PERIOD_MS 1
#define MOUNT_POINT "/sdcard"


static void lv_tick_task(void *arg);



static lv_disp_drv_t disp_drv;
lv_indev_t * indev_keypad;
SemaphoreHandle_t xGuiSemaphore;
lv_group_t * group;
lv_obj_t * file_explorer;

lv_obj_t *ui_main_screen_obj = NULL;
lv_obj_t *ui_file_explorer_screen_obj = NULL;
lv_obj_t *ui_flash_screen_obj = NULL;


extern const lv_img_dsc_t intro_logo;
extern const lv_img_dsc_t usb_icon;

static bool usb_is_connected = false;

void indev_group()
{
	if(group != NULL) lv_group_remove_all_objs(group);
	group = lv_group_create();
	lv_indev_set_group(indev_keypad, group);
}


static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}



static void my_timer(lv_timer_t * timer)
{
    ESP_LOGI(TAG, "Timerr");
    lv_obj_clean(lv_scr_act());
    if(usb_is_connected)
    {
        lv_obj_t * icon = lv_img_create(lv_scr_act());
        lv_img_set_src(icon, &usb_icon);
    }
}

void graphic_driver_main_task(void *pvParameter)
{

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    /* Initialize SPI or I2C bus used by the drivers */
    lvgl_driver_init();

    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    /* Use double buffered when not working with monochrome displays */
    #ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
		lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
		assert(buf2 != NULL);
    #else
    	static lv_color_t *buf2 = NULL;
    #endif

    static lv_disp_draw_buf_t disp_buf;
    // static lv_color_t buf_1[MY_DISP_HOR_RES * 10];
    uint32_t size_in_px = DISP_BUF_SIZE;

    #if defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_IL3820 || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_JD79653A || defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_UC8151D ||    defined CONFIG_LV_TFT_DISPLAY_CONTROLLER_SSD1306

    /* Actual size in pixels, not bytes. */
    size_in_px *= 8;
    #endif

    /* Initialize the working buffer depending on the selected display.

    NOTE: buf2 == NULL when using monochrome displays. */
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, MY_DISP_HOR_RES * MY_DISP_VER_RES);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = disp_driver_flush;

    #if defined CONFIG_DISPLAY_ORIENTATION_PORTRAIT || defined CONFIG_DISPLAY_ORIENTATION_PORTRAIT_INVERTED
    disp_drv.rotated = 1;
    TY
    #endif

    /* When using a monochrome display we need to register the callbacks:

    rounder_cb
    set_px_cb */
    #ifdef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    disp_drv.rounder_cb = disp_driver_rounder;
    disp_drv.set_px_cb = disp_driver_set_px;
    #endif
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /* Register an input device when enabled on the menuconfig */
    #if CONFIG_LV_TOUCH_CONTROLLER != TOUCH_CONTROLLER_NONE
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.read_cb = touch_driver_read;
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    lv_indev_drv_register(&indev_drv);
    #endif

    /* Create and start a periodic timer interrupt to call lv_tick_inc */
    const esp_timer_create_args_t periodic_timer_args = {
    .callback = &lv_tick_task, .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(
    esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));


    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_KEYPAD;
	indev_drv.read_cb = keypad_2_lv;
	indev_keypad = lv_indev_drv_register(&indev_drv);

	indev_group();


	lv_obj_t * icon = lv_img_create(lv_scr_act());
	/*From variable*/
	lv_img_set_src(icon, &intro_logo);

	lv_timer_t * timer = lv_timer_create(my_timer, 1500, NULL);

	lv_timer_set_repeat_count(timer, 1);

	//lv_timer_ready(timer);

    while (1) {

        vTaskDelay(pdMS_TO_TICKS(5));
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }


    free(buf1);
#ifndef CONFIG_LV_TFT_DISPLAY_MONOCHROME
    free(buf2);
#endif
    vTaskDelete(NULL);
}

void graphic_driver_init(void)
{
    xTaskCreatePinnedToCore(graphic_driver_main_task, "gui", 4096*2, NULL, 0, NULL, 1);
}

void graphic_driver_usb_connected(void)
{
    usb_is_connected = true;
}
