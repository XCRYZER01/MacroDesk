#pragma once

#include <lvgl.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* What a button does on the deck itself. Most buttons are MACRO_ACTION_KEYS:
 * they only send their `keys` to the PC. */
typedef enum {
    MACRO_ACTION_KEYS = 0,   /* send `keys`, nothing else (the default) */
    MACRO_ACTION_NONE,       /* placeholder, does nothing */
    MACRO_ACTION_PROFILE,    /* switch to profile number `arg` */
    MACRO_ACTION_PAGE,       /* open sidebar page `arg` (0 = the image page) */
    MACRO_ACTION_JOG_OPEN,   /* open the jog pad, and send `keys` */
    MACRO_ACTION_JOG_MOVE,   /* jog pad arrow, repeats while held */
    MACRO_ACTION_JOG_STEP,   /* toggle the jog step between 10 mm and 1 mm */
    MACRO_ACTION_JOG_CLOSE,
} macro_action_t;

/* One button. Only the first three fields are required:
 *
 *   {LV_SYMBOL_SAVE, "Save", "Ctrl+S"}
 *
 * keys is written the way you would say it:
 *   "Ctrl+Shift+G", "Del", "F6", "Shift+Tab", "+", "search:Revolve"
 * Modifiers: Ctrl, Shift, Alt, Win. Named keys: Del, Esc, Tab, Enter, Space,
 * Backspace, Home, End, PgUp, PgDn, Up, Down, Left, Right, F1-F12.
 * "search:NAME" presses S, types NAME and presses Enter (Fusion command search).
 *
 * icon and accent are only drawn on sidebar pages; on image pages the picture
 * shows the button. */
typedef struct {
    const char *icon;        /* LV_SYMBOL_* or short text */
    const char *label;
    const char *keys;        /* NULL = send nothing */
    uint32_t accent;         /* icon colour on sidebar pages, 0 = default */
    macro_action_t action;   /* default MACRO_ACTION_KEYS */
    uint8_t arg;             /* profile / page index for PROFILE and PAGE */
} macro_button_t;

/* A touch area placed over a button painted in the background image. */
typedef struct {
    lv_coord_t x, y, w, h;
    macro_button_t button;
} macro_zone_t;

/* A sidebar page drawn by LVGL on top of the image grid. */
typedef struct {
    const char *title;
    const char *hint;
    const macro_button_t *buttons;
    uint8_t count;
    uint8_t cols;
} macro_page_t;

typedef struct {
    const char *name;
    const uint16_t *image;          /* 800x480 RGB565, see tools/png_to_rgb565.py */
    const macro_zone_t *zones;
    size_t zone_count;
    const macro_page_t *pages;      /* pages[0] is the image page and is never drawn */
    size_t page_count;
    uint32_t highlight_fill;        /* sidebar highlight colours */
    uint32_t highlight_border;
} macro_profile_t;

/* Defined in macro_deck_profiles.c */
extern const macro_profile_t macro_profiles[];
extern const size_t macro_profile_count;

typedef void (*macro_deck_button_cb_t)(const macro_button_t *button, void *user_data);

/** Build and load the 800x480 control-deck screen. */
void macro_deck_ui_create(macro_deck_button_cb_t button_cb, void *user_data);

/** Return the root screen, or NULL before macro_deck_ui_create(). */
lv_obj_t *macro_deck_ui_get_screen(void);

#ifdef __cplusplus
}
#endif
