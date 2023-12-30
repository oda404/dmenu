/**
 * Copyright Alexandru Olaru
 * See LICENSE file for copyright and license details
 */

#define _GNU_SOURCE // for DT_*
#include "apps.h"
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#include "item.h"

static const char *g_syswide_apps_path = "/usr/share/applications";
static struct item *g_apps;
static size_t g_app_count;

static bool str_endswith(const char *str, const char *end)
{
    char *f = strstr(str, end);
    if (!f)
        return NULL;

    return strlen(f) == strlen(end);
}

static int parse_desktop_file_value(const char *line, const char *name, char **out_value)
{
    /* Omit Allocation for null terminator since these lines should end with \n, and if not they'll have a letter cropped out */
    size_t value_len = strlen(line) - strlen(name);
    if (!value_len)
        return -1;

    *out_value = malloc(value_len * sizeof(char));
    if (!(*out_value))
        return -1;

    snprintf(*out_value, value_len, "%s", line + strlen(name));
    return 0;
}

static int find_icon_path_by_name(const char *name, char *out_path)
{
    char tmppath[PATH_MAX];
    snprintf(tmppath, PATH_MAX, "/usr/share/icons/hicolor/256x256/apps/%s.png", name);
    if (access(tmppath, F_OK) == 0)
    {
        snprintf(out_path, PATH_MAX, "%s", tmppath);
        return 0;
    }

    snprintf(tmppath, PATH_MAX, "/usr/share/icons/%s.png", name);
    if (access(tmppath, F_OK) == 0)
    {
        snprintf(out_path, PATH_MAX, "%s", tmppath);
        return 0;
    }

    snprintf(tmppath, PATH_MAX, "/usr/share/icons/hicolor/128x128/apps/%s.png", name);
    if (access(tmppath, F_OK) == 0)
    {
        snprintf(out_path, PATH_MAX, "%s", tmppath);
        return 0;
    }

    snprintf(tmppath, PATH_MAX, "/usr/share/pixmaps/%s.png", name);
    if (access(tmppath, F_OK) == 0)
    {
        snprintf(out_path, PATH_MAX, "%s", tmppath);
        return 0;
    }

    return -1;
}

#define TEARDOWN_APP(_app)        \
    {                             \
        if (_app.text)            \
            free(_app.text);      \
        if (_app.icon_path)       \
            free(_app.icon_path); \
        if (_app.exec_cmd)        \
            free(_app.exec_cmd);  \
    }

static int parse_desktop_file(const char *path, Display *dpy, Visual *visual, size_t icon_wh)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;

    bool parse_section = false;
    struct item app = {0};

    /* How did I get this number? I read the biggest line in my /usr/share/applications/gimp.desktop */
    char line[1024];
    while (fgets(line, 1024, f))
    {
        if (!parse_section)
        {
            if (strcmp(line, "[Desktop Entry]\n") == 0)
                parse_section = true;

            continue;
        }
        else if (strstr(line, "[Desktop"))
        {
            break;
        }

        if (strstr(line, "Name=") == line) // FIXME: locales?
        {
            if (parse_desktop_file_value(line, "Name=", &app.text) < 0)
            {
                TEARDOWN_APP(app);
                fclose(f);
                return -1;
            }
        }
        else if (strstr(line, "Icon=") == line)
        {
            if (parse_desktop_file_value(line, "Icon=", &app.icon_path) < 0)
            {
                TEARDOWN_APP(app);
                fclose(f);
                return -1;
            }
        }
        else if (strstr(line, "Exec=") == line)
        {
            if (parse_desktop_file_value(line, "Exec=", &app.exec_cmd) < 0)
            {
                TEARDOWN_APP(app);
                fclose(f);
                return -1;
            }

            /* Strip out %F %U, am I doing this right? */
            char *c = strchr(app.exec_cmd, '%');
            if (c)
                *c = '\0';
        }
    }
    fclose(f);

    if (!app.exec_cmd || !app.text)
    {
        TEARDOWN_APP(app);
        return -1;
    }

    if (app.icon_path)
    {
        char imgpath[PATH_MAX];
        if (find_icon_path_by_name(app.icon_path, imgpath) < 0)
            goto done;

        int x, y, ch;
        uint8_t *data = stbi_load(imgpath, &x, &y, &ch, 4);
        if (!data)
            goto done;

        uint8_t *resize_data = malloc(icon_wh * icon_wh * ch);
        if (!resize_data)
        {
            stbi_image_free(data);
            goto done;
        }

        stbir_resize_uint8_linear(data, x, y, 0, resize_data, icon_wh, icon_wh, 0, (stbir_pixel_layout)ch);

        /* For some fucking reason the red and blue channels are swapped and my understanding is that it's the visual's fault.
        I tried changing the red and blue mask values for the visual but nothing changed. So this is my solution that
        I don't know if is going to work for all cases. */
        for (size_t i = 0; i < icon_wh * icon_wh * ch; i += ch)
        {
            u8 tmp = resize_data[i];
            resize_data[i] = resize_data[i + 2];
            resize_data[i + 2] = tmp;
        }

        app.ximage = XCreateImage(dpy, visual, ch * 8, ZPixmap, 0, (char *)resize_data, icon_wh, icon_wh, 32, 0);
        app.ximage_w = icon_wh;
        app.ximage_h = icon_wh;

        /** As per the man page :
         * "Note that when the image is created using XCreateImage, XGetImage, or XSubImage, the destroy p
         * rocedure that the XDestroyImage function calls frees both the image structure and the data pointed
         * to by the image structure."
         * So we don't need to free 'resize_data'.
         */
        stbi_image_free(data);
    }

