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

 typedef struct 
{
    // the current angle of the tray
    float trayAngle;
    // the numeric position (1-20)
    int trayPosition;
    // how confident we are about the trays position
    // if this number is "low" issue a warning that maybe the tray or camera is dirty
    float trayConfidence;
    int trayID;
    int trayNumPos;
    // if the tray is detected
    struct statusBits_s
    {
        bool trayPresent:1;
        bool TooDark:1; // camera can somehow tell that its too dark to see anything
        bool trayCantMove:1;   // tray tried to move but didn't
        bool trayWrongPosition:1;   // tray moved but can't get to the requested position
    } statusBits;
} camera_status_t;

extern volatile camera_status_t cameraStatus;
extern uint32_t active_camera_mode;
extern volatile opencv_config_t opencv_config;
extern int32_t thermalAlpha;
extern bool Calibration;
extern bool Flatten,ContourCalibrate,Pause;

EXTERNC void start_camera_thread(lv_obj_t *img);

EXTERNC void Send_Contour_Cursor_Pos(int x, int y);

EXTERNC void opencv_save_config();
EXTERNC void opencv_read_config();

#undef EXTERNC


#endif