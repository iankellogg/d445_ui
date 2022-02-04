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
extern int32_t polyfit;
extern bool Calibration;
extern bool Flatten,ContourCalibrate,Pause;

EXTERNC void start_camera_thread(lv_obj_t *img);

EXTERNC void Send_Contour_Cursor_Pos(int x, int y);

#undef EXTERNC


#endif