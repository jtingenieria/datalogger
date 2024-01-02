/*
 * lv_app_helper_functions.h
 *
 *  Created on: 19 oct. 2023
 *      Author: juant
 */

#ifndef MAIN_LV_APP_HELPER_FUNCTIONS_H_
#define MAIN_LV_APP_HELPER_FUNCTIONS_H_


/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"

#ifndef MY_DISP_HOR_RES
#define MY_DISP_HOR_RES 128
#endif

#ifndef MY_DISP_VER_RES
#define MY_DISP_VER_RES 64
#endif

void keypad_2_lv(lv_indev_drv_t * drv, lv_indev_data_t*data);

#endif /* MAIN_LV_APP_HELPER_FUNCTIONS_H_ */
