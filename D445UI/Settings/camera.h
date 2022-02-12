#ifndef __CAMERASETTINGS_H_
#define __CAMERASETTINGS_H_

#include "lvgl/lvgl.h"


#ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif

EXTERNC lv_obj_t * Create_Camera_Menu(lv_obj_t *Parent, lv_obj_t *Menu);

#undef EXTERNC
#endif