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
extern int32_t Threshold_Slider,Blur_Value,Threshold_size,contourFilter,dilation_size;
extern int32_t polyfit,matchValue;
extern bool Calibration;
extern bool Flatten,ContourCalibrate,Pause;

EXTERNC void start_camera_thread(lv_obj_t *img);

EXTERNC void Send_Contour_Cursor_Pos(int x, int y);
// max is, if its the max color limit
EXTERNC void SetColorFilter(bool max, int h, int s, int v);
EXTERNC void GetColorFilter(bool max, int *h, int *s, int *v);

#undef EXTERNC


#endif