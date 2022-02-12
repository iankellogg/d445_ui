
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
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

#include "opencv.h"

typedef struct 
{
    // the current angle of the tray
    float trayAngle;
    // the numeric position (1-20)
    int trayPosition;
    // how confident we are about the trays position
    // if this number is "low" issue a warning that maybe the tray or camera is dirty
    float trayConfidence;
    // if the tray is detected
    struct statusBits
    {
        bool trayPresent:1;
        bool TooDark:1; // camera can somehow tell that its too dark to see anything
        bool trayCantMove:1;   // tray tried to move but didn't
        bool trayWrongPosition:1;   // tray moved but can't get to the requested position
    };


} camera_status_t;




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



    label = lv_label_create(tab1);
    lv_obj_clear_flag(tab1, LV_OBJ_FLAG_SCROLLABLE);


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