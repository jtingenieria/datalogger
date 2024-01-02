/**
 * @file lv_file_explorer.h
 *
 */

#ifndef LV_ROLLER_FILE_EXPLORER_H
#define LV_ROLLER_FILE_EXPLORER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "lvgl_helpers.h"




/*********************
 *      DEFINES
 *********************/

#define LV_FILE_EXPLORER_PATH_MAX_LEN 128
#define LV_ROLLER_FILE_EXPLORER_MAX_DIR 20

/**********************
 *      TYPEDEFS
 **********************/

/*Data of canvas*/
typedef struct {
    lv_obj_t obj;
    lv_obj_t * cont;
    lv_obj_t * roller;
    lv_obj_t * browser_area;
    lv_obj_t * path_label;
    char * sel_fp;
    char   cur_path[LV_FILE_EXPLORER_PATH_MAX_LEN];
    char   base_path[LV_FILE_EXPLORER_PATH_MAX_LEN];
    int prev_index[LV_ROLLER_FILE_EXPLORER_MAX_DIR];
    int level_count;
} lv_roller_file_explorer_t;

/***********************
 * GLOBAL VARIABLES
 ***********************/


/**********************
 * GLOBAL PROTOTYPES
 **********************/
lv_obj_t * lv_roller_file_explorer_create(lv_obj_t * parent);


/*=====================
 * Getter functions
 *====================*/

/**
 * Get file explorer Selected file
 * @param obj   pointer to a file explorer object
 * @return      pointer to the file explorer selected file
 */
char * lv_roller_file_explorer_get_sel_fn(lv_obj_t * obj);

/**
 * Get file explorer cur path
 * @param obj   pointer to a file explorer object
 * @return      pointer to the file explorer cur path
 */
char * lv_roller_file_explorer_get_cur_path(lv_obj_t * obj);



/**
 * Get file explorer path obj(label)
 * @param obj   pointer to a file explorer object
 * @return      pointer to the file explorer path obj(label)
 */
lv_obj_t * lv_roller_file_explorer_get_roller_obj(lv_obj_t * obj);



/*=====================
 * Other functions
 *====================*/

/**
 * Open a specified path
 * @param obj   pointer to a file explorer object
 * @param dir   pointer to the path
 */
void lv_roller_file_explorer_open_dir(lv_obj_t * obj, char * dir);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SKETCHPAD_H*/
