#ifndef CARD_ASSETS_H
#define CARD_ASSETS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include <lvgl.h>
#endif

extern const lv_font_t card_font_hdr_11;
extern const lv_font_t card_font_val_22;
extern const lv_font_t card_font_val_20;
extern const lv_font_t card_font_unit_17;
extern const lv_font_t card_font_unit_15;

extern const lv_img_dsc_t img_icon_temp;
extern const lv_img_dsc_t img_icon_hum;
extern const lv_img_dsc_t img_icon_soil;

#ifdef __cplusplus
}
#endif

#endif /* CARD_ASSETS_H */
