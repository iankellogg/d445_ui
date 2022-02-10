
// OPEN CV INCLUDES
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/objdetect.hpp>
#include "opencv2/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
using namespace cv;
using namespace cv::xfeatures2d;

#include "opencv.h"
#include <stdlib.h>
#include <unistd.h>
#include <cannon.h>
#include <iostream>

#include "mlx90640_ui.h"

using namespace cv;
using namespace std;
//using namespace cv::Feature2D;

#define SHAPEFILENAME "shape.mat"
#define CONFIGFILENAME "opencv.cfg"

// active camera mode switches which camera output is shown to the user
opencv_config_t opencv_config = { .Threshold_Slider=500, .Blur_Value=5, .Threshold_size=100, .dilation_size=2, .thetaOffset=16753, .polyfit = 30, .matchValue = 50 };
uint32_t active_camera_mode = 4;
int32_t thermalAlpha=0;
bool Flatten = false,ContourCalibrate=false,Pause=false;
bool foundQRcode=true;
// if 0, no tray present
// sets to 10 when present, counts down for each frame it is missing, but resets to 10 when found
static const int TrayPresentResetVal = 10;
int TrayPresentCounter=0;

// This puts the thread in calibration mode
// where we calibrate the lens distoration
bool Calibration = false;
static const int CHECKERBOARD[2] = {4,4};

Point2f cursorPos = Point2f(1000,1000);
pthread_t CameraThread;
    QRCodeDetector qrDecoder;

