#ifndef __NETWORK_SETTINGS_H_
#define __NETWORK_SETTINGS_H_
#include <lvgl/lvgl.h>

#ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif

EXTERNC lv_obj_t *Create_NetworkSettings_Menu(lv_obj_t *Parent, lv_obj_t *Menu);
#undef EXTERNC

#endif