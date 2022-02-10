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
 typedef struct
 {
    int32_t Threshold_Slider;
    int32_t Blur_Value;
    int32_t Threshold_size;
    int32_t dilation_size;
    int32_t thetaOffset;    
    int32_t polyfit;
    int32_t matchValue;
 } opencv_config_t;

extern uint32_t active_camera_mode;
extern opencv_config_t opencv_config;
extern int32_t thermalAlpha;
extern bool Calibration;
extern bool Flatten,ContourCalibrate,Pause;

EXTERNC void start_camera_thread(lv_obj_t *img);

EXTERNC void Send_Contour_Cursor_Pos(int x, int y);
// max is, if its the max color limit
EXTERNC void SetColorFilter(bool max, int h, int s, int v);
EXTERNC void GetColorFilter(bool max, int *h, int *s, int *v);
EXTERNC void opencv_save_config();
EXTERNC void opencv_read_config();

#undef EXTERNC


#endif