// the handle we want to find
// simplified knob
//vector<Point> ShapeToFind = {Point2i(288, 193),Point2i(414, 456),Point2i(456, 333),Point2i(569, 244)};
// fully sized rounded knob
vector<Point> ShapeToFind = {Point2i(290, 194),Point2i(289, 195),Point2i(288, 195),Point2i(287, 196),Point2i(286, 196),Point2i(279, 203),Point2i(279, 204),Point2i(278, 205),Point2i(278, 206),Point2i(277, 207),Point2i(277, 217),Point2i(278, 218),Point2i(278, 221),Point2i(279, 222),Point2i(279, 223),Point2i(280, 224),Point2i(280, 226),Point2i(281, 227),Point2i(281, 228),Point2i(282, 229),Point2i(282, 230),Point2i(283, 231),Point2i(283, 233),Point2i(284, 234),Point2i(284, 235),Point2i(285, 236),Point2i(285, 237),Point2i(286, 238),Point2i(286, 239),Point2i(287, 240),Point2i(287, 241),Point2i(288, 242),Point2i(288, 243),Point2i(289, 244),Point2i(289, 245),Point2i(290, 246),Point2i(290, 248),Point2i(291, 249),Point2i(291, 250),Point2i(292, 251),Point2i(292, 252),Point2i(293, 253),Point2i(293, 254),Point2i(294, 255),Point2i(294, 256),Point2i(295, 257),Point2i(295, 258),Point2i(296, 259),Point2i(296, 260),Point2i(297, 261),Point2i(297, 262),Point2i(298, 263),Point2i(298, 264),Point2i(299, 265),Point2i(299, 266),Point2i(300, 267),Point2i(300, 268),Point2i(301, 269),Point2i(301, 270),Point2i(302, 271),Point2i(302, 272),Point2i(303, 273),Point2i(303, 274),Point2i(304, 275),Point2i(304, 277),Point2i(305, 278),Point2i(305, 279),Point2i(306, 280),Point2i(306, 281),Point2i(307, 282),Point2i(307, 283),Point2i(309, 285),Point2i(309, 286),Point2i(310, 287),Point2i(310, 288),Point2i(311, 289),Point2i(311, 291),Point2i(312, 292),Point2i(312, 293),Point2i(313, 294),Point2i(313, 295),Point2i(314, 296),Point2i(314, 297),Point2i(315, 298),Point2i(315, 299),Point2i(317, 301),Point2i(317, 302),Point2i(318, 303),Point2i(318, 304),Point2i(319, 305),Point2i(319, 306),Point2i(320, 307),Point2i(320, 309),Point2i(321, 310),Point2i(321, 311),Point2i(322, 312),Point2i(322, 313),Point2i(323, 314),Point2i(323, 315),Point2i(324, 316),Point2i(324, 317),Point2i(325, 318),Point2i(325, 319),Point2i(326, 320),Point2i(326, 321),Point2i(327, 322),Point2i(327, 323),Point2i(328, 324),Point2i(328, 325),Point2i(329, 326),Point2i(329, 327),Point2i(330, 328),Point2i(330, 329),Point2i(331, 330),Point2i(331, 331),Point2i(332, 332),Point2i(332, 333),Point2i(333, 334),Point2i(333, 335),Point2i(334, 336),Point2i(334, 337),Point2i(335, 338),Point2i(335, 339),Point2i(336, 340),Point2i(336, 342),Point2i(337, 343),Point2i(337, 344),Point2i(338, 345),Point2i(338, 346),Point2i(339, 347),Point2i(339, 348),Point2i(340, 349),Point2i(340, 350),Point2i(341, 351),Point2i(341, 352),Point2i(342, 353),Point2i(342, 354),Point2i(343, 355),Point2i(343, 356),Point2i(344, 357),Point2i(344, 358),Point2i(345, 359),Point2i(345, 360),Point2i(346, 361),Point2i(346, 362),Point2i(347, 363),Point2i(347, 364),Point2i(348, 365),Point2i(348, 366),Point2i(349, 367),Point2i(349, 368),Point2i(350, 369),Point2i(350, 370),Point2i(351, 371),Point2i(351, 372),Point2i(353, 374),Point2i(353, 375),Point2i(354, 376),Point2i(354, 377),Point2i(355, 378),Point2i(355, 379),Point2i(356, 380),Point2i(356, 381),Point2i(357, 382),Point2i(357, 383),Point2i(358, 384),Point2i(358, 385),Point2i(359, 386),Point2i(359, 387),Point2i(360, 388),Point2i(360, 389),Point2i(361, 390),Point2i(361, 391),Point2i(362, 392),Point2i(362, 393),Point2i(364, 395),Point2i(364, 396),Point2i(365, 397),Point2i(365, 398),Point2i(366, 399),Point2i(366, 400),Point2i(367, 401),Point2i(367, 402),Point2i(368, 403),Point2i(368, 404),Point2i(369, 405),Point2i(369, 407),Point2i(370, 408),Point2i(370, 409),Point2i(371, 410),Point2i(371, 411),Point2i(372, 412),Point2i(372, 414),Point2i(373, 415),Point2i(373, 416),Point2i(374, 417),Point2i(374, 418),Point2i(375, 419),Point2i(375, 420),Point2i(376, 421),Point2i(376, 422),Point2i(377, 423),Point2i(377, 424),Point2i(378, 425),Point2i(378, 426),Point2i(379, 427),Point2i(379, 428),Point2i(380, 429),Point2i(380, 430),Point2i(381, 431),Point2i(381, 432),Point2i(382, 433),Point2i(382, 434),Point2i(383, 435),Point2i(383, 436),Point2i(384, 437),Point2i(384, 438),Point2i(385, 439),Point2i(385, 440),Point2i(387, 442),Point2i(387, 443),Point2i(388, 444),Point2i(388, 445),Point2i(390, 447),Point2i(390, 448),Point2i(395, 453),Point2i(397, 453),Point2i(398, 454),Point2i(399, 454),Point2i(400, 455),Point2i(402, 455),Point2i(403, 456),Point2i(411, 456),Point2i(413, 454),Point2i(414, 454),Point2i(415, 453),Point2i(416, 453),Point2i(421, 448),Point2i(421, 447),Point2i(423, 445),Point2i(423, 444),Point2i(424, 443),Point2i(424, 442),Point2i(425, 441),Point2i(425, 439),Point2i(426, 438),Point2i(426, 436),Point2i(427, 435),Point2i(427, 433),Point2i(428, 432),Point2i(428, 429),Point2i(429, 428),Point2i(429, 426),Point2i(430, 425),Point2i(430, 422),Point2i(431, 421),Point2i(431, 419),Point2i(432, 418),Point2i(432, 415),Point2i(433, 414),Point2i(433, 411),Point2i(434, 410),Point2i(434, 408),Point2i(435, 407),Point2i(435, 406),Point2i(436, 405),Point2i(436, 400),Point2i(437, 399),Point2i(437, 397),Point2i(438, 396),Point2i(438, 393),Point2i(439, 392),Point2i(439, 390),Point2i(440, 389),Point2i(440, 387),Point2i(441, 386),Point2i(441, 384),Point2i(442, 383),Point2i(442, 382),Point2i(443, 381),Point2i(443, 375),Point2i(444, 374),Point2i(444, 372),Point2i(445, 371),Point2i(445, 368),Point2i(446, 367),Point2i(446, 365),Point2i(447, 364),Point2i(447, 362),Point2i(448, 361),Point2i(448, 358),Point2i(449, 357),Point2i(449, 355),Point2i(450, 354),Point2i(450, 353),Point2i(451, 352),Point2i(451, 349),Point2i(452, 348),Point2i(452, 345),Point2i(453, 344),Point2i(453, 342),Point2i(454, 341),Point2i(454, 340),Point2i(455, 339),Point2i(455, 338),Point2i(456, 337),Point2i(456, 336),Point2i(458, 334),Point2i(458, 333),Point2i(469, 322),Point2i(470, 322),Point2i(472, 320),Point2i(473, 320),Point2i(475, 318),Point2i(476, 318),Point2i(478, 316),Point2i(479, 316),Point2i(480, 315),Point2i(481, 315),Point2i(483, 313),Point2i(484, 313),Point2i(485, 312),Point2i(486, 312),Point2i(488, 310),Point2i(489, 310),Point2i(491, 308),Point2i(492, 308),Point2i(493, 307),Point2i(494, 307),Point2i(496, 305),Point2i(497, 305),Point2i(498, 304),Point2i(499, 304),Point2i(501, 302),Point2i(502, 302),Point2i(503, 301),Point2i(504, 301),Point2i(506, 299),Point2i(507, 299),Point2i(508, 298),Point2i(509, 298),Point2i(511, 296),Point2i(512, 296),Point2i(513, 295),Point2i(514, 295),Point2i(516, 293),Point2i(517, 293),Point2i(518, 292),Point2i(519, 292),Point2i(521, 290),Point2i(522, 290),Point2i(523, 289),Point2i(524, 289),Point2i(526, 287),Point2i(527, 287),Point2i(528, 286),Point2i(529, 286),Point2i(531, 284),Point2i(532, 284),Point2i(533, 283),Point2i(534, 283),Point2i(536, 281),Point2i(537, 281),Point2i(538, 280),Point2i(539, 280),Point2i(541, 278),Point2i(542, 278),Point2i(543, 277),Point2i(544, 277),Point2i(546, 275),Point2i(547, 275),Point2i(548, 274),Point2i(549, 274),Point2i(551, 272),Point2i(552, 272),Point2i(554, 270),Point2i(555, 270),Point2i(565, 260),Point2i(565, 259),Point2i(566, 258),Point2i(566, 257),Point2i(567, 256),Point2i(567, 254),Point2i(568, 253),Point2i(568, 246),Point2i(567, 245),Point2i(567, 244),Point2i(566, 243),Point2i(566, 242),Point2i(563, 239),Point2i(563, 238),Point2i(562, 238),Point2i(558, 234),Point2i(557, 234),Point2i(556, 233),Point2i(555, 233),Point2i(554, 232),Point2i(552, 232),Point2i(551, 231),Point2i(549, 231),Point2i(548, 230),Point2i(544, 230),Point2i(543, 229),Point2i(538, 229),Point2i(537, 228),Point2i(531, 228),Point2i(530, 227),Point2i(525, 227),Point2i(524, 226),Point2i(518, 226),Point2i(517, 225),Point2i(512, 225),Point2i(511, 224),Point2i(505, 224),Point2i(504, 223),Point2i(498, 223),Point2i(497, 222),Point2i(491, 222),Point2i(490, 221),Point2i(483, 221),Point2i(482, 220),Point2i(477, 220),Point2i(476, 219),Point2i(470, 219),Point2i(469, 218),Point2i(464, 218),Point2i(463, 217),Point2i(458, 217),Point2i(457, 216),Point2i(452, 216),Point2i(451, 215),Point2i(444, 215),Point2i(443, 214),Point2i(438, 214),Point2i(437, 213),Point2i(430, 213),Point2i(429, 212),Point2i(424, 212),Point2i(423, 211),Point2i(417, 211),Point2i(416, 210),Point2i(411, 210),Point2i(410, 209),Point2i(404, 209),Point2i(403, 208),Point2i(397, 208),Point2i(396, 207),Point2i(390, 207),Point2i(389, 206),Point2i(382, 206),Point2i(381, 205),Point2i(375, 205),Point2i(374, 204),Point2i(369, 204),Point2i(368, 203),Point2i(360, 203),Point2i(359, 202),Point2i(353, 202),Point2i(352, 201),Point2i(348, 201),Point2i(347, 200),Point2i(341, 200),Point2i(340, 199),Point2i(338, 199),Point2i(337, 198),Point2i(330, 198),Point2i(329, 197),Point2i(324, 197),Point2i(323, 196),Point2i(319, 196),Point2i(318, 195),Point2i(313, 195),Point2i(312, 194),Point2i(307, 194),Point2i(306, 193),Point2i(293, 193),Point2i(292, 194)};//{Point2i(487,162),Point2i(262,356),Point2i(532,418),Point2i(471,296)};


