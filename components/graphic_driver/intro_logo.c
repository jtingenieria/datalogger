#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_INTRO_LOGO
#define LV_ATTRIBUTE_IMG_INTRO_LOGO
#endif

//https://lvgl.io/tools/imageconverter CF_INDEXED_1_BIT

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_INTRO_LOGO uint8_t intro_logo_map[] = {
    0xff, 0xff, 0xff, 0xff,     /*Color of index 0*/
    0x00, 0x00, 0x00, 0xff,   /*Color of index 1*/

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x7e, 0x0f, 0xff, 0xc7, 0xe0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x7e, 0x0f, 0xff, 0x87, 0xe0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0xfe, 0x1f, 0xff, 0x8f, 0xe0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x80, 0xfe, 0x00, 0xf0, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x0f, 0xc1, 0xff, 0x00, 0xf0, 0x1f, 0xf0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x07, 0xc3, 0xff, 0x01, 0xf0, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1e, 0x07, 0xc3, 0xcf, 0x01, 0xf0, 0x3c, 0xf0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1e, 0x07, 0xc7, 0x8f, 0x01, 0xf0, 0x78, 0xf0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3e, 0x07, 0xc7, 0x8f, 0x81, 0xe0, 0x78, 0xf8, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3e, 0x0f, 0x8f, 0xff, 0x81, 0xe0, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3e, 0x0f, 0x9f, 0xff, 0x83, 0xe1, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3c, 0x3f, 0x1f, 0xff, 0x83, 0xe1, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x3e, 0x07, 0xc3, 0xc3, 0xe0, 0x7c, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x3c, 0x07, 0xc3, 0xc3, 0xc0, 0x7c, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x7c, 0x07, 0xc7, 0xc7, 0xc0, 0x7c, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0f, 0x80, 0x07, 0xe0, 0x01, 0xf8, 0x01, 0xf8, 0x1f, 0xff, 0x1f, 0xf8, 0x00, 0x00,
    0x00, 0x00, 0x0f, 0x00, 0x1f, 0xf8, 0x07, 0xff, 0x07, 0xff, 0x1f, 0xfe, 0x1f, 0xfe, 0x00, 0x00,
    0x00, 0x00, 0x0f, 0x00, 0x7f, 0xfc, 0x1f, 0xff, 0x9f, 0xff, 0x9f, 0xfe, 0x1f, 0xff, 0x00, 0x00,
    0x00, 0x00, 0x1f, 0x00, 0xfe, 0xfe, 0x3f, 0x9e, 0x3f, 0x9e, 0x3e, 0x00, 0x3e, 0x3f, 0x00, 0x00,
    0x00, 0x00, 0x1f, 0x00, 0xf8, 0x3e, 0x3e, 0x04, 0x3e, 0x04, 0x3e, 0x00, 0x3e, 0x1f, 0x00, 0x00,
    0x00, 0x00, 0x1f, 0x01, 0xf0, 0x1f, 0x7c, 0x00, 0x7c, 0x00, 0x3e, 0x00, 0x3e, 0x1f, 0x00, 0x00,
    0x00, 0x00, 0x1e, 0x01, 0xf0, 0x1f, 0x7c, 0x00, 0x7c, 0x00, 0x3f, 0xfc, 0x3c, 0x1f, 0x00, 0x00,
    0x00, 0x00, 0x1e, 0x01, 0xf0, 0x1f, 0x7c, 0x0f, 0x7c, 0x0f, 0x3f, 0xf8, 0x3c, 0x3e, 0x00, 0x00,
    0x00, 0x00, 0x3e, 0x01, 0xf0, 0x1f, 0x7c, 0x0f, 0x7c, 0x0f, 0x7f, 0xf8, 0x7f, 0xfe, 0x00, 0x00,
    0x00, 0x00, 0x3e, 0x01, 0xf0, 0x3e, 0x7c, 0x0f, 0x7c, 0x0f, 0x7c, 0x00, 0x7f, 0xf8, 0x00, 0x00,
    0x00, 0x00, 0x3e, 0x01, 0xf8, 0x3e, 0x7e, 0x1e, 0x7e, 0x1e, 0x7c, 0x00, 0x7f, 0xf0, 0x00, 0x00,
    0x00, 0x00, 0x3c, 0x00, 0xfc, 0xfc, 0x3f, 0x3e, 0x3f, 0x3e, 0x78, 0x00, 0x78, 0xf8, 0x00, 0x00,
    0x00, 0x00, 0x3f, 0xf8, 0xff, 0xf8, 0x3f, 0xfe, 0x3f, 0xfe, 0x7f, 0xf8, 0x78, 0xf8, 0x00, 0x00,
    0x00, 0x00, 0x7f, 0xf0, 0x7f, 0xf0, 0x1f, 0xfc, 0x1f, 0xfc, 0xff, 0xf8, 0xf8, 0x7c, 0x00, 0x00,
    0x00, 0x00, 0x7f, 0xf0, 0x1f, 0xc0, 0x07, 0xf0, 0x07, 0xf0, 0xff, 0xf8, 0xf8, 0x7c, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0xf7, 0xf8, 0x66, 0x0c, 0x1f, 0x1f, 0xec, 0x19, 0x9f, 0xef, 0xe3, 0x03, 0x80, 0x00,
    0x00, 0x01, 0xf7, 0xf8, 0x67, 0x0c, 0x3f, 0xdf, 0xee, 0x19, 0x9f, 0xef, 0xf3, 0x03, 0x80, 0x00,
    0x00, 0x00, 0x30, 0xc0, 0x67, 0x8c, 0x70, 0x98, 0x0f, 0x19, 0x98, 0x0c, 0x33, 0x07, 0xc0, 0x00,
    0x00, 0x00, 0x30, 0xc0, 0x67, 0xcc, 0x60, 0x18, 0x0f, 0x99, 0x98, 0x0c, 0x33, 0x06, 0xc0, 0x00,
    0x00, 0x00, 0x30, 0xc0, 0x66, 0xcc, 0xe0, 0x1f, 0xcd, 0x99, 0x9f, 0xcc, 0x33, 0x0c, 0xc0, 0x00,
    0x00, 0x00, 0x30, 0xc0, 0x66, 0x6c, 0xe0, 0xdf, 0xcc, 0xd9, 0x9f, 0xcf, 0xf3, 0x0c, 0x60, 0x00,
    0x00, 0x00, 0x30, 0xc0, 0x66, 0x7c, 0x60, 0xd8, 0x0c, 0xf9, 0x98, 0x0f, 0xe3, 0x1f, 0xe0, 0x00,
    0x00, 0x01, 0x30, 0xc0, 0x66, 0x3c, 0x70, 0xd8, 0x0c, 0x79, 0x98, 0x0c, 0x63, 0x1f, 0xf0, 0x00,
    0x00, 0x03, 0xf0, 0xc0, 0x66, 0x1c, 0x3f, 0xdf, 0xec, 0x39, 0x9f, 0xec, 0x73, 0x18, 0x30, 0x00,
    0x00, 0x01, 0xe0, 0xc0, 0x66, 0x0c, 0x1f, 0x1f, 0xec, 0x19, 0x9f, 0xec, 0x3b, 0x30, 0x38, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const lv_img_dsc_t intro_logo = {
  .header.cf = LV_IMG_CF_INDEXED_1BIT,
  .header.always_zero = 0,
  .header.reserved = 0,
  .header.w = 128,
  .header.h = 64,
  .data_size = 1032,
  .data = intro_logo_map,
};
