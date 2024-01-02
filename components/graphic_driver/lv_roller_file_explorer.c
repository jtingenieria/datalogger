/**
 * @file lv_file_explorer.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_roller_file_explorer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include <string.h>

static const char *TAG="roller_file_explorer";


/*********************
 *      DEFINES
 *********************/
#define MY_CLASS &lv_roller_file_explorer_class

#define FILE_EXPLORER_HEAD_HEIGHT               (60)
#define FILE_EXPLORER_QUICK_ACCESS_AREA_WIDTH   (22)
#define FILE_EXPLORER_BROWER_AREA_WIDTH         (100 - FILE_EXPLORER_QUICK_ACCESS_AREA_WIDTH)



/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void lv_roller_file_explorer_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_roller_file_explorer_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

static void browser_file_event_handler(lv_event_t * e);

static void init_style(lv_obj_t * obj);
static void show_dir(lv_obj_t * obj, char * path);
static void strip_ext(char * dir);
static void sort_table_items(lv_obj_t * tb, int16_t lo, int16_t hi);
static void exch_table_item(lv_obj_t * tb, int16_t i, int16_t j);
static bool is_begin_with(const char * str1, const char * str2);
static bool is_end_with(const char * str1, const char * str2);

/**********************
 *  STATIC VARIABLES
 **********************/
const lv_obj_class_t lv_roller_file_explorer_class = {
    .constructor_cb = lv_roller_file_explorer_constructor,
    .destructor_cb  = lv_roller_file_explorer_destructor,
    .width_def      = LV_SIZE_CONTENT,
    .height_def     = LV_SIZE_CONTENT,
    .instance_size  = sizeof(lv_roller_file_explorer_t),
    .base_class     = &lv_obj_class
};

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_roller_file_explorer_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

char * lv_roller_file_explorer_get_sel_fn(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;

    return explorer->sel_fp;
}

char * lv_roller_file_explorer_get_cur_path(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);

    lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;

    return explorer->cur_path;
}


void lv_roller_file_explorer_open_dir(lv_obj_t * obj, char * dir)
{
    lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;
    strcpy(explorer->base_path,dir);
    lv_memset_00(explorer->prev_index,sizeof(explorer->prev_index));
    explorer->level_count=0;
    show_dir(obj, dir);
}

lv_obj_t * lv_roller_file_explorer_get_roller_obj(lv_obj_t * obj){
	lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;
	return explorer->roller;
}

static void init_style(lv_obj_t * obj)
{
    lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;

//    static lv_style_t cont_style;
//    lv_style_init(&cont_style);
//    lv_style_set_radius(&cont_style, 0);
//    lv_style_set_bg_opa(&cont_style, LV_OPA_0);
//    lv_style_set_border_width(&cont_style, 0);
//    lv_style_set_outline_width(&cont_style, 0);
//    lv_style_set_pad_column(&cont_style, 0);
//    lv_style_set_pad_row(&cont_style, 0);
//    //lv_style_set_flex_flow(&cont_style, LV_FLEX_FLOW_ROW);
//    lv_style_set_pad_all(&cont_style, 0);
//    lv_style_set_layout(&cont_style, LV_LAYOUT_FLEX);


    static lv_style_t brower_area_style;
    lv_style_init(&brower_area_style);
    lv_style_set_pad_all(&brower_area_style, 0);
    lv_style_set_radius(&brower_area_style, 0);
    lv_style_set_border_width(&brower_area_style, 0);
    lv_style_set_outline_width(&brower_area_style, 0);
    //lv_style_set_bg_color(&brower_area_style, lv_color_hex(0xffffff));

    lv_obj_set_style_anim_time(explorer->roller, 250, LV_PART_MAIN | LV_STATE_DEFAULT);
    //lv_obj_set_height( ui_Roller1, 64);
    //lv_obj_set_width( ui_Roller1, 128);  /// 1
    lv_obj_set_align( explorer->roller, LV_ALIGN_CENTER );
    lv_obj_set_style_text_align(explorer->roller, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(explorer->roller, 10, LV_PART_MAIN| LV_STATE_DEFAULT);
    //lv_obj_set_style_radius(ui_Roller1, 6, LV_PART_MAIN| LV_STATE_DEFAULT);
    //lv_obj_set_style_bg_color(ui_Roller1, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(explorer->roller, 0, LV_PART_MAIN| LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(explorer->roller, 0, LV_PART_MAIN| LV_STATE_DEFAULT);

    lv_obj_set_style_text_color(explorer->roller, lv_color_hex(0), LV_PART_SELECTED | LV_STATE_DEFAULT );
    lv_obj_set_style_text_opa(explorer->roller, 255, LV_PART_SELECTED| LV_STATE_DEFAULT);
    //lv_obj_set_style_text_letter_space(explorer->roller, -1, LV_PART_SELECTED| LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(explorer->roller, 10, LV_PART_SELECTED| LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(explorer->roller, LV_TEXT_ALIGN_CENTER, LV_PART_SELECTED| LV_STATE_DEFAULT);
    lv_obj_set_style_radius(explorer->roller, 8, LV_PART_SELECTED| LV_STATE_DEFAULT);
    //lv_obj_set_style_bg_color(ui_Roller1, lv_color_hex(0xFFFFFF), LV_PART_SELECTED | LV_STATE_DEFAULT );
    lv_obj_set_style_bg_opa(explorer->roller, 0, LV_PART_SELECTED| LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(explorer->roller, lv_color_hex(0), LV_PART_SELECTED | LV_STATE_DEFAULT );
    lv_obj_set_style_border_opa(explorer->roller, 255, LV_PART_SELECTED| LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(explorer->roller, 1, LV_PART_SELECTED| LV_STATE_DEFAULT);

    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xffffff), 0);

    //lv_obj_add_style(explorer->cont, &cont_style, 0);
    lv_obj_add_style(explorer->browser_area, &brower_area_style, 0);
}


static void lv_roller_file_explorer_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;

    lv_memset_00(explorer->cur_path, sizeof(explorer->cur_path));
    lv_obj_set_size(obj, LV_PCT(100), LV_PCT(100));
    lv_obj_set_align(obj, LV_ALIGN_CENTER);

    //explorer->cont = lv_obj_create(obj);
    //lv_obj_set_size(explorer->cont, LV_PCT(100), LV_PCT(100));

    explorer->browser_area = lv_obj_create(obj);//lv_obj_create(explorer->cont);

    lv_obj_set_size(explorer->browser_area, LV_PCT(100), LV_PCT(100));
    lv_obj_set_align(explorer->browser_area, LV_ALIGN_CENTER);

    explorer->roller = lv_roller_create(explorer->browser_area);
    lv_obj_set_size(explorer->roller, LV_PCT(100), LV_PCT(100));
    lv_obj_set_align(explorer->roller, LV_ALIGN_CENTER);
    lv_roller_set_options(explorer->roller,"",LV_ROLLER_MODE_NORMAL);

    lv_obj_add_event_cb(explorer->roller, browser_file_event_handler, LV_EVENT_ALL, obj);

    init_style(obj);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_roller_file_explorer_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");
}



