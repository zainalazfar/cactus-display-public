/*******************************************************************************
 * Size: 8x11 px (Retro Tall Pixel Font)
 * Bpp: 1
 * Font: PressStart2P-Regular.ttf
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef PRESS_START_2P_10
#define PRESS_START_2P_10 1
#endif

#if PRESS_START_2P_10

/*-----------------
 *    BITMAPS
 *----------------*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    0x00, 0xff, 0xff, 0xb0, 0xd8, 0xde, 0xf7, 0xbd, 0x80, 0x6d, 0xff, 0xfb,
    0x66, 0xcd, 0x9b, 0x7f, 0xfe, 0xd8, 0x10, 0xf9, 0xf6, 0x8f, 0xcf, 0xc5,
    0xfe, 0xfc, 0x20, 0x63, 0xce, 0x96, 0x4d, 0x86, 0xc9, 0xa5, 0xcf, 0x18,
    0x71, 0xf3, 0x66, 0xc7, 0x1b, 0x76, 0xe6, 0x7e, 0xfc, 0xff, 0xc0, 0x37,
    0x6c, 0xcc, 0xcc, 0x73, 0xce, 0x63, 0x33, 0x33, 0xec, 0x6c, 0x70, 0xe7,
    0xff, 0xe7, 0x1b, 0x36, 0x30, 0xc3, 0x3f, 0xfc, 0xc3, 0x0c, 0x6d, 0xec,
    0xff, 0xf0, 0xfc, 0x02, 0x0c, 0x10, 0x41, 0x86, 0x08, 0x20, 0xc1, 0x00,
    0x38, 0xf9, 0x36, 0x3c, 0x78, 0xf1, 0xe3, 0x7c, 0x70, 0x31, 0xc7, 0x0c,
    0x30, 0xc3, 0x0c, 0xff, 0xf0, 0x7d, 0xff, 0x18, 0x73, 0xcf, 0x9e, 0x70,
    0xff, 0xfc, 0x7e, 0xfc, 0x30, 0xc3, 0xc7, 0xc1, 0xe3, 0xfe, 0xf8, 0x1c,
    0x78, 0xf3, 0x6c, 0xdf, 0xff, 0x86, 0x0c, 0x18, 0xfd, 0xfb, 0x07, 0xff,
    0xe0, 0xc1, 0xe3, 0xfe, 0xf8, 0x3c, 0xf9, 0x86, 0x0f, 0xdf, 0xb1, 0xe3,
    0xfe, 0xf8, 0xff, 0xff, 0x18, 0xe1, 0x87, 0x0c, 0x18, 0x30, 0x60, 0x79,
    0xfb, 0x17, 0x27, 0x9f, 0xe7, 0xc3, 0xfc, 0xf8, 0x7d, 0xff, 0x1e, 0x37,
    0xef, 0xc1, 0x86, 0x7c, 0xf0, 0xfc, 0x3f, 0x6d, 0x86, 0xde, 0xc0, 0x19,
    0xcc, 0xcc, 0x31, 0x8c, 0x18, 0xc0, 0xff, 0xfc, 0x07, 0xff, 0xe0, 0xc7,
    0x18, 0x61, 0x98, 0xc6, 0xc6, 0x00, 0x7d, 0xff, 0xfe, 0x30, 0xc7, 0x8e,
    0x00, 0x38, 0x70, 0x7d, 0xfe, 0x0d, 0xdb, 0xb7, 0xef, 0xc0, 0xfc, 0xf8,
    0x38, 0xf9, 0xb6, 0x3c, 0x7f, 0xff, 0xe3, 0xc7, 0x8c, 0xfd, 0xff, 0x1e,
    0x3f, 0xd8, 0xf1, 0xe3, 0xfd, 0xf8, 0x3c, 0xfd, 0x9e, 0x0c, 0x18, 0x30,
    0x73, 0x7e, 0x78, 0xf9, 0xfb, 0x36, 0x3c, 0x78, 0xf1, 0xe3, 0xfd, 0xf0,
    0xff, 0xff, 0x06, 0x0f, 0xdf, 0xb0, 0x60, 0xff, 0xfc, 0xff, 0xff, 0x06,
    0x0f, 0xdf, 0xb0, 0x60, 0xc1, 0x80, 0x3e, 0xfd, 0x86, 0x0c, 0xf9, 0xf1,
    0xe3, 0x7e, 0x7c, 0xc7, 0x8f, 0x1e, 0x3f, 0xff, 0xf1, 0xe3, 0xc7, 0x8c,
    0xff, 0xf3, 0x0c, 0x30, 0xc3, 0x0c, 0xff, 0xf0, 0x06, 0x0c, 0x18, 0x30,
    0x60, 0xc1, 0xe3, 0xfe, 0xf8, 0xc7, 0x9f, 0x36, 0xcf, 0x1f, 0x3e, 0x6e,
    0xdf, 0x9c, 0xc3, 0x0c, 0x30, 0xc3, 0x0c, 0x30, 0xff, 0xf0, 0xc7, 0xdf,
    0xbf, 0xfd, 0x7a, 0xf5, 0xeb, 0xc7, 0x8c, 0xc7, 0xcf, 0x9f, 0xbd, 0xfb,
    0xf3, 0xe3, 0xc7, 0x8c, 0x7d, 0xff, 0x1e, 0x3c, 0x78, 0xf1, 0xe3, 0xfe,
    0xf8, 0xfd, 0xff, 0x1e, 0x3c, 0x7f, 0xff, 0x60, 0xc1, 0x80, 0x7d, 0xff,
    0x1e, 0x3c, 0x7b, 0xf7, 0xe6, 0xfe, 0xf4, 0xfd, 0xff, 0x1e, 0x3c, 0xff,
    0xfe, 0x6e, 0xdf, 0x9c, 0x7d, 0xff, 0x1e, 0x07, 0xcf, 0xc1, 0xe3, 0xfe,
    0xf8, 0xff, 0xf3, 0x0c, 0x30, 0xc3, 0x0c, 0x30, 0xc0, 0xc7, 0x8f, 0x1e,
    0x3c, 0x78, 0xf1, 0xe3, 0xfe, 0xf8, 0xc7, 0x8f, 0x1e, 0x3e, 0xff, 0xdf,
    0x3e, 0x38, 0x20, 0xd7, 0xaf, 0x5e, 0xbd, 0x7f, 0xff, 0xff, 0x44, 0x88,
    0xc7, 0x8f, 0x1b, 0x63, 0x8f, 0x9b, 0x77, 0xc7, 0x8c, 0xcf, 0x3c, 0xf3,
    0xfd, 0xe7, 0x8c, 0x30, 0xc0, 0xff, 0xfc, 0x38, 0xe1, 0xc7, 0x1c, 0x38,
    0xff, 0xfc, 0xff, 0xcc, 0xcc, 0xcc, 0xff, 0x81, 0x81, 0x01, 0x03, 0x03,
    0x02, 0x02, 0x06, 0x04, 0xff, 0x33, 0x33, 0x33, 0xff, 0x77, 0xf6, 0xff,
    0xfc, 0xb4, 0x7c, 0xfd, 0xfb, 0xfc, 0x7f, 0xdf, 0x80, 0xc1, 0x83, 0x07,
    0xef, 0xf8, 0xf1, 0xe3, 0xfe, 0xf8, 0x7f, 0xff, 0x06, 0x0c, 0x1f, 0xdf,
    0x80, 0x06, 0x0c, 0x1b, 0xff, 0xf8, 0xf1, 0xe3, 0xfe, 0xfc, 0x7d, 0xff,
    0xff, 0xfc, 0x1f, 0x9f, 0x00, 0x1c, 0xf3, 0x3f, 0xfc, 0xc3, 0x0c, 0x30,
    0xc0, 0x7f, 0xff, 0x1e, 0x37, 0xef, 0xc1, 0xbe, 0x7c, 0xc1, 0x83, 0x07,
    0xef, 0xf8, 0xf1, 0xe3, 0xc7, 0x8c, 0x30, 0xc0, 0x1c, 0x70, 0xc3, 0x0c,
    0xff, 0xf0, 0x18, 0xc0, 0x73, 0x8c, 0x63, 0x18, 0xfd, 0xe0, 0xc1, 0x83,
    0x06, 0x3c, 0xff, 0xbe, 0x66, 0xcf, 0x8c, 0x71, 0xc3, 0x0c, 0x30, 0xc3,
    0x0c, 0xff, 0xf0, 0xfd, 0xfe, 0xdd, 0xbb, 0x76, 0xed, 0x80, 0xfd, 0xff,
    0x1e, 0x3c, 0x78, 0xf1, 0x80, 0x7d, 0xff, 0x1e, 0x3c, 0x7f, 0xdf, 0x00,
    0xfd, 0xff, 0x1e, 0x3f, 0xdf, 0xb0, 0x60, 0xc0, 0x7f, 0xff, 0x1e, 0x37,
    0xef, 0xc1, 0x83, 0x06, 0xdf, 0xfe, 0x30, 0xc3, 0x0c, 0x00, 0x7d, 0xfb,
    0xf3, 0xe0, 0x7f, 0xff, 0x00, 0x30, 0xc3, 0x3f, 0xfc, 0xc3, 0x0c, 0x30,
    0xc0, 0xc7, 0x8f, 0x1e, 0x3c, 0x7f, 0xdf, 0x80, 0xcf, 0x3c, 0xf3, 0xfd,
    0xe7, 0x80, 0xd7, 0xaf, 0x5e, 0xbd, 0x7f, 0xdb, 0x00, 0xc7, 0xdd, 0xf1,
    0xc6, 0xdd, 0xf1, 0x80, 0xc7, 0x8f, 0x1e, 0x3f, 0xef, 0xc1, 0xbe, 0x7c,
    0xff, 0xfc, 0x71, 0xc7, 0x1f, 0xff, 0x80, 0x37, 0x66, 0xce, 0x66, 0x73,
    0xff, 0xff, 0xf0, 0xce, 0x66, 0x37, 0x66, 0xec, 0x70, 0xe2, 0xed, 0xf1,
    0xc0,
};

/*-----------------
 *    GLYPH DSC
 *----------------*/
