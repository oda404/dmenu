
#ifndef DMENU_ITEM_H
#define DMENU_ITEM_H

#include <X11/Xlib.h>
#include "types.h"
#include "drw.h"

struct item
{
    char *text;
    struct item *left, *right;
    int out;

    char *exec_cmd;
    char *icon_path;

    Img *img;
};

#endif // !DMENU_ITEM_H
