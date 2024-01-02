/*
 * ui_flash_screen.h
 *
 *  Created on: 19 oct. 2023
 *      Author: juant
 */

#ifndef MAIN_UI_FLASH_SCREEN_H_
#define MAIN_UI_FLASH_SCREEN_H_

#include "ui_flash_screen.h"

void flash_screen_anim();
void flash_screen();

void ui_flash_screen_set_percentage(int32_t v);
void ui_flash_screen_upload_done();
void ui_flash_screen_set_filename(char * filename);

#endif /* MAIN_UI_FLASH_SCREEN_H_ */