void opencv_save_config()
{
  cv::FileStorage file;
    file.open(CONFIGFILENAME, cv::FileStorage::WRITE);
    if (file.isOpened())
    {
        // Write to file!
        //file << "warp" << Trans;
        file << "threshold" << opencv_config.Threshold_Slider;
        file << "threshold_size" << opencv_config.Threshold_size;
        file << "blur" << opencv_config.Blur_Value;
        file << "dilation" << opencv_config.dilation_size;
        file << "theta" << opencv_config.thetaOffset;
        file << "polyfit" << opencv_config.polyfit;
        file << "match" << opencv_config.matchValue;
        file.release();
        printf("Saved config\r\n");
    }
}
void opencv_read_config()
{
  cv::FileStorage file;
    file.open(CONFIGFILENAME, cv::FileStorage::READ);
	  if (file.isOpened()==true)
	  {
        file["threshold"] >> opencv_config.Threshold_Slider;
        file["threshold_size"] >> opencv_config.Threshold_size;
        file["blur"] >>  opencv_config.Blur_Value;
        file["dilation"] >>  opencv_config.dilation_size;
        file["theta"] >>  opencv_config.thetaOffset;
        file["polyfit"] >>  opencv_config.polyfit;
        file["match"] >>  opencv_config.matchValue;
         file.release();
	  } else
      {
          printf("failed to open config\r\n");
      }
}




