
#include <Settings/settings.h>
#include "Settings/TestMethods.h"
#include "Settings/BathSettings.h"
#include "Settings/Network.h"

static lv_obj_t *tab;
static lv_obj_t * list1;
static lv_obj_t *settings_content;


static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
    }
}


static void back_event_handler(lv_event_t * e);
static void switch_handler(lv_event_t * e);
lv_obj_t * root_page;
static lv_obj_t * create_slider(lv_obj_t * parent,
                                   const char * icon, const char * txt, int32_t min, int32_t max, int32_t val);
static lv_obj_t * create_switch(lv_obj_t * parent,
                                   const char * icon, const char * txt, bool chk);


/**
 * Bath Control
 *      Bath Set point
 *      Temperature Offset
 *      PID Control
 *      Temperature Plot
 *  Display
 *         Theme Settings
 * Stage Lighting
 *      Red Slider
 *      Green Slider
 *      Blue Slider
 *      White Slider
 *  Motor Control
 *      Bath Lift Up/Down
 *         Current Sample Height    (Save Current)
 *          Current Cleaning Station Height  (Save Current)
 *          Current Atlantic Tube Drop Height  (Save Current)
 *      Manual Bath Latch
 *      Cleaning Station Left/Right
 *      Tray Position Control
 *          When on Position 1, Left/Right Manual control (Save Current)
 *        
 * Camera Settings
 *      ** Live Camera **
 *      Pause Button        Preprocess/Post Process toggle
 *         Thermal Camera Opacity Slider
 *          Thermal Camera X Offset Slider
 *          Thermal Camera Y Offset Slider
 *          Shape Training, 
 *         
 *  
 */


void create_settings_tab(lv_obj_t *tabRef)
{
    tab = tabRef;
    lv_obj_clear_flag(tab, LV_OBJ_FLAG_SCROLLABLE);

lv_obj_t * menu = lv_menu_create(tab);
   lv_obj_set_size(menu, lv_pct(30),lv_pct(100));
   lv_obj_set_style_pad_top(menu,50,LV_PART_MAIN);
    lv_color_t bg_color = lv_obj_get_style_bg_color(menu, 0);
    if(lv_color_brightness(bg_color) > 127) {
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 10), 0);
    }else{
        lv_obj_set_style_bg_color(menu, lv_color_darken(lv_obj_get_style_bg_color(menu, 0), 50), 0);
    }
    // this adds the back button that 
    lv_menu_set_mode_root_back_btn(menu, LV_MENU_ROOT_BACK_BTN_ENABLED);
    lv_obj_add_event_cb(menu, back_event_handler, LV_EVENT_CLICKED, menu);
    lv_obj_set_size(menu, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_center(menu);

    lv_obj_t * cont;
    lv_obj_t * section;

    /*Create sub pages*/

    lv_obj_t * sub_sound_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_sound_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_sound_page);
    section = lv_menu_section_create(sub_sound_page);
    create_switch(section, LV_SYMBOL_AUDIO, "Sound", false);

    lv_obj_t * sub_display_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_display_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_display_page);
    section = lv_menu_section_create(sub_display_page);
    create_slider(section, LV_SYMBOL_SETTINGS, "Brightness", 0, 150, 100);

    lv_obj_t * sub_software_info_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_software_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(sub_software_info_page);
    create_text(section, NULL, "Version 1.0", 0);

    lv_obj_t * sub_legal_info_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_legal_info_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    section = lv_menu_section_create(sub_legal_info_page);
    for(uint32_t i=0; i<15; i++){
        create_text(section, NULL, "This is a long long long long long long long long long text, if it is long enough it may scroll.", 0);
    }

    lv_obj_t * sub_about_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_about_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_about_page);
    section = lv_menu_section_create(sub_about_page);
    cont = create_text(section, NULL, "Software information", 0);
    lv_menu_set_load_page_event(menu, cont, sub_software_info_page);
    cont = create_text(section, NULL, "Legal information", 0);
    lv_menu_set_load_page_event(menu, cont, sub_legal_info_page);

    lv_obj_t * sub_menu_mode_page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(sub_menu_mode_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(sub_menu_mode_page);
    section = lv_menu_section_create(sub_menu_mode_page);
    cont = create_switch(section, LV_SYMBOL_AUDIO, "Sidebar enable", true);
    lv_obj_add_event_cb(lv_obj_get_child(cont, 2), switch_handler, LV_EVENT_VALUE_CHANGED, menu);

    /*Create a root page*/
    root_page = lv_menu_page_create(menu, "Settings");
    lv_obj_set_style_pad_hor(root_page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);


    create_text(root_page, NULL, "Instrument", 0);
    section = lv_menu_section_create(root_page); 
    lv_obj_t *TestMethod_Page = Create_TestMethod_Menu(section,menu);
    lv_obj_t *BathSettings_Page = Create_BathSettings_Menu(section,menu);
    lv_obj_t *NetworkSettings_Page = Create_NetworkSettings_Menu(section,menu);
    


    create_text(root_page, NULL, "Service", 0);
    section = lv_menu_section_create(root_page);
    cont = create_text(section, LV_SYMBOL_PLUS, "Temperature", 0);
    lv_menu_set_load_page_event(menu, cont, sub_about_page);
    cont = create_text(section, LV_SYMBOL_EYE_OPEN, "Camera", 0);
    lv_menu_set_load_page_event(menu, cont, sub_about_page);
    cont = create_text(section, LV_SYMBOL_REFRESH, "Motor", 0);
    lv_menu_set_load_page_event(menu, cont, sub_about_page);

    lv_menu_set_sidebar_page(menu, root_page);
    lv_menu_set_page(menu, TestMethod_Page);

    lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
}


