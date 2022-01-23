#include "Settings/TestMethods.h"
#include "settings.h"

static lv_obj_t * list1;

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked: %s", lv_list_get_btn_text(list1, obj));
    }
}

   static  lv_style_t style_title, style_btn;
void Create_TestMethod_List_Btn(lv_obj_t *list, const char *Name)
{

    lv_obj_t * btn;
    btn = lv_list_add_btn(list, NULL, Name);
    lv_obj_add_style(btn, &style_title, 0);
    lv_obj_t *Btn = lv_btn_create(btn);

        lv_obj_t *img = lv_img_create(Btn);
        lv_img_set_src(img, LV_SYMBOL_EDIT);
    lv_obj_add_style(Btn, &style_btn, 0);
    Btn = lv_btn_create(btn);

        img = lv_img_create(Btn);
        lv_img_set_src(img, LV_SYMBOL_CLOSE);
    lv_obj_add_style(Btn, &style_btn, 0);

    lv_obj_set_size(btn,lv_pct(100),lv_pct(5));
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

}

lv_obj_t *Create_TestMethod_Page(lv_obj_t *menu)
{
    lv_style_init(&style_title);
    lv_style_init(&style_btn);

    lv_obj_t * page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(page);
    lv_obj_t *section = lv_menu_section_create(page);

/*Create a list*/
    list1 = lv_list_create(page);
    lv_obj_set_size(list1, lv_pct(100), lv_pct(100));
    lv_obj_center(list1);

    /*Add buttons to the list*/

    lv_list_add_text(list1, "Test Methods");
    lv_style_set_text_font(&style_title, &lv_font_montserrat_24);
    lv_style_set_text_font(&style_btn, &lv_font_montserrat_14);

    Create_TestMethod_List_Btn(list1,"Verify KV");
    Create_TestMethod_List_Btn(list1,"Verify KV");
    Create_TestMethod_List_Btn(list1,"Verify KV");



    return page;
}


lv_obj_t * Create_TestMethod_Menu(lv_obj_t *Parent, lv_obj_t *Menu)
{
    lv_obj_t *page = Create_TestMethod_Page(Menu);
   lv_obj_t * cont = create_text(Parent, LV_SYMBOL_LIST, "Test Methods", 0);
    lv_menu_set_load_page_event(Menu, cont, page);
    return page;
}