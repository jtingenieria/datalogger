/*
 * graphic_driver.h
 *
 *  Created on: 17 oct. 2023
 *      Author: juant
 */

#ifndef MAIN_GRAPHIC_DRIVER_H_
#define MAIN_GRAPHIC_DRIVER_H_

/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

extern lv_obj_t *ui_main_screen_obj;
extern lv_obj_t *ui_file_explorer_screen_obj;
extern lv_obj_t *ui_flash_screen_obj;


void graphic_driver_main_task(void *pvParameter);
void main_screen_event_handler(lv_event_t * e);
void flash_screen_event_handler(lv_event_t * e);

void graphic_driver_init(void);
void graphic_driver_usb_connected(void);

void graphic_driver_show_text(char * text);
void graphic_driver_show_usb(void);
void graphic_driver_show_recording(bool is_recording);
void graphic_driver_clear(void);

#endif /* MAIN_GRAPHIC_DRIVER_H_ */
