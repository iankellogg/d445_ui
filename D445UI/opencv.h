#ifndef __opencv_h_
#define __opencv_h_

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif


#ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif
extern uint32_t active_camera_mode;
extern int32_t Threshold_Slider,Blur_Value;
extern bool Calibration;

EXTERNC void start_camera_thread(lv_obj_t *img);

#undef EXTERNC


#endif