/**
 * @file lv_port_indev.c
 *
 */

#include "lv_port_indev.h"

#include "lvgl/lvgl.h"
#if USE_EVDEV != 0 || USE_BSD_EVDEV
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#if USE_BSD_EVDEV
#include <dev/evdev/input.h>
#else
#include <linux/input.h>
#endif

#if USE_XKB
#include "xkb.h"
#endif /* USE_XKB */

#if USE_TSLIB
#include "tslib.h"
struct tsdev *ts;
#endif /* USE_TSLIB */


bool evdev_set_file(char* dev_name);
int map(int x, int in_min, int in_max, int out_min, int out_max);

/**********************
 *  STATIC VARIABLES
 **********************/
int evdev_fd = -1;
int evdev_root_x;
int evdev_root_y;
int evdev_button;

int evdev_key_val;
static bool evdev_press_cancelled;
static lv_obj_t *evdev_pressed_obj;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void evdev_feedback(lv_indev_drv_t *drv, uint8_t event_code)
{
    lv_indev_t *indev = lv_indev_get_act();

    LV_UNUSED(drv);
    if(indev == NULL || lv_indev_get_type(indev) != LV_INDEV_TYPE_POINTER)
        return;

    if(event_code == LV_EVENT_PRESSED) {
        evdev_pressed_obj = lv_indev_get_obj_act();
        if(evdev_pressed_obj != NULL &&
           !lv_obj_has_flag(evdev_pressed_obj, LV_OBJ_FLAG_USER_4))
            lv_obj_clear_flag(evdev_pressed_obj, LV_OBJ_FLAG_PRESS_LOCK);
    } else if(event_code == LV_EVENT_PRESS_LOST && evdev_pressed_obj != NULL) {
        // 滑出后取消本次按压，松手前不转移到其他对象
        evdev_press_cancelled = true;
        lv_indev_reset(indev, evdev_pressed_obj);
        evdev_pressed_obj = NULL;
    } else if(event_code == LV_EVENT_RELEASED) {
        evdev_pressed_obj = NULL;
    }
}

void lv_port_indev_set_drag_obj(lv_obj_t *obj, bool enable)
{
    if(obj == NULL)
        return;

    if(enable)
        lv_obj_add_flag(obj, LV_OBJ_FLAG_USER_4);
    else
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_USER_4);
}

/**
 * Initialize the evdev interface
 */
void evdev_init(void)
{
    if (!evdev_set_file(EVDEV_NAME)) {
        return;
    }

#if USE_XKB
    xkb_init();
#endif
}
/**
 * reconfigure the device file for evdev
 * @param dev_name set the evdev device filename
 * @return true: the device file set complete
 *         false: the device file doesn't exist current system
 */
bool evdev_set_file(char* dev_name)
{
     if(evdev_fd != -1) {
        close(evdev_fd);
     }
#if USE_TSLIB == 0
#if USE_BSD_EVDEV
     evdev_fd = open(dev_name, O_RDWR | O_NOCTTY);
#else
     evdev_fd = open(dev_name, O_RDWR | O_NOCTTY | O_NDELAY);
#endif

     if(evdev_fd == -1) {
        perror("unable to open evdev interface:");
        return false;
     }

#if USE_BSD_EVDEV
     fcntl(evdev_fd, F_SETFL, O_NONBLOCK);
#else
     fcntl(evdev_fd, F_SETFL, O_ASYNC | O_NONBLOCK);
#endif
#else
     ts = ts_setup(NULL, 1);
     if(!ts){
        perror("ts_setup");
        return false;
     }
#endif
     evdev_root_x = 0;
     evdev_root_y = 0;
     evdev_key_val = 0;
     evdev_button = LV_INDEV_STATE_REL;
     evdev_press_cancelled = false;
     evdev_pressed_obj = NULL;

     return true;
}
/**
 * Get the current position and state of the evdev
 * @param data store the evdev data here
 */