done:
    /* I think I just found my very first compiler bug in "clang version 16.0.6". If i put a label before any
    type of variable declaration it errors out with "error: expected expression" for that declaration.
    If i slap some other type of instrution like ';' (a noop) it works. This will need some further investigation
    */
    ;

    struct item *tmp = realloc(g_apps, (g_app_count + 1) * sizeof(struct item));
    if (!tmp)
    {
        TEARDOWN_APP(app);
        return -1;
    }

    g_apps = tmp;
    ++g_app_count;
    g_apps[g_app_count - 1] = app;
    return 0;
}

static int enumerate_desktop_apps_in_dirpath(const char *dirpath, Display *dpy, Visual *visual, size_t icon_wh)
{
    DIR *d = opendir(dirpath);
    if (!d)
        return -1;

    struct dirent *dirent;
    while ((dirent = readdir(d)) != NULL)
    {
        if (dirent->d_type != DT_REG && dirent->d_type != DT_LNK)
            continue;

        if (!str_endswith(dirent->d_name, ".desktop"))
            continue;

        char fullpath[PATH_MAX];
        snprintf(fullpath, PATH_MAX, "%s/%s", dirpath, dirent->d_name);
        parse_desktop_file(fullpath, dpy, visual, icon_wh);
    }

    closedir(d);
    return 0;
}

int dmenu_apps_parse(Display *dpy, Visual *visual, size_t icon_wh)
{
    enumerate_desktop_apps_in_dirpath(g_syswide_apps_path, dpy, visual, icon_wh);

    char *user = getlogin();
    if (!user)
        return -1;

    if (user)
    {
        char local_apps_path[PATH_MAX];
        snprintf(local_apps_path, PATH_MAX, "/home/%s/.local/share/applications", user);
        enumerate_desktop_apps_in_dirpath(local_apps_path, dpy, visual, icon_wh);
    }

    /* Mark the end of the list by a NULL text, as per whoever wrote the item matching code's (goofy) decision */
    struct item *tmp = realloc(g_apps, (g_app_count + 1) * sizeof(struct item));
    if (!tmp)
        return -1;

    g_apps = tmp;
    ++g_app_count;
    g_apps[g_app_count - 1].text = NULL;
    return 0;
}

void dmenu_apps_cleanup()
{
    if (g_apps)
    {
        for (struct item *it = g_apps; it->text; ++it)
        {
            free(it->text);
            free(it->icon_path);
            free(it->exec_cmd);
        }

        free(g_apps);
        g_apps = NULL;
        g_app_count = 0;
    }
}

struct item *dmenu_get_app_items()
{
    return g_apps;
}
