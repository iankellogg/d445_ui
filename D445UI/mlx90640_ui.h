#pragma once

#ifdef __cplusplus
 #define EXTERNC extern "C"
 #else
 #define EXTERNC
 #endif

typedef struct 
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
}thermal_color_t;


#define MLX90640_SENSOR_W 32
#define MLX90640_SENSOR_H 24

typedef float thermal_image_t[24*32];

typedef thermal_color_t thermal_color_image_t[24*32];

EXTERNC void thermal_init();
EXTERNC void thermal_getframe(thermal_image_t image);
EXTERNC float thermal_getTempAtPoint(uint32_t x, uint32_t y);
EXTERNC void thermal_colorImage(thermal_image_t image, thermal_color_image_t colorImage);

#undef EXTERNC