void evdev_read(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
#if USE_TSLIB == 0
    struct input_event in;

    while(read(evdev_fd, &in, sizeof(struct input_event)) > 0) {
        if(in.type == EV_REL) {
            if(in.code == REL_X)
				#if EVDEV_SWAP_AXES
					evdev_root_y += in.value;
				#else
					evdev_root_x += in.value;
				#endif
            else if(in.code == REL_Y)
				#if EVDEV_SWAP_AXES
					evdev_root_x += in.value;
				#else
					evdev_root_y += in.value;
				#endif
        } else if(in.type == EV_ABS) {
            if(in.code == ABS_X)
				#if EVDEV_SWAP_AXES
					evdev_root_y = in.value;
				#else
					evdev_root_x = in.value;
				#endif
            else if(in.code == ABS_Y)
				#if EVDEV_SWAP_AXES
					evdev_root_x = in.value;
				#else
					evdev_root_y = in.value;
				#endif
            else if(in.code == ABS_MT_POSITION_X)
                                #if EVDEV_SWAP_AXES
                                        evdev_root_y = in.value;
                                #else
                                        evdev_root_x = in.value;
                                #endif
            else if(in.code == ABS_MT_POSITION_Y)
                                #if EVDEV_SWAP_AXES
                                        evdev_root_x = in.value;
                                #else
                                        evdev_root_y = in.value;
                                #endif
            else if(in.code == ABS_MT_TRACKING_ID) {
                                if(in.value == -1) {
                                    evdev_button = LV_INDEV_STATE_REL;
                                    evdev_press_cancelled = false;
                                    evdev_pressed_obj = NULL;
                                } else if(in.value == 0)
                                    evdev_button = LV_INDEV_STATE_PR;
            }
        } else if(in.type == EV_KEY) {
            if(in.code == BTN_MOUSE || in.code == BTN_TOUCH) {
                if(in.value == 0) {
                    evdev_button = LV_INDEV_STATE_REL;
                    evdev_press_cancelled = false;
                    evdev_pressed_obj = NULL;
                } else if(in.value == 1)
                    evdev_button = LV_INDEV_STATE_PR;
            } else if(drv->type == LV_INDEV_TYPE_KEYPAD) {
#if USE_XKB
                data->key = xkb_process_key(in.code, in.value != 0);
#else
                switch(in.code) {
                    case KEY_BACKSPACE:
                        data->key = LV_KEY_BACKSPACE;
                        break;
                    case KEY_ENTER:
                        data->key = LV_KEY_ENTER;
                        break;
                    case KEY_PREVIOUS:
                        data->key = LV_KEY_PREV;
                        break;
                    case KEY_NEXT:
                        data->key = LV_KEY_NEXT;
                        break;
                    case KEY_UP:
                        data->key = LV_KEY_UP;
                        break;
                    case KEY_LEFT:
                        data->key = LV_KEY_LEFT;
                        break;
                    case KEY_RIGHT:
                        data->key = LV_KEY_RIGHT;
                        break;
                    case KEY_DOWN:
                        data->key = LV_KEY_DOWN;
                        break;
                    case KEY_TAB:
                        data->key = LV_KEY_NEXT;
                        break;
                    default:
                        data->key = 0;
                        break;
                }
#endif /* USE_XKB */
                if (data->key != 0) {
                    /* Only record button state when actual output is produced to prevent widgets from refreshing */
                    data->state = (in.value) ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
                }
                evdev_key_val = data->key;
                evdev_button = data->state;
                return;
            }
        }
    }

    if(drv->type == LV_INDEV_TYPE_KEYPAD) {
        /* No data retrieved */
        data->key = evdev_key_val;
        data->state = evdev_button;
        return;
    }
    if(drv->type != LV_INDEV_TYPE_POINTER)
        return ;
#else
    struct ts_sample samp;
    while(ts_read(ts, &samp, 1) == 1) {
        #if EVDEV_SWAP_AXES
            evdev_root_x = samp.y;
            evdev_root_y = samp.x;
        #else
            evdev_root_x = samp.x;
            evdev_root_y = samp.y;
        #endif

        if(samp.pressure == 0) {
            evdev_button = LV_INDEV_STATE_REL;
            evdev_press_cancelled = false;
            evdev_pressed_obj = NULL;
        } else
            evdev_button = LV_INDEV_STATE_PR;
    }

#endif
    /*Store the collected data*/

#if EVDEV_CALIBRATE
    data->point.x = map(evdev_root_x, EVDEV_HOR_MIN, EVDEV_HOR_MAX, 0, drv->disp->driver->hor_res);
    data->point.y = map(evdev_root_y, EVDEV_VER_MIN, EVDEV_VER_MAX, 0, drv->disp->driver->ver_res);
#else
    data->point.x = evdev_root_x;
    data->point.y = evdev_root_y;
#endif

    if(evdev_press_cancelled) {
        data->state = LV_INDEV_STATE_REL;
        if(evdev_button == LV_INDEV_STATE_REL)
            evdev_press_cancelled = false;
    } else {
        data->state = evdev_button;
    }

    if(data->point.x < 0)
      data->point.x = 0;
    if(data->point.y < 0)
      data->point.y = 0;
    if(data->point.x >= drv->disp->driver->hor_res)
      data->point.x = drv->disp->driver->hor_res - 1;
    if(data->point.y >= drv->disp->driver->ver_res)
      data->point.y = drv->disp->driver->ver_res - 1;

    return ;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
int map(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;

    evdev_init();

    /* Basic initialization */
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    indev_drv.feedback_cb = evdev_feedback;

    /* Register the driver in LVGL and save the created input device object */
    lv_indev_drv_register(&indev_drv);
}

#endif