VideoCapture cap;

Scalar colorFilter_min = Scalar(0,10,140), colorFilter_max = Scalar(180,255,255);

void SetColorFilter(bool max, int h, int s, int v)
{
    if (max)
    {
        colorFilter_max = Scalar(h,s,v);
    }
    else
    {
        colorFilter_min = Scalar(h,s,v);
    }
}
void GetColorFilter(bool max, int *h, int *s, int *v)
{
    if (max)
    {
        *h  = colorFilter_max.val[0];
        *s  = colorFilter_max.val[1];
        *v  = colorFilter_max.val[2];
    }
    else
    {
        *h  = colorFilter_min.val[0];
        *s  = colorFilter_min.val[1];
        *v  = colorFilter_min.val[2];
    }
}


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
    char posText[4];
    for (int i=0;i<numSamples;i++)
    {
        center.x = radius*cos((startAngle+i*(360.0/(float)numSamples))*M_PI/180.0) + knob.x;
        center.y = radius*sin((startAngle+i*(360.0/(float)numSamples))*M_PI/180.0) + knob.y;
         cv::circle	(canvas, center, 40, Scalar(200,0,0),-1,LINE_8,0);
        sprintf(posText,"%d",i+1);
        putText(canvas, posText, center, FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255), 1, LINE_AA);
    }
}


bool FlattenImage( Mat& inputFrame, Mat& OutputTransform)
{
// this converts the warpped image to a grey color space for further processing
   // cvtColor(warpped, gray , COLOR_BGRA2GRAY,0);
    std::vector<Point2f> corner_pts;
    bool success = findChessboardCorners(inputFrame, Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK  | CALIB_CB_NORMALIZE_IMAGE);
    if(success)
    {  
        cv::TermCriteria criteria(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, .001);
        // redefine the checkerboard corners for better accuracy
        cornerSubPix(inputFrame,corner_pts,Size(11,11), Size(-1,-1),criteria);
        vector<Point2f> points;
        // create a bounding box of just the corners
        int corner_pts_size = corner_pts.size();
        points.push_back(corner_pts[0]);
        points.push_back(corner_pts[(corner_pts_size/4)-1]);
        points.push_back(corner_pts[corner_pts_size-1]);
        points.push_back(corner_pts[((3*corner_pts_size)/4)]);

        vector<Point2f> warpPoints;
        int scale=5; // this determines how big the flatten image will appear
        // this will center the image
        int dist = inputFrame.rows/scale; 
        int mid_y = inputFrame.cols/2;
        int mid_x = inputFrame.rows/2;
        // creating the position for the new image
        // the smaller this new position is, the more zoomed out it will appear
        warpPoints.push_back(Point(mid_y-dist,mid_x-dist));
        warpPoints.push_back(Point(mid_y+dist,mid_x-dist));
        warpPoints.push_back(Point(mid_y+dist,mid_x+dist));
        warpPoints.push_back(Point(mid_y-dist,mid_x+dist));
        // creates the transform that will be used later on
        OutputTransform = getPerspectiveTransform(points,warpPoints);
        std::cout << "Trans: " << OutputTransform << endl;
        return true;
    }
    return false;
}

void CalibrateThread(void * args)
{

}

