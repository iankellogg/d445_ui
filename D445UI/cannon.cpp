
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

    lv_obj_t * kb;
    lv_obj_t *tabview;
    lv_obj_t * handleImg;
VideoCapture cap;
Mat frame;
    static lv_img_dsc_t imgDsc;



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

void CamUpdate(lv_timer_t * timer)
{
   lv_obj_t *img = (lv_obj_t*)timer->user_data;

        cap.read(frame);
	cvtColor(frame, frame, COLOR_BGR2BGRA,0);
    //uint length = frame.total()*frame.channels();
    uchar * arr = frame.isContinuous()? frame.data: frame.clone().data;
    imgDsc.data = arr;
    lv_img_set_src(img,&imgDsc);
   // lv_img_cache_invalidate_src(NULL);

}

    static lv_style_t style, style_sel;
void create_cannon_application(void)
{
	 LV_FONT_DECLARE(Orbitron_120);
//     int iret1 = pthread_create( &grpc_thread, NULL,c_RunServer, NULL);

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
    lv_obj_set_size(canvas,SDL_HOR_RES,SDL_HOR_RES);
    //lv_canvas_set_buffer(canvas, cbuf, SDL_HOR_RES,SDL_HOR_RES, LV_IMG_CF_TRUE_COLOR);
    lv_obj_center(canvas);
    //lv_canvas_fill_bg(canvas, lv_palette_lighten(LV_PALETTE_BLUE, 3), LV_OPA_TRANSP);
    //static Mat colorFrame;
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

	timer = lv_timer_create(CamUpdate, 60,  img);
   




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