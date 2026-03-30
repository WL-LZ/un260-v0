#ifndef LV_LOADING_GRID_H
#define LV_LOADING_GRID_H

#include "lvgl/lvgl.h"

typedef struct {
    lv_coord_t dot_size;
    lv_coord_t dot_gap;
    lv_color_t dot_color;
    lv_opa_t opa_min;
    lv_opa_t opa_max;
    uint16_t anim_time;
    uint16_t anim_playback_time;
    uint16_t delay_step;
} lv_loading_grid_config_t;

void lv_loading_grid_config_init(lv_loading_grid_config_t *cfg);
lv_obj_t *lv_loading_grid_create(lv_obj_t *parent);
lv_obj_t *lv_loading_grid_create_sized(lv_obj_t *parent, lv_coord_t size);
lv_obj_t *lv_loading_grid_create_with_config(lv_obj_t *parent, const lv_loading_grid_config_t *cfg);
lv_obj_t *lv_loading_grid_create_sized_with_config(lv_obj_t *parent, lv_coord_t size,
                                                   const lv_loading_grid_config_t *cfg);
void lv_loading_grid_set_opa(lv_obj_t *grid, lv_opa_t opa);

#endif
