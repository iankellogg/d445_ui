
// OPEN CV INCLUDES
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "opencv.h"
#include <stdlib.h>
#include <unistd.h>
#include <cannon.h>
#include <iostream>

using namespace cv;
using namespace std;

// active camera mode switches which camera output is shown to the user
// 0 = raw in color
// 1 = Computer Vision Input
// 2 = Computer Vision recreation
uint32_t active_camera_mode = 0;

int32_t Threshold_Slider=0,Blur_Value=7;

// This puts the thread in calibration mode
// where we calibrate the lens distoration
bool Calibration = false;
static const int CHECKERBOARD[2] = {4,4};


pthread_t CameraThread;

VideoCapture cap;


void init_camera()
{
     int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    // open selected camera using selected API
    cap.open(deviceID, apiID);
    // check if we succeeded
    if (!cap.isOpened()) {
        printf("ERROR! Unable to open camera\n");
        return;
    }
    cap.set(CAP_PROP_FRAME_WIDTH,800);
    cap.set(CAP_PROP_FRAME_HEIGHT,600);
}


void *CamUpdate(void *arg)
{
   lv_obj_t *img = (lv_obj_t*)arg;
Mat frame;
Mat gray;
Mat outputFrame;
int i = 1;

init_camera();

    lv_img_dsc_t imgDsc;

  std::vector<cv::Point3f> objp;
  
  // Creating vector to store vectors of 3D points for each checkerboard image
  std::vector<std::vector<cv::Point3f> > objpoints;
  // Creating vector to store vectors of 2D points for each checkerboard image
  std::vector<std::vector<cv::Point2f> > imgpoints;
   cv::Mat map1, map2;
    bool calibrated=false;
  for(int i{0}; i<CHECKERBOARD[1]; i++)
  {
    for(int j{0}; j<CHECKERBOARD[0]; j++)
      objp.push_back(cv::Point3f(j,i,0));
  }
   int calCount=0;


    while (1) 
    {

        cap.read(frame);
        if (frame.empty())
        {
            continue;
        }
        cvtColor(frame, gray , COLOR_BGRA2GRAY,0);

        //do the calibration
        if (Calibration == true) 
        {
            
            std::vector<Point2f> corner_pts;
            bool success = findChessboardCorners(gray, Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK  | CALIB_CB_NORMALIZE_IMAGE);
            if(success)
            {
                cv::TermCriteria criteria(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, .001);
                // refining pixel coordinates for given 2d points.

                cornerSubPix(gray,corner_pts,Size(11,11), Size(-1,-1),criteria);
                drawChessboardCorners(frame, Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);
  std::vector<std::vector<cv::Point3f> > objpoints_buffer;
  // Creating vector to store vectors of 2D points for each checkerboard image
  std::vector<std::vector<cv::Point2f> > imgpoints_buffer;
                 objpoints_buffer.push_back(objp);
                imgpoints_buffer.push_back(corner_pts);

                cv::TermCriteria criteria2(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, 1e-6);
                cv::Mat K, D;
                std::vector<cv::Mat> rvecs, tvecs;
                int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC|cv::fisheye::CALIB_FIX_SKEW;
                try {
                cv::fisheye::calibrate(objpoints_buffer, imgpoints_buffer, gray.size(), K, D, rvecs, tvecs, flags, criteria2);
                } catch(Exception e){
                    printf("Calibration Failed %d\r\n",calCount);

                    continue;
                } 
                printf("Calibrated SUccess %d\r\n",calCount);
                 objpoints.push_back(objp);
                imgpoints.push_back(corner_pts);
                calCount++;
            }
                
            if (calCount>100)
            {         

                cv::TermCriteria criteria(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, 1e-6);
                cv::Mat K, D;
                std::vector<cv::Mat> rvecs, tvecs;
                int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC|cv::fisheye::CALIB_FIX_SKEW;
                try {
                cv::fisheye::calibrate(objpoints, imgpoints, gray.size(), K, D, rvecs, tvecs, flags, criteria);
                } catch(Exception e){
                    printf("Calibration Failed\r\n");

                    continue;
                } 
                printf("Calibrated Success\r\n");
                cv::fisheye::initUndistortRectifyMap(K, D, cv::Mat::eye(3, 3, CV_32F), K, gray.size(), CV_16SC2, map1, map2);
    std::cout
        << "K=" << K << std::endl
        << "D=" << D << std::endl;
                calibrated=true;
                Calibration=false;
                calCount=0;
			}
        } else {
            if (calibrated)
            {
                
                 cv::remap(gray, gray, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
            }

            if (Blur_Value>0 && i==0)
            {
                //blur( imgray, imgray, Size(blur_slider,blur_slider) );
                if (Blur_Value%2==0)
                    Blur_Value++;
                medianBlur(gray,gray,Blur_Value);
            }
            if (Threshold_Slider>0 || i == 1)
            {
                i=0;
                threshold(gray,gray,Threshold_Slider, 255, 0);
            }
            else
            {
            adaptiveThreshold(gray,gray,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,50*2+1,1);
            //	Mat element = getStructuringElement(MORPH_RECT , (5,1));
            int dilation_size=1;
            Mat element = getStructuringElement( MORPH_RECT ,Size( 2*dilation_size + 1, 2*dilation_size+1 ),Point( dilation_size, dilation_size ) );
            //				   	  dilate( imgray, imgray, element );
            morphologyEx(gray,gray, MORPH_CLOSE, element);
            }
        }

        if (active_camera_mode==1)
        {
            cvtColor(gray, outputFrame, COLOR_GRAY2BGRA,0);
        }
        else
        {
            cvtColor(frame, outputFrame, COLOR_BGR2BGRA,0);
        }

        //uint length = frame.total()*frame.channels();
        uchar * arr = outputFrame.isContinuous()? outputFrame.data: outputFrame.clone().data;
        imgDsc.data = arr;
    imgDsc.data_size = outputFrame.total()*outputFrame.channels();
    imgDsc.header.h = outputFrame.rows;
    imgDsc.header.w = outputFrame.cols;
    imgDsc.header.always_zero=0;
    imgDsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    pthread_mutex_lock(&lvgl_mutex);
        lv_img_set_src(img,&imgDsc);
    pthread_mutex_unlock(&lvgl_mutex);
    // lv_img_cache_invalidate_src(NULL);
      usleep(60 * 1000);
    }
}




void start_camera_thread(lv_obj_t *img)
{
    
    pthread_create(&CameraThread, NULL, &CamUpdate, img);
}