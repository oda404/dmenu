/* See LICENSE file for copyright and license details. */
/* Default settings; can be overriden by command line. */

/* -fn option overrides fonts[0]; default X11 font or font set */
static const char *fonts[] = {
	"monospace:size=10"};
static const char *prompt = NULL; /* -p  option; prompt to the left of input field */
static const char *colors[SchemeLast][2] = {
	/*     fg         bg       */
	[SchemeNorm] = {"#ffbbbbbb", "#e0222222"},
	[SchemeSel] = {"#ffeeeeee", "#ff005577"},
	[SchemeOut] = {"#ff000000", "#e000ffff"},
};
/* -l option; if nonzero, dmenu uses vertical list with given number of lines */
static unsigned int lines = 10;

/*
 * Characters not considered part of a word while deleting words
 * for example: " /?\"&[]"
 */
static const char worddelimiters[] = " ";

/* The menu is placed in the middle of the screen, if you want it to be offset
on the Y axis change this number (pixels) */
#define MENU_Y_OFFSET 40
/* Width of the menu (pixels) */
#define MENU_WIDTH 400
/* The height of the submenu selectors. The things that say "Applications" and "Binaries" (pixels) */
#define MENU_SELECT_HEIGHT 50
/* Inside padding of the menu (pixels) */
#define MENU_PADDING 8
