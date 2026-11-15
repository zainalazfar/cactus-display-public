/*******************************************************************************
 * Size: 14 px (~10% reduced from 16px)
 * Bpp: 1
 * Digits: 0-9 and space for tuner frequency display
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef PRESS_START_2P_14
#define PRESS_START_2P_14 1
#endif

#if PRESS_START_2P_14

/*-----------------
 *    BITMAPS
 *----------------*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    0x00, 0x1f, 0x81, 0xf8, 0x31, 0xcf, 0x1f, 0xf0, 0xff, 0x0f, 0xf0, 0xff,
    0x0f, 0xf8, 0xf3, 0x8c, 0x1f, 0x81, 0xf8, 0x0f, 0x00, 0xf0, 0x3f, 0x03,
    0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0x0f, 0x00, 0xf0, 0xff, 0xff,
    0xff, 0x3f, 0xc7, 0xfe, 0xf0, 0xfe, 0x1f, 0x01, 0xf1, 0xfe, 0x1f, 0xc3,
    0xf8, 0xff, 0x0f, 0x80, 0xff, 0xff, 0xff, 0x3f, 0xf3, 0xff, 0x01, 0xc0,
    0x7c, 0x07, 0x81, 0xfc, 0x1f, 0xe0, 0x0f, 0xe0, 0xff, 0x0f, 0x7f, 0xe3,
    0xfc, 0x07, 0xc0, 0x7c, 0x1f, 0xc3, 0xfc, 0x39, 0xcf, 0x1c, 0xf1, 0xef,
    0xff, 0xff, 0xf0, 0x1c, 0x01, 0xc0, 0x1c, 0xff, 0xcf, 0xfc, 0xf0, 0x0f,
    0xfc, 0xff, 0xc0, 0x0f, 0x00, 0xf0, 0x0f, 0xe0, 0xff, 0x0f, 0x7f, 0xe3,
    0xfc, 0x1f, 0xc1, 0xfc, 0x38, 0x0f, 0x80, 0xf0, 0x0f, 0xfc, 0xff, 0xef,
    0x0f, 0xf0, 0xff, 0x0f, 0x7f, 0xe3, 0xfc, 0xff, 0xff, 0xff, 0xf0, 0xfe,
    0x1f, 0x01, 0xc0, 0x78, 0x07, 0x81, 0xe0, 0x1e, 0x01, 0xe0, 0x1e, 0x01,
    0xe0, 0x3f, 0x87, 0xf8, 0xf0, 0xcf, 0x8c, 0xf8, 0xc7, 0xf8, 0x3f, 0x8c,
    0x7f, 0xc7, 0xfc, 0x0f, 0x7f, 0xe3, 0xfc, 0x3f, 0xc7, 0xfe, 0xf0, 0xff,
    0x0f, 0xf0, 0xf7, 0xff, 0x3f, 0xf0, 0x0f, 0x01, 0xf0, 0x1c, 0x3f, 0x83,
    0xf8,
};

/*-----------------
 *    GLYPH DSC
 *----------------*/
static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 224, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 19, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 37, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 55, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 73, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 91, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 109, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 127, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 145, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 163, .adv_w = 224, .box_w = 12, .box_h = 12, .ofs_x = 1, .ofs_y = 1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 26, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/
#if LVGL_VERSION_MAJOR == 8
static lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};

/*-----------------
 *  PUBLIC FONT
 *----------------*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t press_start_2p_14 = {
#else
lv_font_t press_start_2p_14 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,
    .line_height = 14,
    .base_line = 0,
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};

#endif /*#if PRESS_START_2P_14*/