static void browser_file_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_user_data(e);
    lv_obj_t * roller = lv_event_get_target(e);

    bool subdir=0;

    static char file_name[LV_FILE_EXPLORER_PATH_MAX_LEN];
    static char str_fn[LV_FILE_EXPLORER_PATH_MAX_LEN];

    lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;

    if(code == LV_EVENT_VALUE_CHANGED) {
        memset(file_name, 0, sizeof(file_name));
        memset(str_fn, 0, sizeof(str_fn));
        lv_roller_get_selected_str(roller, str_fn, LV_FILE_EXPLORER_PATH_MAX_LEN);
        ESP_LOGI(TAG, "str_fn is: %s",str_fn);

        if((strcmp(str_fn, ".") == 0))  return;
        if((strcmp(str_fn, "") == 0))  return;

        if((strcmp(str_fn, "..") == 0) && (strlen(explorer->cur_path) > strlen(explorer->base_path)+1))
        {
        	//ESP_LOGI(TAG,"CUR: %s \t BASE: %s",explorer->cur_path,explorer->base_path);
            strip_ext(explorer->cur_path);
            strip_ext(explorer->cur_path);
            lv_snprintf(file_name, sizeof(file_name), "%s", explorer->cur_path);
        }
        else
        {
            if(strcmp(str_fn, "..") != 0){
                lv_snprintf(file_name, sizeof(file_name), "%s%s", explorer->cur_path, str_fn);
                subdir=true;
            }
        }
        //ESP_LOGI(TAG, "file_name is: %s",file_name);

        lv_fs_dir_t dir;
        lv_fs_res_t res;
        res = lv_fs_dir_open(&dir, file_name);
        if(res == LV_FS_RES_OK) {
            lv_fs_dir_close(&dir);

            if(subdir){
            	explorer->prev_index[explorer->level_count]=lv_roller_get_selected(roller);
            	if(explorer->level_count<LV_ROLLER_FILE_EXPLORER_MAX_DIR) explorer->level_count++;
            }
            else{
            	if(explorer->level_count>0)explorer->level_count--;
            }
            show_dir(obj, file_name);
            //ESP_LOGI(TAG,"Change screen");
        }
        else {
            if(strcmp(str_fn, "..") != 0) {
                explorer->sel_fp = file_name;
                lv_event_send(obj, LV_EVENT_READY, NULL);
                //ESP_LOGI(TAG,"Value change");
            }
        }
    }

    else if(code==LV_EVENT_KEY){
    		lv_indev_t* i = lv_indev_get_act();
    		uint32_t key = 0;
    		if (i) key = lv_indev_get_key(i);
    		if (key == LV_KEY_ESC){
				memset(file_name, 0, sizeof(file_name));
				if(strlen(explorer->cur_path) > strlen(explorer->base_path)+1)
				{
					strip_ext(explorer->cur_path);
					strip_ext(explorer->cur_path);
					lv_snprintf(file_name, sizeof(file_name), "%s", explorer->cur_path);
				}
				else{
					lv_event_send(obj, LV_EVENT_CANCEL, NULL);
				}
				lv_fs_dir_t dir;
				lv_fs_res_t res;
				res = lv_fs_dir_open(&dir, file_name);
				//ESP_LOGI(TAG, "res is: %d",res);
				if(res == LV_FS_RES_OK) {
					lv_fs_dir_close(&dir);
					if(explorer->level_count>0)explorer->level_count--;
					show_dir(obj, file_name);
					//ESP_LOGI(TAG,"Change screen");
				}
    		}

    }

}


