
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
#include <pthread.h>
#include "grpc.h"

    lv_obj_t * kb;
    lv_obj_t *tabview;
    lv_obj_t * handleImg;
    pthread_t grpc_thread;



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
lv_obj_t * slider;

void slider_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * img = lv_event_get_user_data(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {
        LV_LOG_USER("Angle: %d",lv_slider_get_value(ta));
        lv_img_set_angle(img, lv_slider_get_value(ta));
    }
}

    static lv_style_t style, style_sel;
void create_cannon_application(void)
{
	 LV_FONT_DECLARE(Orbitron_120);
//     int iret1 = pthread_create( &grpc_thread, NULL,c_RunServer, NULL);


     

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
    lv_png_init();
    
    lv_style_init(&style);
    lv_style_set_radius(&style, 50);
    lv_style_set_bg_color(&style,lv_palette_main(LV_PALETTE_BLUE));
    lv_style_init(&style_sel);
    lv_style_set_radius(&style_sel, 50);
    lv_style_set_bg_color(&style_sel,lv_palette_main(LV_PALETTE_GREEN));
    for (int i=0;i<20;i++)
    {
        Create_Circle(canvas, 360*i/20, SDL_HOR_RES/2-100-20);
    }


    handleImg = lv_img_create(tab1);
    lv_img_set_src(handleImg, "c:/../handle.png");
    lv_obj_align(handleImg, LV_ALIGN_CENTER, 0, 0);
    lv_point_t pivot;
    lv_img_get_pivot(handleImg, &pivot);
    //pivot.y *=1.2;
    lv_img_set_pivot(handleImg, pivot.x, pivot.y);    /*Rotate around the top left corner*/
    //lv_obj_set_size(handleImg,240,240);
    
    //  slider = lv_slider_create(tab1);
    // lv_obj_set_flex_grow(slider, 1);
    // lv_slider_set_range(slider, 0, 360);
    // lv_slider_set_value(slider, 0, LV_ANIM_OFF);
    // lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, handleImg);



    label = lv_label_create(tab2);
    lv_label_set_text(label, "Second tab");
    /*Add content to the tabs*/

    create_settings_tab(tab3);

    //lv_obj_scroll_to_view_recursive(label, LV_ANIM_ON);


}


void sample_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    int num = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED)
    {
        int angle = num*360/20;
        LV_LOG_USER("Button %d %d",num,angle);
        //lv_slider_set_value(slider,angle,LV_ANIM_ON);
        //lv_event_send(slider,LV_EVENT_VALUE_CHANGED,handleImg);
        lv_img_set_angle(handleImg, angle);
        c_run_Client();
    }
}
void Create_Circle(lv_obj_t *Parent, int angle, int radius)
{
    float offset = -90;
    int x = radius * cos((angle+offset)*M_PI/180.0) + (SDL_HOR_RES-100)/2;
    int y = radius * sin((angle+offset)*M_PI/180.0) + (SDL_HOR_RES-100)/2;

    lv_obj_t *btn = lv_btn_create(Parent);
    //lv_obj_remove_style_all(btn);       
    lv_obj_add_style(btn, &style, 0);    
    lv_obj_add_style(btn, &style_sel, LV_STATE_CHECKED);
    lv_obj_add_event_cb(btn, sample_event_handler, LV_EVENT_ALL, angle*20/360);
    lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(btn,100,100);
    lv_obj_set_pos(btn,x,y);
    return btn;
}

void ta_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    lv_obj_t * kb = lv_event_get_user_data(e);
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