bool CalibrateLens(Mat& inputFrame, Mat& OutMap1, Mat &OutMap2)
{
    Mat objp;
    Mat gray;
    static int calCount=0;
  // Creating vector to store vectors of 3D points for each checkerboard image
 static  std::vector<std::vector<cv::Point3f> > objpoints;
  // Creating vector to store vectors of 2D points for each checkerboard image
 static std::vector<std::vector<cv::Point2f> > imgpoints;
  for(int i{0}; i<CHECKERBOARD[1]; i++)
  {
    for(int j{0}; j<CHECKERBOARD[0]; j++)
      objp.push_back(cv::Point3f(j,i,0));
  }
    // chessboard requires a gray image
    cvtColor(inputFrame, gray , COLOR_BGRA2GRAY,0);
    std::vector<Point2f> corner_pts;
    bool success = findChessboardCorners(gray, Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, CALIB_CB_ADAPTIVE_THRESH | CALIB_CB_FAST_CHECK  | CALIB_CB_NORMALIZE_IMAGE);
    if(success)
    {
        // refining pixel coordinates for given 2d points.
        cv::TermCriteria criteria(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, .001);
        cornerSubPix(gray,corner_pts,Size(11,11), Size(-1,-1),criteria);


        // draw the found chessboard on the color frame
        drawChessboardCorners(inputFrame, Size(CHECKERBOARD[0], CHECKERBOARD[1]), corner_pts, success);


        std::vector<std::vector<cv::Point3f> > objpoints_buffer;
        // Creating vector to store vectors of 2D points for each checkerboard image
        std::vector<std::vector<cv::Point2f> > imgpoints_buffer;
        objpoints_buffer.push_back(objp);
        imgpoints_buffer.push_back(corner_pts);

        cv::TermCriteria criteria2(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, 1e-6);
        cv::Mat K, D;
        std::vector<cv::Mat> rvecs, tvecs;
        int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC|cv::fisheye::CALIB_FIX_SKEW;
        // attempt to calibrate a single image, if it fails we know that it wasn't a good image.
        // even though we found the chessboard on this frame for some incredibly dumb reason we can still fail to find it on calibrate
        try {
            cv::fisheye::calibrate(objpoints_buffer, imgpoints_buffer, gray.size(), K, D, rvecs, tvecs, flags, criteria2);
        } catch(Exception e){
            printf("Calibration Failed %d\r\n",calCount);
            return false;
        } 
        printf("Calibrated SUccess %d\r\n",calCount);
        objpoints.push_back(objp);
        imgpoints.push_back(corner_pts);
        calCount++;
    }
    // now that we have enough images
    // try to calibrate for real
    if (calCount>100)
    {         
        cv::TermCriteria criteria(cv::TermCriteria::EPS|cv::TermCriteria::MAX_ITER, 30, 1e-6);

        Mat K,D;
        std::vector<cv::Mat> rvecs, tvecs;
        int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC|cv::fisheye::CALIB_FIX_SKEW;
        try {
            cv::fisheye::calibrate(objpoints, imgpoints, gray.size(), K, D, rvecs, tvecs, flags, criteria);
        } catch(Exception e){
            printf("Calibration Failed\r\n");
            return false;
        } 
        printf("Calibrated Success\r\n");
        cv::fisheye::initUndistortRectifyMap(K, D, cv::Mat::eye(3, 3, CV_32F), K, gray.size(), CV_16SC2, OutMap1, OutMap2);
        std::cout
        << "K=" << K << std::endl
        << "D=" << D << std::endl;
       // calibrated=true;
        //Calibration=false;
        calCount=0;
        return true;
    }
}

cv::Point3d findOrientation(const vector<Point>& src)
{
      cv::Moments m = cv::moments(src, true);
      double cen_x=m.m10/m.m00;
      double cen_y=m.m01/m.m00;

	  double fix;
      double u11_ = (m.m11/m.m00) - (cen_x*cen_y);
      double u20_ = (m.m20/m.m00) - (cen_x*cen_x);
      double u02_ = (m.m02/m.m00) - (cen_y*cen_y);


     double m_11= 2.0*m.m11-m.m00*(cen_x*cen_x+cen_y*cen_y);// m.mu11/m.m00;    
     double m_02=m.m02-m.m00*cen_y*cen_y;// m.mu02/m.m00;
     double m_20=m.m20-m.m00*cen_x*cen_x;//m.mu20/m.m00;    
     double theta = (m_20==m_02)?0:atan2(m_11, m_20-m_02)/2.0;
    

    //   double theta=0;
    //   if ((u20_ - u02_)!=0)
    //   {
    // 	  double x = (u20_ - u02_);
    // 	  double y = (2.0 * u11_);
    // 	  theta =  atan2 (y,x)/2.0;
           theta = (theta / M_PI) * 180.0 ;
    	//   if ((x<0 && y>0) ||(x>0 && y<0))
    	//   {
    	// 	  fix = 90-theta;
    	//   }
    	//   else
    	//   if ((x>0 && y>0) || (x<0 && y<0))
    	//   {
    	// 	  fix = 270- theta;
    	//   }
     // }

      //printf("%f, %f, %f, %f \r\n",(2.0 * u11_),(u20_ - u02_),theta,fix);

      //line(drawing,Point(cen_x,cen_y),Point(cen_x-(u20_ - u02_),cen_y-(2.0 * u11_)),Scalar(0,255,0),2);

    return cv::Point3d(cen_x,cen_y,theta);
}

