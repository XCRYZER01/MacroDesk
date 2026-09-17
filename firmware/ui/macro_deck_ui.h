#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MACRO_ACTION_NONE = 0,
    MACRO_ACTION_NEW_DESIGN,
    MACRO_ACTION_OPEN,
    MACRO_ACTION_SAVE,
    MACRO_ACTION_UNDO,
    MACRO_ACTION_REDO,
    MACRO_ACTION_LINE,
    MACRO_ACTION_RECTANGLE,
    MACRO_ACTION_CIRCLE,
    MACRO_ACTION_ARC,
    MACRO_ACTION_DIMENSION,
    MACRO_ACTION_EXTRUDE,
    MACRO_ACTION_REVOLVE,
    MACRO_ACTION_FILLET,
    MACRO_ACTION_CHAMFER,
    MACRO_ACTION_SHELL,
    MACRO_ACTION_MOVE,
    MACRO_ACTION_COMBINE,
    MACRO_ACTION_HOLE,
    MACRO_ACTION_PATTERN,
    MACRO_ACTION_MIRROR,
    MACRO_ACTION_VIEW_HOME,
    MACRO_ACTION_VIEW_FIT,
    MACRO_ACTION_VIEW_ZOOM,
    MACRO_ACTION_VIEW_PAN,
    MACRO_ACTION_VIEW_ORBIT,
    MACRO_ACTION_DISPLAY_SHADED,
    MACRO_ACTION_DISPLAY_WIREFRAME,
    MACRO_ACTION_DISPLAY_HIDDEN,
    MACRO_ACTION_PROFILE_FUSION,
    MACRO_ACTION_PROFILE_ORCA,
    MACRO_ACTION_PROFILE_BAMBU,
    MACRO_ACTION_PROFILE_SYSTEM,
    MACRO_ACTION_NAV_HOME,
    MACRO_ACTION_NAV_SKETCH,
    MACRO_ACTION_NAV_SOLID,
    MACRO_ACTION_NAV_SURFACE,
    MACRO_ACTION_NAV_MESH,
    MACRO_ACTION_NAV_SHEET_METAL,
    MACRO_ACTION_NAV_TOOLS,
    MACRO_ACTION_NAV_SETTINGS,
} macro_action_t;

typedef void (*macro_deck_action_cb_t)(macro_action_t action, void *user_data);

/** Build and load the 800x480 Fusion 360 control-deck screen. */
void macro_deck_ui_create(macro_deck_action_cb_t action_cb, void *user_data);

/** Update the clock shown in the header. Strings are copied by LVGL. */
void macro_deck_ui_set_clock(const char *date, const char *time);

/** Return the root screen, or NULL before macro_deck_ui_create(). */
lv_obj_t *macro_deck_ui_get_screen(void);

#ifdef __cplusplus
}
#endif
