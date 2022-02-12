
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "Settings/camera.h"
#include "cannon.h"

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

static int32_t Brightness = 100;

void colorwheel_cb(lv_event_t * e);
void Cam_Click_CB(lv_event_t * e);
void White_CB(lv_event_t * e);

lv_obj_t *Create_Camera_Page(lv_obj_t *menu)
{

    lv_obj_t * page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(page);
    lv_obj_t *section = lv_menu_section_create(page);

  lv_obj_set_size(section, lv_pct(100), lv_pct(100));



	// cvtColor(frame, frame, COLOR_BGR2BGRA,0);
    // uint length = frame.total()*frame.channels();
    // uchar * arr = frame.isContinuous()? frame.data: frame.clone().data;
    lv_obj_t *img_btn = lv_btn_create(section);
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
    
    lv_obj_t * cont2 = lv_obj_create(section);

    lv_obj_set_flex_flow(cont2, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(cont2, lv_pct(100), 100);
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

    lv_obj_t * rows = lv_obj_create(section);
    lv_obj_set_flex_flow(rows, LV_FLEX_FLOW_COLUMN);
   lv_obj_align_to(rows,cont2,LV_ALIGN_OUT_BOTTOM_LEFT,0,0);
  lv_obj_set_size(rows, lv_pct(100), lv_pct(100));
    
    /** Button Row **/
    lv_obj_t * button_row = lv_obj_create(rows);
    lv_obj_set_flex_flow(button_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_size(button_row, lv_pct(100), 100);
   //lv_obj_set_x(cont2, lv_pct(50));
   // button to pause video feed
   //******************************************** BUttons ********************************/
   lv_obj_t *PauseButton = lv_btn_create(button_row);
    lv_obj_add_flag(PauseButton, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(PauseButton, LV_SIZE_CONTENT);
    lv_obj_t *label = lv_label_create(PauseButton);
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
    lv_obj_add_event_cb(FlattenButton, button_event_cb, LV_EVENT_CLICKED, &flt_btn);


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
        lv_label_set_text(row, "Thermal Alpha");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    lv_obj_t *slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 255);
    lv_slider_set_value(slider, thermalAlpha, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, &thermalAlpha);

   row = lv_label_create(rows);
     lv_obj_set_height(row,50);
     lv_obj_set_width(row,lv_pct(100));
        lv_label_set_text(row, "Match Value");
        lv_label_set_long_mode(row, LV_LABEL_LONG_SCROLL_CIRCULAR);
        //lv_obj_set_flex_grow(row, 1);
    slider = lv_slider_create(row);
     lv_obj_align(slider,LV_ALIGN_TOP_RIGHT,0,25);
    //lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, opencv_config.matchValue, LV_ANIM_OFF);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_ALL, &opencv_config.matchValue);

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
   

    return page;
}


lv_obj_t * Create_Camera_Menu(lv_obj_t *Parent, lv_obj_t *Menu)
{
    lv_obj_t *page = Create_Camera_Page(Menu);
    lv_obj_t * cont = create_text(Parent, LV_SYMBOL_EYE_OPEN, "Camera", 0);
    lv_menu_set_load_page_event(Menu, cont, page);



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


    return page;
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