void processKnobMatch(vector<Point> &Knob, Mat &OutputFrame)
{
    Point knob_center;
    cv::Moments m = cv::moments(Knob, true);
    double cen_x=m.m10/m.m00;
    double cen_y=m.m01/m.m00;
    knob_center = Point(cen_x,cen_y);

    // this finds the point closest to the center of the contour
    // because the shape of the knob has a point near the center, it must be pointing that way
    // double minDistance=DBL_MAX;
    // int minDistancePoint=0;
    // for (int j=0; j<Knob.size();j++)
    // {
    //     Point p = Knob[j];
    //     double res = norm(p-knob_center);
    //     if (res<minDistance)
    //     {
    //         minDistance = res;
    //         minDistancePoint = j;
    //     }
    // }

    // double buffer = 180+(atan2(Knob[minDistancePoint].y-knob_center.y,Knob[minDistancePoint].x-knob_center.x)*180.0/M_PI);
    double theta = findOrientation(Knob).z;
//double drawTheta = theta+(((double)opencv_config.thetaOffset-18000.0)/100.0);
    // static double theta;
    // theta = .75*theta + 0.25*buffer;

    //  avgAngle = theta;// 0.9*avgAngle + 0.1*theta;
    // printf("%f\r\n",avgAngle);
    char angleText[15];
    //sprintf(angleText,"%f = %f - %f",avgAngle,mp.z,mu.z);
    sprintf(angleText,"%0.02f",theta);
    putText(OutputFrame, angleText, Point(8*OutputFrame.size().width/10,OutputFrame.size().height-10), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255), 1, LINE_AA);
    fillPoly(OutputFrame,Knob,Scalar(0,0,255));
   // line(OutputFrame,knob_center,Knob[minDistancePoint],Scalar(0,255,0));

    vector<Point> Tri;
    minEnclosingTriangle(Knob,Tri);
    polylines(OutputFrame,Tri,true,Scalar(255,0,0));
    // find the shortest line
    line(OutputFrame,Point(OutputFrame.cols/2-10,OutputFrame.rows/2),Point(OutputFrame.cols/2+10,OutputFrame.rows/2),Scalar(0,255,0),5);
    line(OutputFrame,Point(OutputFrame.cols/2,OutputFrame.rows/2-10),Point(OutputFrame.cols/2,OutputFrame.rows/2+10),Scalar(0,255,0),5);

    
    line(OutputFrame,Point(cen_x-10,cen_y),Point(cen_x+10,cen_y),Scalar(0,0,0),5);
    line(OutputFrame,Point(cen_x,cen_y-10),Point(cen_x,cen_y+10),Scalar(0,0,0),5);

    //draw_tray(OutputFrame, 20, 300, knob_center,drawTheta);

    
    
}

