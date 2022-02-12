/*
 * cannon.h
 *
 *  Created on: Oct 23, 2021
 *      Author: ikellogg
 */

#ifndef CANNON_H_
#define CANNON_H_

#include <libintl.h>
#include <locale.h>
#include "lvgl/lvgl.h"
#ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif
#define _(STRING) gettext(STRING)
#include <pthread.h>

extern pthread_mutex_t lvgl_mutex;
    extern lv_obj_t * kb;
 EXTERNC   void ta_event_cb(lv_event_t * e);
 EXTERNC void radio_event_handler(lv_event_t * e);
 
EXTERNC void slider_event_cb(lv_event_t * e);



typedef struct
{
    char *Text;
    char *ToggledText;
    int32_t *Value;
} button_cb_t;
EXTERNC void button_event_cb(lv_event_t * e);

EXTERNC void create_cannon_application(void);

#undef EXTERNC

#endif /* CANNON_H_ */
