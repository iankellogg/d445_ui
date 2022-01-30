#include "Settings/Network.h"
#include "settings.h"
#include "cannon.h"


#include <iwlib.h>

void Wifi_Scan(lv_obj_t *obj)
{
    wireless_scan_head head;
  wireless_scan *result;
  iwrange range;
  int sock;

  /* Open socket to kernel */
  sock = iw_sockets_open();

  /* Get some metadata to use for scanning */
  if (iw_get_range_info(sock, "wlan0", &range) < 0) {
    printf("Error during iw_get_range_info. Aborting.\n");
    return;
  }

  /* Perform the scan */
  if (iw_scan(sock, "wlan0", range.we_version_compiled, &head) < 0) {
    printf("Error during iw_scan. Aborting.\n");
    return;
  }

  /* Traverse the results */
  result = head.result;
  while (NULL != result) {
    create_text(obj,NULL,result->b.essid,1);
    result = result->next;
  }

}


lv_obj_t *Create_NetworkSettings_Page(lv_obj_t *menu)
{

    lv_obj_t * page = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(page, lv_obj_get_style_pad_left(lv_menu_get_main_header(menu), 0), 0);
    lv_menu_separator_create(page);
    lv_obj_t *section = lv_menu_section_create(page);

    
    lv_obj_t * obj = create_text(section, NULL, "Wired ", 1);
    lv_obj_t *sw = lv_switch_create(obj);
    
    obj = create_text(section, NULL, "Wireless ", 1);
    sw = lv_switch_create(obj);
    lv_obj_t *Wifi_Settings = lv_obj_create(section);
 /*Create a list*/
   lv_obj_set_flex_flow(Wifi_Settings, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(Wifi_Settings, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

     Wifi_Scan(Wifi_Settings);

    return page;
}


lv_obj_t *Create_NetworkSettings_Menu(lv_obj_t *Parent, lv_obj_t *Menu)
{
    
    lv_obj_t *page = Create_NetworkSettings_Page(Menu);
    lv_obj_t * cont = create_text(Parent, LV_SYMBOL_WIFI, "Network", 0);
    lv_menu_set_load_page_event(Menu, cont, page);
    return page;
}