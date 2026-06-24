/**
 * @file lv_port_indev.h
 *
 */

#ifndef LV_PORT_INDEV_H
#define LV_PORT_INDEV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_drv_conf.h"
#include "lvgl/lvgl.h"

void lv_port_indev_init(void);
void lv_port_indev_set_drag_obj(lv_obj_t *obj, bool enable);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_PORT_INDEV_H */
