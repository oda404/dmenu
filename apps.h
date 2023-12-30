/**
 * Copyright Alexandru Olaru
 * See LICENSE file for copyright and license details
 */

#ifndef DMENU_APPS_H
#define DMENU_APPS_H

#include <stddef.h>
#include <X11/Xlib.h>

int dmenu_apps_parse(Display *dpy, Visual *visual, size_t icon_wh);
void dmenu_apps_cleanup();
struct item *dmenu_get_app_items();

#endif // !DMENU_APPS_H
