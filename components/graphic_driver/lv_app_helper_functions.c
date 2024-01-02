/*
 * lv_app_helper_functions.c
 *
 *  Created on: 19 oct. 2023
 *      Author: juant
 */

#include "lv_app_helper_functions.h"

/**
 * Get current key
 * @return key
 */
static uint32_t keypad_get_key(void)
{

    return 0;
}


void keypad_2_lv(lv_indev_drv_t * drv, lv_indev_data_t*data){
	 static uint32_t last_key = 0;

	    /*Get whether the a key is pressed and save the pressed key*/
	    uint32_t act_key = keypad_get_key();
	    if(act_key != 0) {
	        data->state = LV_INDEV_STATE_PR;

	        /*Translate the keys to LVGL control characters according to your key definitions*/
	        switch(act_key) {
	            case 1:
	                act_key = LV_KEY_ESC;
	                break;
	            case 2:
	                act_key = LV_KEY_PREV;
	                break;
	            case 3:
	                act_key = LV_KEY_UP;
	                break;
	            case 4:
	                act_key = LV_KEY_DOWN;
	                break;
	            case 5:
	                act_key = LV_KEY_ENTER;
	                break;
	        }

	        last_key = act_key;
	    }
	    else {
	        data->state = LV_INDEV_STATE_REL;
	    }

	    data->key = last_key;
}

