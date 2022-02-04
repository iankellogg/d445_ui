
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
int32_t polyfit = 2;
int32_t Threshold_Slider=0,Blur_Value=7;
bool Flatten = false,ContourCalibrate=false,Pause=false;

// This puts the thread in calibration mode
// where we calibrate the lens distoration
bool Calibration = false;
static const int CHECKERBOARD[2] = {4,4};

Point2f cursorPos = Point2f(1000,1000);
pthread_t CameraThread;

// the handle we want to find
vector<Point> ShapeToFind = {Point2i(487,162),Point2i(262,356),Point2i(532,418),Point2i(471,296)};


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


void draw_tray(Mat &canvas, int numSamples, int radius, Point &knob, double startAngle)
{
    Point center;
    for (int i=0;i<numSamples;i++)
    {
        center.x = radius*cos((startAngle+i*(360.0/(float)numSamples))*M_PI/180.0) + knob.x;
        center.y = radius*sin((startAngle+i*(360.0/(float)numSamples))*M_PI/180.0) + knob.y;
         cv::circle	(canvas, center, 40, Scalar(200,0,0),-1,LINE_8,0);
    }
}

void *CamUpdate(void *arg)
{
   lv_obj_t *img = (lv_obj_t*)arg;
Mat frame;
Mat gray;
Mat warpped; 
Mat drawing;
Mat outputFrame;
int count = 1;

init_camera();

    lv_img_dsc_t imgDsc;

  std::vector<cv::Point3f> objp;
  
  // Creating vector to store vectors of 3D points for each checkerboard image
  std::vector<std::vector<cv::Point3f> > objpoints;
  // Creating vector to store vectors of 2D points for each checkerboard image
  std::vector<std::vector<cv::Point2f> > imgpoints;
   cv::Mat map1, map2;
   double tran_c[3][3] = {{0.9887319429809974, 0.05716665516063728, -117.719050132904},{ -0.07706136048259898, 0.9687507839912129, 47.90186573630533},{ 1.14939671416363e-05, -7.609421690938215e-05, 1}};

   cv::Mat trans = cv::Mat(3,3,CV_64F,&tran_c);
    bool calibrated=true;
    bool flatten=true;
  for(int i{0}; i<CHECKERBOARD[1]; i++)
  {
    for(int j{0}; j<CHECKERBOARD[0]; j++)
      objp.push_back(cv::Point3f(j,i,0));
  }
   int calCount=0;
   double K_c[3][3] = { {418.5248911727859, 0, 396.5416920228854},{0, 417.3716543944635, 271.7286752066629},{0, 0, 1}};
   double D_c[4] = { -0.04456047071614869, 0.2307124415380794, -0.7307970854849617, 0.6943486286581442};
   Mat K=cv::Mat(3, 3, CV_64F, &K_c);
   Mat D=cv::Mat(4, 1, CV_64F, &D_c);
     std::cout
        << "K=" << K << std::endl
        << "D=" << D << std::endl;
cv::Size s;
s.width=800;
s.height=600;
cv::fisheye::initUndistortRectifyMap(K, D, cv::Mat::eye(3, 3, CV_32F), K, s, CV_16SC2, map1, map2);
  
           drawing = Mat::zeros( s, CV_8UC3 );



    while (1) 
    {
        if (!Pause)
        {
            cap.read(frame);
            if (frame.empty())
            {
                continue;
            }
        }

        if (Calibration == true) 
        {
            
        cvtColor(frame, gray , COLOR_BGRA2GRAY,0);
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
            calCount=0;
            if (calibrated)
            {    
                try {
                 cv::remap(frame, warpped, map1, map2, cv::INTER_LINEAR, cv::BORDER_CONSTANT);
                } catch (Exception e)
                {
                    printf("remap fail\r\n");
                }
			 }
            if (flatten && Flatten==false)
            { 
                if (calibrated )
                    warpPerspective(warpped,warpped,trans,Size(frame.cols,frame.rows),INTER_LINEAR);
                    else
                    warpPerspective(frame,warpped,trans,Size(frame.cols,frame.rows),INTER_LINEAR);

            } 
            
        cvtColor(warpped, gray , COLOR_BGRA2GRAY,0);
            if (Flatten == true)
        {
            std::vector<Point2f> corner_pts;
            bool success = findChessboardCorners(gray, Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK  | CALIB_CB_NORMALIZE_IMAGE);
            if(success)
            {  
                cv::TermCriteria criteria(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, .001);
              
                cornerSubPix(gray,corner_pts,Size(11,11), Size(-1,-1),criteria);
                vector<Point2f> points;
                    int corner_pts_size = corner_pts.size();
                    points.push_back(corner_pts[0]);
                    points.push_back(corner_pts[(corner_pts_size/4)-1]);
                    points.push_back(corner_pts[corner_pts_size-1]);
                    points.push_back(corner_pts[((3*corner_pts_size)/4)]);
                    // printf("Drawing %d %d %d %d\r\n",0,(corner_pts_size/4)-1,((3*corner_pts_size)/4)-1,corner_pts_size-1);
                    //points.push_back(corner_pts[0]);
                    //polylines(frame,points,true,(0,255,255));
                    // imshow("Corners",frame);

                    vector<Point2f> warpPoints;
                    int scale=5;
                    int dist = frame.rows/scale;
                    int mid_y = frame.cols/2;
                    int mid_x = frame.rows/2;
                    warpPoints.push_back(Point(mid_y-dist,mid_x-dist));
                    warpPoints.push_back(Point(mid_y+dist,mid_x-dist));
                    warpPoints.push_back(Point(mid_y+dist,mid_x+dist));
                    warpPoints.push_back(Point(mid_y-dist,mid_x+dist));
                    trans = getPerspectiveTransform(points,warpPoints);
                    std::cout << "Trans: " << trans << endl;
                    flatten=true;
                     Flatten= false;
                 }
        }

            if (Blur_Value>0 && count==0)
            {
                //blur( imgray, imgray, Size(blur_slider,blur_slider) );
                if (Blur_Value%2==0)
                    Blur_Value++;
                medianBlur(gray,gray,Blur_Value);
            }
            if (Threshold_Slider>0 || count == 1)
            {
                count=0;
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

			   vector<vector<Point> > contours;
		   vector<Vec4i> hierarchy;
		   findContours( gray, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );
           drawing = Mat::zeros( gray.size(), CV_8UC3 );
           double closestMatch = DBL_MAX;
           int found=-1;
		   for( size_t i = 0; i< contours.size(); i++ )
           {
               
               Scalar color = Scalar( 133, 133, 133 );
			   approxPolyDP	(contours[i],contours[i],((double)polyfit/50.0) * arcLength(contours[i], true),true);
               if (ContourCalibrate )
               {
                  double testMatch = cv::pointPolygonTest	(	contours[i],cursorPos,true );
                  if (testMatch>=0 && testMatch<closestMatch)
                  {
                      closestMatch = testMatch;
                      found = i;
                  }
			    drawContours( drawing, contours, i, color, 2, LINE_8, hierarchy, 0 );
               }
               else if (ShapeToFind.size() > 1)
               {
                   // start trying to find the contour
                    double ret = matchShapes(contours[i],ShapeToFind,CONTOURS_MATCH_I1,0.0);
                    if (ret<0.05)
                    {     
                         Point knob_center;
                        cv::Moments m = cv::moments(contours[i], true);
                        double cen_x=m.m10/m.m00;
                        double cen_y=m.m01/m.m00;
				        knob_center = Point(cen_x,cen_y);

			// this finds the point closest to the center of the contour
					// because the shape of the knob has a point near the center, it must be pointing that way
				  double minDistance=DBL_MAX;
				  int minDistancePoint=0;
				  for (int j=0; j<contours[i].size();j++)
				  {
						Point p = contours[i][j];
						double res = norm(p-knob_center);
						if (res<minDistance)
						{
							minDistance = res;
							minDistancePoint = j;
						}
				  }
				  double theta = 180-(atan2(contours[i][minDistancePoint].y-knob_center.y,contours[i][minDistancePoint].x-knob_center.x)*180.0/3.14159);


				//  avgAngle = theta;// 0.9*avgAngle + 0.1*theta;
				 // printf("%f\r\n",avgAngle);
				  char angleText[15];
				  //sprintf(angleText,"%f = %f - %f",avgAngle,mp.z,mu.z);
				  sprintf(angleText,"%f",theta);
				  putText(drawing, angleText, Point(50,frame.rows-50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255), 1, LINE_AA);
				        fillPoly(drawing,contours[i],Scalar(0,0,255));
			        line(drawing,knob_center,contours[i][minDistancePoint],Scalar(0,255,0));
                    
                     draw_tray(drawing, 20, 300, knob_center,theta);
                    break;
                    }
               }
            }
            if (found>=0 && ContourCalibrate)
            {
			   drawContours( drawing, contours, found, Scalar( 33, 200, 33 ), 2, LINE_8, hierarchy, 0 );
               
                ShapeToFind.clear();
                for (int i=0; i<contours[found].size();i++)
                {
                    ShapeToFind.push_back(contours[found][i]);
                }
               cout << "Shape: " << ShapeToFind << endl;
            }





        }
    switch (active_camera_mode)
    {
        case 3:
            cvtColor(drawing,outputFrame,COLOR_BGR2BGRA,0);
            break;
        case 2:
            cvtColor(gray, outputFrame, COLOR_GRAY2BGRA,0);
            break;
        case 1:
            cvtColor(warpped, outputFrame, COLOR_BGR2BGRA,0);
            break;
        case 0:
        default:
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



void Send_Contour_Cursor_Pos(int x, int y)
{
    cursorPos.x = x;
    cursorPos.y = y;
}

void start_camera_thread(lv_obj_t *img)
{
    
    pthread_create(&CameraThread, NULL, &CamUpdate, img);
}