static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 128, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 128, .box_w = 3, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 5, .adv_w = 128, .box_w = 5, .box_h = 5, .ofs_x = 1, .ofs_y = 7},
    {.bitmap_index = 9, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 18, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 27, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 36, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 45, .adv_w = 128, .box_w = 2, .box_h = 5, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 47, .adv_w = 128, .box_w = 4, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 52, .adv_w = 128, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 57, .adv_w = 128, .box_w = 7, .box_h = 8, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 64, .adv_w = 128, .box_w = 6, .box_h = 8, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 70, .adv_w = 128, .box_w = 3, .box_h = 5, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 72, .adv_w = 128, .box_w = 6, .box_h = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 74, .adv_w = 128, .box_w = 2, .box_h = 3, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 75, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 84, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 93, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 101, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 110, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 119, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 128, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 137, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 146, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 155, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 164, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 173, .adv_w = 128, .box_w = 2, .box_h = 8, .ofs_x = 2, .ofs_y = 3},
    {.bitmap_index = 175, .adv_w = 128, .box_w = 3, .box_h = 9, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 179, .adv_w = 128, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 186, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 191, .adv_w = 128, .box_w = 5, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 198, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 207, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 216, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 225, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 234, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 243, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 252, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 261, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 270, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 279, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 288, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 296, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 305, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 314, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 322, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 331, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 340, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 349, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 358, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 367, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 376, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 385, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 393, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 402, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 411, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 420, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 429, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 437, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 446, .adv_w = 128, .box_w = 4, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 451, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 460, .adv_w = 128, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 465, .adv_w = 128, .box_w = 5, .box_h = 3, .ofs_x = 1, .ofs_y = 9},
    {.bitmap_index = 467, .adv_w = 128, .box_w = 7, .box_h = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 469, .adv_w = 128, .box_w = 2, .box_h = 3, .ofs_x = 3, .ofs_y = 9},
    {.bitmap_index = 470, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 477, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 486, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 493, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 502, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 509, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 517, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 525, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 534, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 542, .adv_w = 128, .box_w = 5, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 550, .adv_w = 128, .box_w = 7, .box_h = 10, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 559, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 567, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 574, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 581, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 588, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 596, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 610, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 617, .adv_w = 128, .box_w = 6, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 625, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 632, .adv_w = 128, .box_w = 6, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 638, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 645, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 652, .adv_w = 128, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 660, .adv_w = 128, .box_w = 7, .box_h = 7, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 667, .adv_w = 128, .box_w = 4, .box_h = 10, .ofs_x = 2, .ofs_y = 2},
    {.bitmap_index = 672, .adv_w = 128, .box_w = 2, .box_h = 10, .ofs_x = 3, .ofs_y = 2},
    {.bitmap_index = 675, .adv_w = 128, .box_w = 4, .box_h = 10, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 680, .adv_w = 128, .box_w = 7, .box_h = 5, .ofs_x = 0, .ofs_y = 5},
};

static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};

#if LVGL_VERSION_MAJOR == 8
static lv_font_fmt_txt_glyph_cache_t cache;
#endif

static const lv_font_fmt_txt_dsc_t font_dsc = {
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

const lv_font_t press_start_2p_10 = {
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,
    .line_height = 11,
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

#endif
