/**
 * Copyright Alexandru Olaru
 * See LICENSE file for copyright and license details
 */

#ifndef DMENU_APPS_H
#define DMENU_APPS_H

#include <stddef.h>
#include <X11/Xlib.h>
#include "drw.h"

int dmenu_apps_parse(Drw *drw, size_t icon_wh);
void dmenu_apps_cleanup();
struct item *dmenu_get_app_items();

#endif // !DMENU_APPS_H
