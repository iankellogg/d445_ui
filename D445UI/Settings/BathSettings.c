#include "Settings/BathSettings.h"
#include "settings.h"
#include "cannon.h"


// void dec_max_event_cb(lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     lv_obj_t * ta = lv_event_get_target(e);
//     const char *insertedText = lv_event_get_param(e);
//     const char *txt = lv_textarea_get_text(ta);
//     // eat the input
//     lv_textarea_set_insert_replace(ta, "");
// // capture backspace
//     if (insertedText=='\177')
//     {
//         lv_textarea_
//     }
// }

lv_obj_t *Create_BathSettings_Page(lv_obj_t *menu)
{

    lv_obj_t * page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(page);
    lv_obj_t *section = lv_menu_section_create(page);


    lv_obj_t * obj = create_text(section, NULL, "Temp Set Point: ", 1);



    lv_obj_t *TempSetPoint= lv_textarea_create(obj);
    lv_textarea_set_one_line(TempSetPoint, true);
    lv_textarea_set_text(TempSetPoint, "100.00");
    lv_textarea_set_accepted_chars(TempSetPoint,"0123456789.");
    lv_obj_set_width(TempSetPoint,lv_pct(25));
    lv_textarea_set_text_selection(TempSetPoint, true);
    lv_textarea_set_max_length(TempSetPoint,6);
    lv_obj_add_event_cb(TempSetPoint, ta_event_cb, LV_EVENT_ALL, kb);
    //lv_obj_add_event_cb(TempSetPoint, dec_max_event_cb, LV_EVENT_INSERT, NULL);
       // lv_obj_set_pos(TempSetPoint, LV_ALIGN_RIGHT_MID,0, 0);


    
     obj = create_text(section, NULL, "White LED", 0);
     lv_obj_t * slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
     obj = create_text(section, NULL, "Red LED", 0);
     
     slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
     obj = create_text(section, NULL, "Green LED", 0);
slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);
     obj = create_text(section, NULL, "Blue LED", 0);
slider = lv_slider_create(obj);
    lv_obj_set_flex_grow(slider, 1);
    lv_slider_set_range(slider, 0, 100);
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);



    return page;
}


lv_obj_t * Create_BathSettings_Menu(lv_obj_t *Parent, lv_obj_t *Menu)
{
    lv_obj_t *page = Create_BathSettings_Page(Menu);
    lv_obj_t * cont = create_text(Parent, LV_SYMBOL_SETTINGS, "Bath Settings", 0);
    lv_menu_set_load_page_event(Menu, cont, page);

    return page;
}