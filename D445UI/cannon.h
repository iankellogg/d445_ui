/*
 * cannon.h
 *
 *  Created on: Oct 23, 2021
 *      Author: ikellogg
 */

#ifndef CANNON_H_
#define CANNON_H_

#include <libintl.h>
#include <locale.h>
#include "lvgl/lvgl.h"

#define _(STRING) gettext(STRING)


    extern lv_obj_t * kb;
    void ta_event_cb(lv_event_t * e);

void create_cannon_application(void);

#endif /* CANNON_H_ */