static void show_dir(lv_obj_t * obj, char * path)
{
    lv_roller_file_explorer_t * explorer = (lv_roller_file_explorer_t *)obj;

    char fn[LV_FILE_EXPLORER_PATH_MAX_LEN];

    char string_files[13*51]=""; //8.3 file name
    memset(string_files, 0, sizeof(string_files));

    uint16_t index = 0;
    lv_fs_dir_t dir;
    lv_fs_res_t res;

    res = lv_fs_dir_open(&dir, path);
    if(res != LV_FS_RES_OK) {
        LV_LOG_USER("Open dir error %d!", res);
        ESP_LOGE(TAG,"Open dir error %d!", res);
        return;
    }

    while(1) {
    	if(index>50) break;
        res = lv_fs_dir_read(&dir, fn);
        if(res != LV_FS_RES_OK) {
            LV_LOG_USER("Driver, file or directory is not exists %d!", res);
            ESP_LOGE(TAG,"Driver, file or directory is not exists %d!", res);
            break;
        }

        /*fn is empty, if not more files to read*/
        if(strlen(fn) == 0) {
            LV_LOG_USER("Not more files to read!");
            //ESP_LOGI(TAG,"Not more files to read!");
            break;
        }

        if((is_end_with(fn , ".") == true) || (is_end_with(fn , "..") == true))
        {
            continue;
        }
        else if(fn[0] == '/') {/*is dir*/
            strncat(string_files,fn+1,strlen(string_files)-2);
        }
        else {
        	strncat(string_files,fn,strlen(string_files)-2);
        }
        strncat(string_files,"\n",strlen(string_files)-1);
        index++;

    }
    if(index==0) {
    	sprintf(string_files,"..");
    	//ESP_LOGI(TAG, "Roller in: %s", string_files);
    }
    else strncat(string_files,"..\0",strlen(string_files));
    lv_fs_dir_close(&dir);

    //ESP_LOGI(TAG, "Roller: %s", string_files);
    lv_roller_set_options(explorer->roller,string_files,LV_ROLLER_MODE_NORMAL);
    if(explorer->level_count<LV_ROLLER_FILE_EXPLORER_MAX_DIR){
    	lv_roller_set_selected(explorer->roller, explorer->prev_index[explorer->level_count], LV_ANIM_OFF);
    }

    lv_memset_00(explorer->cur_path, sizeof(explorer->cur_path));
    strcpy(explorer->cur_path, path);

    size_t cur_path_len = strlen(explorer->cur_path);
    if((*((explorer->cur_path) + cur_path_len) != '/') && (cur_path_len < LV_FILE_EXPLORER_PATH_MAX_LEN)) {
        *((explorer->cur_path) + cur_path_len) = '/';
    }
}


static bool is_begin_with(const char * str1, const char * str2)
{
    if(str1 == NULL || str2 == NULL)
        return false;

    uint16_t len1 = strlen(str1);
    uint16_t len2 = strlen(str2);
    if((len1 < len2) || (len1 == 0 || len2 == 0))
        return false;

    uint16_t i = 0;
    char * p = str2;
    while(*p != '\0')
    {
        if(*p != str1[i])
            return false;

        p++;
        i++;
    }

    return true;
}



static bool is_end_with(const char * str1, const char * str2)
{
    if(str1 == NULL || str2 == NULL)
        return false;

    uint16_t len1 = strlen(str1);
    uint16_t len2 = strlen(str2);
    if((len1 < len2) || (len1 == 0 || len2 == 0))
        return false;

    while(len2 >= 1)
    {
        if(str2[len2 - 1] != str1[len1 - 1])
            return false;

        len2--;
        len1--;
    }

    return true;
}

static void strip_ext(char *dir)
{
    char *end = dir + strlen(dir);

    while (end >= dir && *end != '/') {
        --end;
    }

    if (end > dir) {
        *end = '\0';
    }
    else if (end == dir) {
        *(end+1) = '\0';
    }

}

