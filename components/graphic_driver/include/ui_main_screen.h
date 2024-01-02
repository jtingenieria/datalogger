/*
 * ui_main_screen.h
 *
 *  Created on: 19 oct. 2023
 *      Author: juant
 */

#ifndef MAIN_UI_MAIN_SCREEN_H_
#define MAIN_UI_MAIN_SCREEN_H_

/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

void main_screen();
lv_obj_t * get_main_screen_obj_keys();

#define UI_MAX_TEXT_LEN 40

enum{
    UI_MAIN_SCREEN_OPTION_CSV=0,
    UI_MAIN_SCREEN_OPTION_STD,
    UI_MAIN_SCREEN_OPTION_BATCH,
    UI_MAIN_SCREEN_OPTION_CONFIG,
    UI_MAIN_SCREEN_OPTION_TOTAL,
};

extern char ui_main_screen_options[UI_MAIN_SCREEN_OPTION_TOTAL][UI_MAX_TEXT_LEN];

#endif /* MAIN_UI_MAIN_SCREEN_H_ */
