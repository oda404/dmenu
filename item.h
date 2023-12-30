
#ifndef DMENU_ITEM_H
#define DMENU_ITEM_H

#include <X11/Xlib.h>
#include "types.h"

struct item
{
    char *text;
    struct item *left, *right;
    int out;

    char *exec_cmd;
    char *icon_path;

    XImage *ximage;
    u32 ximage_w;
    u32 ximage_h;
};

#endif // !DMENU_ITEM_H
