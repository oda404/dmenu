
#ifndef DMENU_ITEM_H
#define DMENU_ITEM_H

struct item
{
    char *text;
    struct item *left, *right;
    int out;

    char *exec_cmd;
    char *icon_path;
};

#endif // !DMENU_ITEM_H
