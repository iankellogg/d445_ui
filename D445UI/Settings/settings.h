#ifndef __D445_SETTINGS_H_
#define __D445_SETTINGS_H_
#include "lvgl/lvgl.h"

#ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif

EXTERNC void create_settings_tab(lv_obj_t *tabRef);
EXTERNC lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt, uint8_t builder_variant);

#undef EXTERNC
#endif