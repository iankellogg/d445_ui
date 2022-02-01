
#include <stdlib.h>
#include <unistd.h>
#include <cannon.h>
#include <Settings/settings.h>
/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include <time.h>
#include <math.h>
// OPEN CV INCLUDES
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

static uint32_t active_camera_mode = 0;
static int32_t Threshold_Slider=0,Blur_Value=7;
lv_obj_t * threshold_slider;

 pthread_mutex_t lvgl_mutex;

    lv_obj_t * kb;
    lv_obj_t *tabview;
    lv_obj_t * handleImg;
VideoCapture cap;
    static lv_img_dsc_t imgDsc;
pthread_t CameraThread;


void slider_event_cb(lv_event_t * e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    int32_t *val = (int32_t*)lv_event_get_user_data(e);
    
    *val = lv_slider_get_value(ta);
}

void Create_Circle(lv_obj_t *Parent, int angle, int radius);
//#include "lvgl_helpers.h"
lv_obj_t *DateTime;
static bool stdTime=true;

void DateTime_Timer(lv_timer_t * timer)
{
  /*Use the user_data*/
  struct tm* t;
  time_t tm = time(NULL);
  t = localtime(&tm);

  int hour;
  char *pm;

  if (stdTime==true)
  {

	  hour = (t->tm_hour)%12;
	  if (hour==0)
		  hour=12;
	  if (t->tm_hour>11)
	  {
		  pm = "PM";
	  }
	  else
	  {
		  pm = "AM";
	  }
  }
  else
  {
	  hour = t->tm_hour;
	  pm = "";
  }
  lv_label_set_text_fmt(DateTime,"%d/%d/%d %d:%02d %s",t->tm_mon+1,t->tm_mday,1900+t->tm_year,hour,t->tm_min,pm);

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

static void radio_event_handler(lv_event_t * e)
{
    uint32_t * active_id = (uint32_t*)lv_event_get_user_data(e);
    lv_obj_t * cont = lv_event_get_current_target(e);
    lv_obj_t * act_cb = lv_event_get_target(e);
    lv_obj_t * old_cb = lv_obj_get_child(cont, *active_id);

    /*Do nothing if the container was clicked*/
    if(act_cb == cont) return;

    lv_obj_clear_state(old_cb, LV_STATE_CHECKED);   /*Uncheck the previous radio button*/
    lv_obj_add_state(act_cb, LV_STATE_CHECKED);     /*Uncheck the current radio button*/

    *active_id = lv_obj_get_index(act_cb);

    //LV_LOG_USER("Selected radio buttons: %d, %d", (int)active_index_1, (int)active_index_2);
}

void *CamUpdate(void *arg)
{
   lv_obj_t *img = (lv_obj_t*)arg;
Mat frame;
Mat gray;
Mat outputFrame;
int i = 1;
    while (1) 
    {

        cap.read(frame);
        if (frame.empty())
        {
            continue;
        }
        cvtColor(frame, gray , COLOR_BGRA2GRAY,0);
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
    pthread_mutex_lock(&lvgl_mutex);
        lv_img_set_src(img,&imgDsc);
    pthread_mutex_unlock(&lvgl_mutex);
    // lv_img_cache_invalidate_src(NULL);
      usleep(60 * 1000);
    }
}

    static lv_style_t style, style_sel;
void create_cannon_application(void)
{
	 LV_FONT_DECLARE(Orbitron_120);
//     int iret1 = pthread_create( &grpc_thread, NULL,c_RunServer, NULL);

Mat frame;

    init_camera();
    cap.read(frame);
    // check if we succeeded
    if (frame.empty()) {
       printf("ERROR! blank frame grabbed\n");
    }

     

    kb = lv_keyboard_create(lv_layer_top());
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);


	    lv_theme_default_init(NULL, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
    /* Get the current screen  */
    lv_obj_t * scr = lv_disp_get_scr_act(NULL);
    // Make the header that includes the time

  /*Create a Tab view object*/
    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 50);
    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_pad_left(tab_btns, LV_HOR_RES / 2, 0);
  
static lv_style_t style_title;
    lv_style_init(&style_title);
    lv_style_set_text_font(&style_title, &lv_font_montserrat_24);
        lv_obj_t * label = lv_label_create(tab_btns);
        lv_obj_add_style(label, &style_title, 0);
        lv_label_set_text(label, "D445 PP1");
        lv_obj_align(label, LV_ALIGN_LEFT_MID, -LV_HOR_RES / 2 + 25, 0);
        
        
	DateTime = lv_label_create(tab_btns);
	lv_timer_t * timer = lv_timer_create(DateTime_Timer, 1000,  DateTime);
	//lv_obj_align(DateTime,  LV_ALIGN_CENTER, 0, 0);
  lv_obj_align_to(DateTime, label, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);


    /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "Tray");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Results");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Settings"); 
    lv_obj_clear_flag(lv_tabview_get_content(tabview), LV_OBJ_FLAG_SCROLLABLE);



    label = lv_label_create(tab1);
    lv_obj_clear_flag(tab1, LV_OBJ_FLAG_SCROLLABLE);


