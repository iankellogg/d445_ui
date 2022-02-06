
#include <mlx90640.h>
#include "mlx90640_ui.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>    
#include <limits.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define FPS 8
#define FRAME_TIME_MICROS (1000000/FPS)

// Despite the framerate being ostensibly FPS hz
// The frame is often not ready in time
// This offset is added to the FRAME_TIME_MICROS
// to account for this.
#define OFFSET_MICROS 850
#define MLX_I2C_ADDR 0x33

#define SENSOR_W 24
#define SENSOR_H 32


    paramsMLX90640 mlx90640;
 pthread_mutex_t mlx90640_mutex;
pthread_t MLX90640Thread;
thermal_image_t image;

void thermalCameraTask(void *param);

void thermal_getframe(thermal_image_t img)
{
    pthread_mutex_lock(&mlx90640_mutex);
    memcpy(img,image,sizeof(thermal_image_t));
    pthread_mutex_unlock(&mlx90640_mutex);
}
    #define NUM_COLORS  5
//var keys = ["white", "gold", "#c07", "#20008c", "black"];
    static float color[NUM_COLORS][3] = { {0,0,0},{32.0/255.0,0,140.0/255.0} , {204.0/255.0,0.0/255.0,119.0/255.0}, {1,221.0/255.0,0}, {1,1,1} };
  //  static float color[NUM_COLORS][3] = {  {1,1,1} , {32.0/255.0,0,140.0/255.0}, {0,12.0/255.0,7.0/255.0}, {1,221.0/255.0,0},{0,0,0}};

void colorPixel(thermal_color_t *pixelOut, float pixelIn, float maxTemp, float minTemp)
{
    int idx1, idx2;
    float fractBetween = 0;
    float vrange = maxTemp-minTemp;
    pixelIn -= minTemp;
    pixelIn /= vrange;
    if(pixelIn <= 0) {idx1=idx2=0;}
    else if(pixelIn >= 1) {idx1=idx2=NUM_COLORS-1;}
    else
    {
        pixelIn *= (NUM_COLORS-1);
        idx1 = floor(pixelIn);
        idx2 = idx1+1;
        fractBetween = pixelIn - (float)idx1;
    }

    int ir, ig, ib;

    ir = (int)((((color[idx2][0] - color[idx1][0]) * fractBetween) + color[idx1][0]) * 255.0);
    ig = (int)((((color[idx2][1] - color[idx1][1]) * fractBetween) + color[idx1][1]) * 255.0);
    ib = (int)((((color[idx2][2] - color[idx1][2]) * fractBetween) + color[idx1][2]) * 255.0);

    
    pixelOut->r = ir;
    pixelOut->g = ig;
    pixelOut->b = ib;
}

void thermal_colorImage(thermal_image_t image, thermal_color_image_t colorImage)
{
    static float maxTemp = 50;
    static float minTemp = 0;
    float newMaxTemp = -INT_MAX;
    float newMinTemp = INT_MAX;
   for(int y = 0; y < SENSOR_W; y++){
        for(int x = 0; x < SENSOR_H; x++){
            float val = image[SENSOR_H * (SENSOR_W-1-y) + x];
            if (val>newMaxTemp) 
            {
                newMaxTemp = val;
            }
            else
            if (val<newMinTemp) 
            {
                newMinTemp = val;
            }
            colorPixel(& colorImage[SENSOR_H * (SENSOR_W-1-y) + x],val,maxTemp,minTemp);

        }
    }
    maxTemp = newMaxTemp;
    minTemp = newMinTemp;
}

void thermal_init()
{

    //MLX90640_SetDeviceMode(MLX_I2C_ADDR, 0);
    //MLX90640_SetSubPageRepeat(MLX_I2C_ADDR, 0);
    MLX90640_SetRefreshRate(MLX_I2C_ADDR, 1<<(FPS-1));
    MLX90640_SetChessMode(MLX_I2C_ADDR);

    uint16_t eeMLX90640[832];
    MLX90640_DumpEE(MLX_I2C_ADDR, eeMLX90640);
    MLX90640_ExtractParameters(eeMLX90640, &mlx90640);

    
    if (pthread_mutex_init(&mlx90640_mutex, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }
    pthread_create(&MLX90640Thread, NULL, &thermalCameraTask, NULL);

}

/// Get a time stamp in microseconds.
uint64_t micros()
{
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    uint64_t us = (uint64_t)(ts.tv_sec*1000000) + (uint64_t)(ts.tv_nsec*1000000000);
    return us;
}



void thermalCameraTask(void *param)
{
    
    static uint16_t eeMLX90640[832];
    float emissivity = 1;
    uint16_t frame[834];
    float mlx90640To[768];
    float eTa;
    int32_t frame_time = (FRAME_TIME_MICROS);
    
    while(1){


        uint64_t start = micros();
        MLX90640_GetFrameData(MLX_I2C_ADDR, frame);

        eTa = MLX90640_GetTa(frame, &mlx90640);
        MLX90640_CalculateTo(frame, &mlx90640, emissivity, eTa, mlx90640To);

        MLX90640_BadPixelsCorrection((&mlx90640)->brokenPixels, mlx90640To, 1, &mlx90640);
        MLX90640_BadPixelsCorrection((&mlx90640)->outlierPixels, mlx90640To, 1, &mlx90640);
       // printf("Time to i2c: %d\r\n",(uint32_t)(micros()-start));

     pthread_mutex_lock(&mlx90640_mutex);
    memcpy(image,mlx90640To,sizeof(image));
     
    pthread_mutex_unlock(&mlx90640_mutex);
    //     int minTemp = INT_MAX;
    //     int maxTemp = -INT_MAX;
    //     for(int i=0;i<768;i++){
    //         if(minTemp > mlx90640To[i]) minTemp = mlx90640To[i];
    //         if(maxTemp < mlx90640To[i]) maxTemp = mlx90640To[i];
    //     }

    //     for(int y = 0; y < SENSOR_W; y++){
    //         for(int x = 0; x < SENSOR_H; x++){
    //             float val = mlx90640To[SENSOR_H * (SENSOR_W-1-y) + x];
    //            // put_pixel_false_colour(y, x, val);
    //         }
    //     }
        

        // uint64_t end = micros();
        // int32_t elapsed = end - start;
        // uint32_t sleepTime = 0;
        // if ((frame_time - elapsed)<0) 
        // { 
        //     sleepTime=1;
        // } else
        // if ((frame_time - elapsed)>1000000)
        // { 
        //     sleepTime=1000000;
        // }
        // else
        // {
        //     sleepTime=elapsed;
        // }
        // printf("sleep time %d\r\n",sleepTime);
        // usleep(sleepTime);
    }
}