static void back_event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_obj_t * menu = lv_event_get_user_data(e);
    static bool displayMenu=true;

    if(lv_menu_back_btn_is_root(menu, obj)) {
        if (displayMenu)
        {

            lv_menu_set_page(menu, lv_menu_get_cur_main_page(menu));
            lv_menu_set_sidebar_page(menu, NULL);
            lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
            displayMenu=false;
        }
        else
        {
            lv_menu_set_page(menu, lv_menu_get_cur_main_page(menu));
            lv_menu_set_sidebar_page(menu, root_page);
            displayMenu=true;

         }
        // lv_obj_t * mbox1 = lv_msgbox_create(NULL, "Hello", "Root back btn click.", NULL, true);
        // lv_obj_center(mbox1);
    }
}

static void switch_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * menu = lv_event_get_user_data(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        if(lv_obj_has_state(obj, LV_STATE_CHECKED)) {
            lv_menu_set_page(menu, NULL);
            lv_menu_set_sidebar_page(menu, root_page);
            lv_event_send(lv_obj_get_child(lv_obj_get_child(lv_menu_get_cur_sidebar_page(menu), 0), 0), LV_EVENT_CLICKED, NULL);
        }else {
            lv_menu_set_sidebar_page(menu, NULL);
            lv_menu_clear_history(menu); /* Clear history because we will be showing the root page later */
            lv_menu_set_page(menu, root_page);
        }
    }
}

lv_obj_t * create_text(lv_obj_t * parent, const char * icon, const char * txt,
                                        uint8_t builder_variant)
{
    lv_obj_t * obj = lv_menu_cont_create(parent);

    lv_obj_t * img = NULL;
    lv_obj_t * label = NULL;

    if(icon) {
        img = lv_img_create(obj);
        lv_img_set_src(img, icon);
    }

    if(txt) {
        label = lv_label_create(obj);
        lv_label_set_text(label, txt);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_flex_grow(label, 1);
    }

    if(builder_variant == 1 && icon && txt) {
        lv_obj_add_flag(img, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        lv_obj_swap(img, label);
    }

    return obj;
}

static lv_obj_t * create_slider(lv_obj_t * parent, const char * icon, const char * txt, int32_t min, int32_t max, int32_t val)
{
    lv_obj_t * obj = create_text(parent, icon, txt, 1);

    lv_obj_t * slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, min, max);
    lv_slider_set_value(slider, val, LV_ANIM_OFF);

    if(icon == NULL) {
        lv_obj_add_flag(slider, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
    }

    return obj;
}

static lv_obj_t * create_switch(lv_obj_t * parent, const char * icon, const char * txt, bool chk)
{
    lv_obj_t * obj = create_text(parent, icon, txt, 0);

    lv_obj_t * sw = lv_switch_create(obj);
    lv_obj_add_state(sw, chk ? LV_STATE_CHECKED : 0);

    return obj;
}
