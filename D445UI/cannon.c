

#include <stdlib.h>
#include <stdio.h> 
#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <cannon.h>
#include <Settings/settings.h>
/* Littlevgl specific */
#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include <time.h>
#include <stdio.h>
#include <math.h>
// PIGPIOD
#include <pigpiod_if2.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <string.h>
#include <errno.h> 
#include "opencv.h"



static const char klipperPath[] = "/tmp/printer";
int hFD; 


void drawHandle_cb(lv_timer_t * timer);


    lv_obj_t * kb;
    lv_obj_t *tabview;
    lv_obj_t * handleImg;


void slider_event_cb(lv_event_t * e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    int32_t *val = (int32_t*)lv_event_get_user_data(e);
    
    *val = lv_slider_get_value(ta);
    if (code ==  LV_EVENT_VALUE_CHANGED|| code==LV_EVENT_RELEASED)
    {
    // if we are done messing with the slider, tell opencv to save parameters
    if (lv_slider_is_dragged(ta)==false)
    {
        opencv_save_config();
    }
    printf("Slider: %d\r\n",*val);
    }
}

// void ContourBtn_CB(lv_event_t * e)
// {

//     lv_obj_t *ta = lv_event_get_target(e);
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t *val = (lv_obj_t*)lv_event_get_user_data(e);
    
//     // toggled state
//     if (code == LV_EVENT_VALUE_CHANGED)
//     {
//             lv_obj_t *label = lv_obj_get_child(ta,0);
//         if (lv_obj_get_state(ta)&LV_STATE_CHECKED == LV_STATE_CHECKED)
//         {
//             ContourCalibrate = true;   
//             lv_obj_add_event_cb(val, Cam_Click_CB, LV_EVENT_ALL, val);
//             lv_label_set_text(label,"Cancel"); 
//         }
//         else
//         {
//             ContourCalibrate = false;
//             lv_obj_remove_event_cb(val, Cam_Click_CB);
//             lv_label_set_text(label,"Contour");
//         }
//     }

// }




void button_event_cb(lv_event_t * e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    button_cb_t *val = (button_cb_t*)lv_event_get_user_data(e);
    
    // toggled state
    if (code == LV_EVENT_VALUE_CHANGED)
    {
            lv_obj_t *label = lv_obj_get_child(ta,0);
        if (lv_obj_get_state(ta)&LV_STATE_CHECKED == LV_STATE_CHECKED)
        {
            *val->Value = true;
            lv_label_set_text(label,val->ToggledText);
        }
        else
        {
            *val->Value = false;
            lv_label_set_text(label,val->Text);
        }
    }   else // toggled state
    if(code == LV_EVENT_CLICKED) 
    {
            *val->Value = true;
       
    }
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



void radio_event_handler(lv_event_t * e)
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


    static lv_style_t style, style_sel;
void create_cannon_application(void)
{
	 LV_FONT_DECLARE(Orbitron_120);
//     int iret1 = pthread_create( &grpc_thread, NULL,c_RunServer, NULL);


opencv_read_config();


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
        Create_Circle(canvas, i, SDL_HOR_RES/2-100-20);
    }


    handleImg = lv_img_create(tab1);
    lv_img_set_src(handleImg, "c:/../handle.png");
    lv_obj_align(handleImg, LV_ALIGN_CENTER, 0, 0);
    lv_point_t pivot;
    lv_img_get_pivot(handleImg, &pivot);
    //pivot.y *=1.2;
    lv_img_set_pivot(handleImg, pivot.x, pivot.y);    /*Rotate around the top left corner*/
    lv_obj_clear_flag(tab1, LV_OBJ_FLAG_SCROLLABLE);
    
	timer = lv_timer_create(drawHandle_cb, 100,  handleImg);


//static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(SDL_HOR_RES, SDL_HOR_RES)];

    // lv_obj_t * canvas = lv_obj_create(tab1);
    // lv_obj_set_size(canvas,lv_pct(100),lv_pct(100));
    // //lv_canvas_set_buffer(canvas, cbuf, SDL_HOR_RES,SDL_HOR_RES, LV_IMG_CF_TRUE_COLOR);
    // lv_obj_center(canvas);
    //lv_canvas_fill_bg(canvas, lv_palette_lighten(LV_PALETTE_BLUE, 3), LV_OPA_TRANSP);
    //static Mat colorFrame;


    



    label = lv_label_create(tab2);
    lv_label_set_text(label, "Second tab");
    /*Add content to the tabs*/

    create_settings_tab(tab3);

    //lv_obj_scroll_to_view_recursive(label, LV_ANIM_ON);

    
    hFD = open(klipperPath, O_RDWR ); 


}


void gotoPos(int pos)
{
    static int cp = -1;
    if (cp==-1)
        cp = (cameraStatus.trayPosition-1)*(7200.0/20.0)+7200.0;
    int p;
     int p1= (pos)*(7200.0/20.0)+round(cp/7200.0)*7200;
     int p2 = (pos-20)*(7200.0/20.0)+round(cp/7200.0)*7200;
    printf ("%d -> %d, %d -> %d or %d\r\n",cameraStatus.trayPosition,pos+1,cp,p1,p2);
    if (abs(p1-cp) > abs(p2-cp))
    {
        p = p2;
    }
    else
    {
        p = p1;
    }
    cp = p;
    char buffer[80];
     int c = snprintf(buffer,80,"MANUAL_STEPPER STEPPER=tray ENABLE=1 MOVE=%d\n",p);
          write(hFD,buffer,c);
}

void drawHandle_cb(lv_timer_t * timer)
{
    lv_obj_t *img = (lv_obj_t*)timer->user_data;

    float angle = cameraStatus.trayAngle;
    lv_img_set_angle(img, angle*10);
    
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
        gotoPos(num);
        //lv_slider_set_value(slider,angle,LV_ANIM_ON);
        //lv_event_send(slider,LV_EVENT_VALUE_CHANGED,handleImg);
        //
        //c_run_Client();
    }
}

void Create_Circle(lv_obj_t *Parent, int sample, int radius)
{
    float offset = -90;
    int angle = 360*sample/20;
    int x = radius * cos((angle+offset)*M_PI/180.0) + (SDL_HOR_RES-100)/2;
    int y = radius * sin((angle+offset)*M_PI/180.0) + (SDL_HOR_RES-100)/2;

    lv_obj_t *btn = lv_btn_create(Parent);
    //lv_obj_remove_style_all(btn);       
    lv_obj_add_style(btn, &style, 0);    
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text_fmt(label,"%d",sample+1);
    lv_obj_center(label);
    lv_obj_add_style(btn, &style_sel, LV_STATE_CHECKED);
    lv_obj_add_event_cb(btn, sample_event_handler, LV_EVENT_ALL, angle*20/360);
    //lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(btn,100,100);
    lv_obj_set_pos(btn,x,y);
    return btn;
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