void *CamUpdate(void *arg)
{
   lv_obj_t *img = (lv_obj_t*)arg;
Mat frame;
Mat gray;
Mat warpped,warpped_blur; 
Mat input;
Mat drawing;
Mat outputFrame;
int count = 1;

init_camera();

    lv_img_dsc_t imgDsc;

  std::vector<cv::Point3f> objp;
  
   cv::Mat map1, map2;
   double tran_c[3][3] = {{0.9887319429809974, 0.05716665516063728, -117.719050132904},{ -0.07706136048259898, 0.9687507839912129, 47.90186573630533},{ 1.14939671416363e-05, -7.609421690938215e-05, 1}};

   cv::Mat trans = cv::Mat(3,3,CV_64F,&tran_c);
    bool calibrated=true;
    bool flatten=true;
   int calCount=0;
   double K_c[3][3] = { {418.5248911727859, 0, 396.5416920228854},{0, 417.3716543944635, 271.7286752066629},{0, 0, 1}};
   double D_c[4] = { -0.04456047071614869, 0.2307124415380794, -0.7307970854849617, 0.6943486286581442};
   Mat K=cv::Mat(3, 3, CV_64F, &K_c);
   Mat D=cv::Mat(4, 1, CV_64F, &D_c);
cv::Size s;
s.width=800;
s.height=600;
cv::fisheye::initUndistortRectifyMap(K, D, cv::Mat::eye(3, 3, CV_32F), K, s, CV_16SC2, map1, map2);
  
           drawing = Mat::zeros( s, CV_8UC3 );
           input = Mat::zeros( s, CV_8UC3 );
           warpped_blur = Mat::zeros(s,CV_8UC3);


	  cv::FileStorage file(SHAPEFILENAME, cv::FileStorage::READ);
	  if (file.isOpened()==true)
	  {
		  file["match"] >> ShapeToFind;
	  }
   // drawKeypoints(shapeFrame,keypoints_object,shapeFrame,Scalar(0,255,0));
    Mat ShapeFrame = Mat::zeros(s,CV_8UC3);

    vector<Point> Tri;
    //active_camera_mode = 69;
    // polylines(ShapeFrame,ShapeToFind,true,Scalar(0,255,0));
    // minEnclosingTriangle(ShapeToFind,Tri);
    // polylines(ShapeFrame,Tri,true,Scalar(255,0,0));




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
        // if the flatten button is on, 
        // flatten the image to the calibration checkboard
        if (Flatten == true)
        {
            cvtColor(warpped, gray , COLOR_BGRA2GRAY,0);
            if (FlattenImage( gray, trans)==true)
            {
                Flatten = false;
                flatten = true;
            }
        }
        // apply blur to the de-warpped image so we can see how it looks
        if (opencv_config.Blur_Value>0 && count==0)
        {
            // blur values must be odd
            if (opencv_config.Blur_Value%2==0)
                opencv_config.Blur_Value++;
            medianBlur(warpped,warpped_blur,opencv_config.Blur_Value);
        }
        else
        {
            warpped_blur = warpped;
        }
        // process the warpped image into grayscale
        // the count is here because of a bug that causes a crash. i don't know why it is required
        if (opencv_config.Threshold_Slider>0 || count == 1)
        {
            count=0;
            cvtColor(warpped_blur,gray,COLOR_BGRA2GRAY,0);
             //threshold(gray,gray,opencv_config.Threshold_Slider, 255, 0);
            // adaptiveThreshold(gray,gray,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,5*2+1,1);
            // create a color filtered image for knob processing
            // this will threshold the image using a HSV color filter
           
           // inRange(warpped_blur, colorFilter_min, colorFilter_max, gray);
            // lets try canny edge detection
            //cvtColor(warpped, gray , COLOR_BGRA2GRAY,0);
             Canny(gray,gray,opencv_config.Threshold_Slider,opencv_config.Threshold_Slider*opencv_config.Threshold_size,7,false);
             Mat element = getStructuringElement( MORPH_RECT ,Size( 2*opencv_config.dilation_size + 1, 2*opencv_config.dilation_size+1 ) );
             morphologyEx(gray,gray, MORPH_CLOSE, element,Point(-1,-1),1);
        }
        else
        {
            cvtColor(warpped_blur, gray , COLOR_BGRA2GRAY,0);
            // this does gaussian thresholding, it will take small patches of the screen and figure out magically the best value to use to threshold
            // but its output is kind of noisy
            adaptiveThreshold(gray,gray,255,ADAPTIVE_THRESH_GAUSSIAN_C,THRESH_BINARY,opencv_config.Threshold_size*2+1,1);
            // so we have to anneal the output back into contours
            Mat element = getStructuringElement( MORPH_RECT ,Size( 2*opencv_config.dilation_size + 1, 2*opencv_config.dilation_size+1 ),Point( opencv_config.dilation_size, opencv_config.dilation_size ) );
            morphologyEx(gray,gray, MORPH_CLOSE, element);
        }

			   vector<vector<Point> > contours;
		   vector<Vec4i> hierarchy;
		   findContours( gray, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1 );
           drawing = Mat::zeros( gray.size(), CV_8UC3 );
           input = Mat::zeros(gray.size(),CV_8UC3);
           double closestMatch = DBL_MAX, closestShapeMatch=DBL_MAX;
           int found=-1;
            int foundShape = 0;
		   for( size_t i = 0; i< contours.size(); i++ )
           {
               Scalar color = Scalar( 133, 133, 133 );
			   approxPolyDP	(contours[i],contours[i],((double)opencv_config.polyfit/1000.0) * arcLength(contours[i], true),true);
            //    if (ContourCalibrate )
            //    {
                  double testMatch = cv::pointPolygonTest	(	contours[i],cursorPos,true );
                  if (testMatch>=0 && testMatch<closestMatch)
                  {
                      closestMatch = testMatch;
                      found = i;
                  }
               //}
               if (ShapeToFind.size() > 1)
               {
                   // start trying to find the contour
                    double ret = matchShapes(contours[i],ShapeToFind,CONTOURS_MATCH_I1,0.0);
                    // limit the match so we dont select everything

                    if (ret<(opencv_config.matchValue/1000.0))
                    {     
                        if (ret<closestShapeMatch)
                        {
                            foundShape = i;
                            closestShapeMatch=ret;
                        }
                    }
               }
			    drawContours( input, contours, i, color, 2, LINE_8, hierarchy, 0 );
               // fillPoly(input,contours[i],Scalar(0,0,255));
            }
            if (found>=0 )
            {
			   drawContours( input, contours, found, Scalar( 33, 200, 33 ), 2, LINE_8, hierarchy, 0 );
               if ( ContourCalibrate)
               {
                ShapeToFind.clear();
                for (int i=0; i<contours[found].size();i++)
                {
                    ShapeToFind.push_back(contours[found][i]);
                }

                 cv::FileStorage file;
                file.open(SHAPEFILENAME, cv::FileStorage::WRITE);
                if (file.isOpened())
                {
                // Write to file!
                //file << "warp" << Trans;
                file << "match" << ShapeToFind;
                file.release();
                printf("Saved SHape\r\n");
                }
                else
                {
                    printf("Failed to save shape\r\n");
                }
              // cout << "Shape: " << ShapeToFind << endl;
               ContourCalibrate = false;
               }
            } 
            if (foundShape==false)
            {
                if (TrayPresentCounter>0)
                {
                    TrayPresentCounter--;
                } else
                {                         
                    char trayNotPresentText[] = "Tray Not Present";
                    putText(drawing, trayNotPresentText, Point2i(drawing.size().width/2,drawing.size().height/2), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255), 1, LINE_AA);
                }
            } else
            {
                processKnobMatch(contours[foundShape], drawing);
			    drawContours( input, contours, foundShape, Scalar( 255, 0, 0 ), 2, LINE_8, hierarchy, 0 );
               // draw confidence


                char trayConfidenceText[15];
                sprintf(trayConfidenceText,"%0.5f",closestShapeMatch);
                putText(drawing, trayConfidenceText, Point2i(drawing.size().width/10,drawing.size().height-10), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255), 1, LINE_AA);
               
               if (TrayPresentCounter==0)
                {
                    foundQRcode = false;
                }
                if (foundQRcode==false)
                {
                    std::string data = qrDecoder.detectAndDecode(frame,noArray(),noArray());
                    if(data.length()>0)
                    {
                        cout << "Decoded Data : " << data << endl;
                        foundQRcode = true;
                    }
                }
                TrayPresentCounter = TrayPresentResetVal;
            }
        }


        // time to process thermal data
        thermal_image_t thermalImage;
        thermal_color_image_t colorThermalImage;
        thermal_getframe(thermalImage);
        thermal_colorImage(thermalImage,colorThermalImage);
        Mat thermal = Mat(24,32, CV_8UC3, colorThermalImage);
        rotate(thermal,thermal,ROTATE_90_CLOCKWISE);

        
    char thermalText[15];
    //sprintf(angleText,"%f = %f - %f",avgAngle,mp.z,mu.z);
    float thermalScale_w = (float)gray.size().width/(float)MLX90640_SENSOR_W;
    float thermalScale_h = (float)gray.size().height/(float)MLX90640_SENSOR_H;
float tempAtPos = thermal_getTempAtPoint(cursorPos.y/thermalScale_h,MLX90640_SENSOR_W-cursorPos.x/thermalScale_w );
    sprintf(thermalText,"%0.2f",tempAtPos);
    
        resize(thermal,thermal,gray.size());
        // mix colors now
        float beta = ( 1.0 - thermalAlpha/255.0 );
   addWeighted( warpped, thermalAlpha/255.0, thermal, beta, 0.0, thermal);
   
    putText(thermal, thermalText, cursorPos, FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255), 1, LINE_AA);

    // double theta=0;
    // 


        // upscale to our size

    switch (active_camera_mode)
    {
        case 69:
            cvtColor(ShapeFrame,outputFrame,COLOR_BGR2BGRA,0);
            break;
        case 5:
            cvtColor(thermal,outputFrame,COLOR_BGR2BGRA,0);
            break;
        case 4: 
        //cvtColor(matchImg,outputFrame,COLOR_BGR2BGRA,0);
          cvtColor(drawing,outputFrame,COLOR_BGR2BGRA,0);
            break;
        case 3:
            cvtColor(input,outputFrame,COLOR_BGR2BGRA,0);
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

    // start thermal
    thermal_init();
}