//static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SDL_HOR_RES, SDL_HOR_RES)];

    lv_obj_t * canvas = lv_obj_create(tab1);
    lv_obj_set_size(canvas,lv_pct(100),lv_pct(100));
    //lv_canvas_set_buffer(canvas, cbuf, SDL_HOR_RES,SDL_HOR_RES, LV_IMG_CF_TRUE_COLOR);
    lv_obj_center(canvas);
    //lv_canvas_fill_bg(canvas, lv_palette_lighten(LV_PALETTE_BLUE, 3), LV_OPA_TRANSP);
    //static Mat colorFrame;


    lv_obj_t * cont2 = lv_obj_create(canvas);

    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(cont2, 200, lv_pct(10));
   //lv_obj_set_x(cont2, lv_pct(50));
   lv_obj_align(cont2,LV_ALIGN_TOP_RIGHT,0,0);
    lv_obj_add_event_cb(cont2, radio_event_handler, LV_EVENT_CLICKED, &active_camera_mode);
    lv_obj_t * chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Raw");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_state(chbox, LV_STATE_CHECKED);
    // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Warped");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Processed");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);

	cvtColor(frame, frame, COLOR_BGR2BGRA,0);
    uint length = frame.total()*frame.channels();
    uchar * arr = frame.isContinuous()? frame.data: frame.clone().data;
    lv_obj_t *img = lv_img_create(canvas);
    imgDsc.data = arr;
    imgDsc.data_size = length;
    imgDsc.header.h = frame.rows;
    imgDsc.header.w = frame.cols;
    imgDsc.header.always_zero=0;
    imgDsc.header.cf = LV_IMG_CF_TRUE_COLOR;
    lv_img_set_src(img,&imgDsc);
    pthread_create(&CameraThread, NULL, &CamUpdate, img);

    lv_obj_t * rows = lv_obj_create(canvas);
    lv_obj_set_flex_flow(rows, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(rows, lv_pct(100), lv_pct(100));
   lv_obj_align_to(rows,img,LV_ALIGN_OUT_BOTTOM_LEFT,0,0);
    
    /** Button Row **/
    lv_obj_t * button_row = lv_obj_create(rows);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(button_row, lv_pct(100), lv_pct(10));
   //lv_obj_set_x(cont2, lv_pct(50));
   // button to pause video feed
   lv_obj_t *PauseButton = lv_btn_create(button_row);
    //lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
  //  lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(PauseButton, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(PauseButton, LV_SIZE_CONTENT);
    label = lv_label_create(PauseButton);
    lv_label_set_text(label, "Pause");
    lv_obj_center(label);
   //button for calibration
   // while calibration
   // threshold slider
     lv_obj_t *row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Threshold");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
       // lv_obj_set_flex_grow(row, 1);
     threshold_slider = lv_slider_create(row);
     lv_obj_align(threshold_slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(threshold_slider, 1);
    lv_slider_set_range(threshold_slider, 0, 255);
    lv_slider_set_value(threshold_slider, Threshold_Slider, LV_ANIM_OFF);
    lv_obj_add_event_cb(threshold_slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, &Threshold_Slider);
   // blur slider

row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Blur");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    lv_obj_t *slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 25);
    lv_slider_set_value(slider, Blur_Value, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, &Blur_Value);
   





    label = lv_label_create(tab2);
    lv_label_set_text(label, "Second tab");
    /*Add content to the tabs*/

    create_settings_tab(tab3);

    //lv_obj_scroll_to_view_recursive(label, LV_ANIM_ON);


}

void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = (lv_obj_t*)lv_event_get_user_data(e);
    if(code == LV_EVENT_FOCUSED) {
        if(lv_indev_get_type(lv_indev_get_act()) != LV_INDEV_TYPE_KEYPAD) {
            lv_keyboard_set_textarea(kb, ta);
            lv_obj_set_style_max_height(kb, LV_HOR_RES * 2 / 4, 0);
            lv_obj_update_layout(tabview);   /*Be sure the sizes are recalculated*/
           // lv_obj_set_height(tabview, LV_VER_RES - lv_obj_get_height(kb));
            lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
            lv_obj_scroll_to_view_recursive(ta, LV_ANIM_OFF);
            if (strcmp(lv_textarea_get_accepted_chars(ta),"0123456789.")==0)
            {
                lv_keyboard_set_mode(kb,LV_KEYBOARD_MODE_NUMBER);
            } else
            {
                
                lv_keyboard_set_mode(kb,LV_KEYBOARD_MODE_TEXT_LOWER);
            }
        }
    }
    else if(code == LV_EVENT_DEFOCUSED) {
        lv_keyboard_set_textarea(kb, NULL);
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    }
    else if(code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_set_height(tabview, LV_VER_RES);
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_state(ta, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, ta);   /*To forget the last clicked object to make it focusable again*/
    }
}