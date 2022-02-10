
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
#include <stdio.h>
#include <math.h>
// PIGPIOD
#include <pigpiod_if2.h>

#include "opencv.h"
static int32_t hPIGPIO = 0;

static int32_t max_duty;
static const uint32_t white = 4;
static const uint32_t red    = 17;
static const uint32_t green  = 22;
static const uint32_t blue   = 27;
static const uint32_t freq   = 1000;

typedef struct
{
    char *Text;
    char *ToggledText;
    int32_t *Value;
} button_cb_t;


static int32_t Brightness = 100;


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
void White_CB(lv_event_t * e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    int32_t *val = (int32_t*)lv_event_get_user_data(e);
    
    *val = lv_slider_get_value(ta);
     set_PWM_dutycycle(hPIGPIO,white,(*val)*max_duty/100.0);
    set_PWM_dutycycle(hPIGPIO,red,0);
    set_PWM_dutycycle(hPIGPIO,blue,0);
    set_PWM_dutycycle(hPIGPIO,green,0);
}
void Cam_Click_CB(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_point_t p;
    lv_obj_t *val = (lv_obj_t*)lv_event_get_user_data(e);
//        uint16_t btn_pr = LV_BTNMATRIX_BTN_NONE;
    /*Search the pressed area*/
    if (code == LV_EVENT_RELEASED)
    {
        lv_indev_t *indev = lv_indev_get_act();
        lv_indev_type_t indev_type = lv_indev_get_type(indev);
        if (indev_type == LV_INDEV_TYPE_ENCODER || indev_type == LV_INDEV_TYPE_KEYPAD) return LV_RES_OK;

        lv_indev_get_point(indev, &p);
        p.x = p.x-val->coords.x1;
        p.y=p.y-val->coords.y1;
        printf("Clicked on image: x: %d y: %d\r\n",p.x,p.y);
        Send_Contour_Cursor_Pos(p.x,p.y);
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

void colorwheel_cb(lv_event_t * e)
{
    lv_obj_t *ta = lv_event_get_target(e);
    lv_obj_t *val = (lv_obj_t*)lv_event_get_user_data(e);
    
    lv_slider_set_value(val,0,false);
    lv_color_t color = lv_colorwheel_get_rgb(ta);
    set_PWM_dutycycle(hPIGPIO,red,color.ch.red*max_duty/255);
    set_PWM_dutycycle(hPIGPIO,blue,color.ch.blue*max_duty/255);
    set_PWM_dutycycle(hPIGPIO,green,color.ch.green*max_duty/255);
    
}

void colorfilter_cb(lv_event_t *e)
{
    
    lv_obj_t *ta = lv_event_get_target(e);
    bool val = (bool)lv_event_get_user_data(e);
    
    lv_color_hsv_t color = lv_colorwheel_get_hsv(ta);
    int32_t h = color.h/2;
    int32_t s = color.s*255.0/100.0;
    int32_t v = color.v*255.0/100.0;
    printf ("H: %d S: %d V: %d\r\n",h,s,v);
    SetColorFilter(val,h,s,v);
    

}


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

void color_radio_cb(lv_event_t * e)
{
     lv_obj_t * cw = (lv_obj_t*)lv_event_get_user_data(e);
    lv_obj_t * cont = lv_event_get_current_target(e);
    lv_obj_t * act_cb = lv_event_get_target(e);

    /*Do nothing if the container was clicked*/
    if(act_cb == cont) return;
    for( int i = 0; i < lv_obj_get_child_cnt(cont); i++) {
    lv_obj_t * child = lv_obj_get_child(cont, i);
        lv_obj_clear_state(child, LV_STATE_CHECKED);   /*Uncheck the previous radio button*/
    /*Do something with child*/
    }

    lv_obj_add_state(act_cb, LV_STATE_CHECKED);     /*Uncheck the current radio button*/
    lv_colorwheel_set_mode(cw,lv_obj_get_index(act_cb));
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


    static lv_style_t style, style_sel;
void create_cannon_application(void)
{
	 LV_FONT_DECLARE(Orbitron_120);
//     int iret1 = pthread_create( &grpc_thread, NULL,c_RunServer, NULL);


opencv_read_config();

     hPIGPIO = pigpio_start(NULL, NULL);
     if (hPIGPIO<0)
     {
         printf("Unable to connect to PIGPIOD\r\n");
     }


     set_PWM_frequency(hPIGPIO,white, freq);
     set_PWM_frequency(hPIGPIO,red, freq);
     set_PWM_frequency(hPIGPIO,blue, freq);
     set_PWM_frequency(hPIGPIO,green, freq);
     max_duty = get_PWM_real_range(hPIGPIO,white);
     set_PWM_dutycycle(hPIGPIO,white,max_duty);
    set_PWM_dutycycle(hPIGPIO,red,0);
    set_PWM_dutycycle(hPIGPIO,blue,0);
    set_PWM_dutycycle(hPIGPIO,green,0);

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


    lv_obj_t * cont2 = lv_obj_create(tab1);

    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(cont2, 200, lv_pct(15));
  //lv_obj_set_flex_grow(cont2, 1);
   //lv_obj_set_x(cont2, lv_pct(50));
   lv_obj_align(cont2,LV_ALIGN_TOP_RIGHT,0,0);
    lv_obj_add_event_cb(cont2, radio_event_handler, LV_EVENT_CLICKED, &active_camera_mode);
    lv_obj_t * chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Raw");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Warped");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Input");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Processed");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "AI");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_state(chbox, LV_STATE_CHECKED);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Thermal");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);

	// cvtColor(frame, frame, COLOR_BGR2BGRA,0);
    // uint length = frame.total()*frame.channels();
    // uchar * arr = frame.isContinuous()? frame.data: frame.clone().data;
    lv_obj_t *img_btn = lv_btn_create(tab1);
    lv_obj_remove_style_all(img_btn);
     lv_obj_t *img = lv_img_create(img_btn);
   static  lv_img_dsc_t imgDsc;
    static const uint8_t emptyFrame[800*600*4];
    imgDsc.data = emptyFrame;
    imgDsc.data_size = 0;
    imgDsc.header.h = 600;
    imgDsc.header.w = 800;
    imgDsc.header.always_zero=0;
    imgDsc.header.cf = LV_IMG_CF_TRUE_COLOR;
     lv_img_set_src(img,&imgDsc);

    start_camera_thread(img);

    lv_obj_t * rows = lv_obj_create(tab1);
    lv_obj_set_flex_flow(rows, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(rows, lv_pct(100), lv_pct(100));
   lv_obj_align_to(rows,img,LV_ALIGN_OUT_BOTTOM_LEFT,0,0);
    
    /** Button Row **/
    lv_obj_t * button_row = lv_obj_create(rows);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(button_row, lv_pct(100), lv_pct(10));
   //lv_obj_set_x(cont2, lv_pct(50));
   // button to pause video feed
   //******************************************** BUttons ********************************/
   lv_obj_t *PauseButton = lv_btn_create(button_row);
    lv_obj_add_flag(PauseButton, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(PauseButton, LV_SIZE_CONTENT);
    label = lv_label_create(PauseButton);
    lv_label_set_text(label, "Pause");
    static button_cb_t pause_btn = {.Text = "Pause",.ToggledText="Cancel",.Value=&Pause};
    lv_obj_add_event_cb(PauseButton, button_event_cb, LV_EVENT_VALUE_CHANGED, &pause_btn);
    lv_obj_center(label);

   lv_obj_t *CalibrationButton = lv_btn_create(button_row);
   lv_obj_add_flag(CalibrationButton, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(CalibrationButton, LV_SIZE_CONTENT);
    label = lv_label_create(CalibrationButton);
    lv_label_set_text(label, "Calibrate");
    lv_obj_center(label);
    static button_cb_t cal_btn = {.Text = "Calibrate",.ToggledText="Cancel",.Value=&Calibration};
    lv_obj_add_event_cb(CalibrationButton, button_event_cb, LV_EVENT_VALUE_CHANGED, &cal_btn);

   lv_obj_t *FlattenButton = lv_btn_create(button_row);
    lv_obj_set_height(FlattenButton, LV_SIZE_CONTENT);
    label = lv_label_create(FlattenButton);
    lv_label_set_text(label, "Flatten");
    lv_obj_center(label);
    static button_cb_t flt_btn = {.Text = "Flatten",.ToggledText="Cancel",.Value=&Flatten};
    lv_obj_add_event_cb(FlattenButton, button_event_cb, LV_EVENT_VALUE_CHANGED, &flt_btn);


   lv_obj_t *ContourButton = lv_btn_create(button_row);
    lv_obj_set_height(ContourButton, LV_SIZE_CONTENT);
    label = lv_label_create(ContourButton);
    lv_label_set_text(label, "Contour");
    lv_obj_center(label);
            lv_obj_add_event_cb(img_btn, Cam_Click_CB, LV_EVENT_ALL, img_btn);
    static button_cb_t contour_btn = {.Text = "Contour",.ToggledText="Cancel",.Value=&ContourCalibrate};
    lv_obj_add_event_cb(ContourButton, button_event_cb, LV_EVENT_CLICKED, &contour_btn);


   //******************************************** Sliders ********************************/
   //button for calibration
   // while calibration
   // threshold slider
     lv_obj_t *row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Threshold");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
       // lv_obj_set_flex_grow(row, 1);
     lv_obj_t *slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(threshold_slider, 1);
    lv_slider_set_range(slider, 0, 1000);
    lv_slider_set_value(slider, opencv_config.Threshold_Slider, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, &opencv_config.Threshold_Slider);
/************************* Color Filter *******************/
row = lv_label_create(rows);
     lv_obj_set_height(row,250);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Filter Color");
    lv_obj_t *cw_hsv_min= lv_colorwheel_create(row, true);
    lv_obj_set_size(cw_hsv_min, 200, 200);
    lv_colorwheel_set_mode(cw_hsv_min,LV_COLORWHEEL_MODE_HUE);
    lv_obj_set_style_arc_width(cw_hsv_min,25,LV_PART_MAIN);
    lv_obj_set_x(cw_hsv_min,300);
    lv_color_hsv_t hsv;
    int h,s,v;
    lv_obj_add_event_cb(cw_hsv_min, colorfilter_cb, LV_EVENT_VALUE_CHANGED, 0);
    GetColorFilter(0,&h,&s,&v);
    hsv.h = h*2;
    hsv.s = s*(255.0/100.0);
    hsv.v = v*(255.0/100.0);
    lv_colorwheel_set_hsv(cw_hsv_min,hsv);
    lv_obj_t *cw_hsv_max= lv_colorwheel_create(row, true);
    lv_obj_set_size(cw_hsv_max, 200, 200);
    lv_colorwheel_set_mode(cw_hsv_max,LV_COLORWHEEL_MODE_HUE);
    lv_obj_set_style_arc_width(cw_hsv_max,25,LV_PART_MAIN);
    lv_obj_align_to(cw_hsv_max,cw_hsv_min,LV_ALIGN_OUT_RIGHT_MID,25,0);
    lv_obj_add_event_cb(cw_hsv_max, colorfilter_cb, LV_EVENT_VALUE_CHANGED, 1);
    GetColorFilter(1,&h,&s,&v);
    hsv.h = h*2;
    hsv.s = s*(255.0/100.0);
    hsv.v = v*(255.0/100.0);
    lv_colorwheel_set_hsv(cw_hsv_max,hsv);


    cont2 = lv_obj_create(row);
    lv_obj_align_to(cont2,cw_hsv_min,LV_ALIGN_OUT_LEFT_MID,0,0);
    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(cont2, 120, 200);
  //lv_obj_set_flex_grow(cont2, 1);
   //lv_obj_set_x(cont2, lv_pct(50));

    lv_obj_add_event_cb(cont2, color_radio_cb, LV_EVENT_CLICKED, cw_hsv_min);
   chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Hue");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_state(chbox, LV_STATE_CHECKED);
    // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Sat");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Val");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);

    
    cont2 = lv_obj_create(row);
    lv_obj_align_to(cont2,cw_hsv_max,LV_ALIGN_OUT_RIGHT_MID,0,0);
    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(cont2, 120, 200);
  //lv_obj_set_flex_grow(cont2, 1);
   //lv_obj_set_x(cont2, lv_pct(50));
    lv_obj_add_event_cb(cont2, color_radio_cb, LV_EVENT_CLICKED, cw_hsv_max);
   chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Hue");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_state(chbox, LV_STATE_CHECKED);
    // lv_obj_add_style(obj, &style_radio, LV_PART_INDICATOR);
    // lv_obj_add_style(obj, &style_radio_chk, LV_PART_INDICATOR | LV_STATE_CHECKED);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Sat");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);
    chbox = lv_checkbox_create(cont2);
    lv_checkbox_set_text(chbox, "Val");
    lv_obj_add_flag(chbox, LV_OBJ_FLAG_EVENT_BUBBLE);

   // blur slider
   row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Auto Threshold Size");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(threshold_slider, 1);
    lv_slider_set_range(slider, 0, 1000);
    lv_slider_set_value(slider, opencv_config.Threshold_size, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, &opencv_config.Threshold_size);
   row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Dilation");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(threshold_slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, opencv_config.dilation_size, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, &opencv_config.dilation_size);


row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Blur");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 25);
    lv_slider_set_value(slider, opencv_config.Blur_Value, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb,LV_EVENT_ALL, &opencv_config.Blur_Value);

    
row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Poly Fit");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, opencv_config.polyfit, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, &opencv_config.polyfit);
row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Match");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 1000);
    lv_slider_set_value(slider, opencv_config.matchValue, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, &opencv_config.matchValue);
    
row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Thermal Alpha");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 255);
    lv_slider_set_value(slider, thermalAlpha, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, &thermalAlpha);
   
row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Angle Offset");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 36000);
    lv_slider_set_value(slider, opencv_config.thetaOffset, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, &opencv_config.thetaOffset);

row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Stage Brightness");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, Brightness, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, White_CB, LV_EVENT_VALUE_CHANGED, &Brightness);
   

row = lv_label_create(rows);
     lv_obj_set_height(row,250);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Stage color");
    lv_obj_t *cw_rgb = lv_colorwheel_create(row, true);
    lv_obj_set_size(cw_rgb, 200, 200);
    lv_colorwheel_set_mode(cw_rgb,LV_COLORWHEEL_MODE_HUE);
    lv_obj_set_style_arc_width(cw_rgb,25,LV_PART_MAIN);
    lv_obj_center(cw_rgb);
    lv_obj_add_event_cb(cw_rgb, colorwheel_cb, LV_EVENT_VALUE_CHANGED, slider);
    



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