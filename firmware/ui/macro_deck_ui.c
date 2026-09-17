#include "macro_deck_ui.h"

#include <stdbool.h>
#include <stddef.h>

#if LVGL_VERSION_MAJOR != 8
#error "macro_deck_ui currently targets LVGL 8.x (Waveshare demo 09 uses LVGL 8.4.0)"
#endif

#define C_HEX(hex) lv_color_hex(hex)

typedef struct {
    const char *icon;
    const char *label;
    const char *shortcut;
    macro_action_t action;
    uint32_t accent;
} action_spec_t;

static const uint32_t COLOR_BG = 0x071017;
static const uint32_t COLOR_PANEL = 0x0D171E;
static const uint32_t COLOR_CARD = 0x152129;
static const uint32_t COLOR_CARD_2 = 0x101B22;
static const uint32_t COLOR_BORDER = 0x2B3A44;
static const uint32_t COLOR_TEXT = 0xF4F7FA;
static const uint32_t COLOR_MUTED = 0x8E9BAB;
static const uint32_t COLOR_ORANGE = 0xF47721;
static const uint32_t COLOR_BLUE = 0x3A9BFF;

static lv_obj_t *s_screen;
static lv_obj_t *s_date_label;
static lv_obj_t *s_time_label;
static macro_deck_action_cb_t s_action_cb;
static void *s_action_user_data;

static lv_style_t s_style_screen;
static lv_style_t s_style_panel;
static lv_style_t s_style_card;
static lv_style_t s_style_card_pressed;
static lv_style_t s_style_selected;
static lv_style_t s_style_text;
static lv_style_t s_style_muted;
static bool s_styles_ready;

static const action_spec_t s_main_actions[] = {
    {LV_SYMBOL_FILE, "New Design", "Ctrl + N", MACRO_ACTION_NEW_DESIGN, 0xB8D8FF},
    {LV_SYMBOL_DIRECTORY, "Open", "Ctrl + O", MACRO_ACTION_OPEN, 0xFFAA2A},
    {LV_SYMBOL_SAVE, "Save", "Ctrl + S", MACRO_ACTION_SAVE, 0x398DFF},
    {LV_SYMBOL_LEFT, "Undo", "Ctrl + Z", MACRO_ACTION_UNDO, 0x3A9BFF},
    {LV_SYMBOL_RIGHT, "Redo", "Ctrl + Y", MACRO_ACTION_REDO, 0xAEB8C6},
    {"/", "Line", "L", MACRO_ACTION_LINE, 0x3A9BFF},
    {"[]", "Rectangle", "R", MACRO_ACTION_RECTANGLE, 0x3A9BFF},
    {"O", "Circle", "C", MACRO_ACTION_CIRCLE, 0x3A9BFF},
    {")", "Arc", "A", MACRO_ACTION_ARC, 0x8CC8FF},
    {"<->", "Dimension", "D", MACRO_ACTION_DIMENSION, 0xFF9138},
    {LV_SYMBOL_UP, "Extrude", "E", MACRO_ACTION_EXTRUDE, 0x55A9FF},
    {LV_SYMBOL_REFRESH, "Revolve", "R", MACRO_ACTION_REVOLVE, 0x3A9BFF},
    {"R", "Fillet", "F", MACRO_ACTION_FILLET, 0x5AAEFF},
    {"/_", "Chamfer", "Shift + C", MACRO_ACTION_CHAMFER, 0x5AAEFF},
    {"[]", "Shell", "Q", MACRO_ACTION_SHELL, 0x3A9BFF},
    {LV_SYMBOL_PLUS, "Move", "M", MACRO_ACTION_MOVE, 0xFF9138},
    {"++", "Combine", "Shift + J", MACRO_ACTION_COMBINE, 0x3A9BFF},
    {"O", "Hole", "H", MACRO_ACTION_HOLE, 0x82BFFF},
    {"::", "Pattern", "Shift + P", MACRO_ACTION_PATTERN, 0x3A9BFF},
    {"|><|", "Mirror", "M", MACRO_ACTION_MIRROR, 0x3A9BFF},
};

static const action_spec_t s_nav_actions[] = {
    {LV_SYMBOL_HOME, "Home", "", MACRO_ACTION_NAV_HOME, COLOR_ORANGE},
    {LV_SYMBOL_EDIT, "Sketch", "", MACRO_ACTION_NAV_SKETCH, 0xB7C9E8},
    {"[]", "Solid", "", MACRO_ACTION_NAV_SOLID, 0xB7C9E8},
    {"~", "Surface", "", MACRO_ACTION_NAV_SURFACE, 0xB7C9E8},
    {"#", "Mesh", "", MACRO_ACTION_NAV_MESH, 0xB7C9E8},
    {"///", "Sheet Metal", "", MACRO_ACTION_NAV_SHEET_METAL, 0xB7C9E8},
    {LV_SYMBOL_SETTINGS, "Tools", "", MACRO_ACTION_NAV_TOOLS, 0xB7C9E8},
    {LV_SYMBOL_SETTINGS, "Settings", "", MACRO_ACTION_NAV_SETTINGS, 0xB7C9E8},
};

static void init_styles(void)
{
    if (s_styles_ready) return;

    lv_style_init(&s_style_screen);
    lv_style_set_bg_color(&s_style_screen, C_HEX(COLOR_BG));
    lv_style_set_bg_opa(&s_style_screen, LV_OPA_COVER);
    lv_style_set_text_color(&s_style_screen, C_HEX(COLOR_TEXT));
    lv_style_set_text_font(&s_style_screen, &lv_font_montserrat_14);

    lv_style_init(&s_style_panel);
    lv_style_set_bg_color(&s_style_panel, C_HEX(COLOR_PANEL));
    lv_style_set_bg_opa(&s_style_panel, LV_OPA_COVER);
    lv_style_set_border_color(&s_style_panel, C_HEX(0x14242E));
    lv_style_set_border_width(&s_style_panel, 1);
    lv_style_set_radius(&s_style_panel, 8);
    lv_style_set_pad_all(&s_style_panel, 0);

    lv_style_init(&s_style_card);
    lv_style_set_bg_color(&s_style_card, C_HEX(COLOR_CARD));
    lv_style_set_bg_opa(&s_style_card, LV_OPA_COVER);
    lv_style_set_border_color(&s_style_card, C_HEX(COLOR_BORDER));
    lv_style_set_border_width(&s_style_card, 1);
    lv_style_set_radius(&s_style_card, 7);
    lv_style_set_shadow_color(&s_style_card, C_HEX(0x000000));
    lv_style_set_shadow_opa(&s_style_card, LV_OPA_30);
    lv_style_set_shadow_width(&s_style_card, 6);
    lv_style_set_shadow_ofs_y(&s_style_card, 2);

    lv_style_init(&s_style_card_pressed);
    lv_style_set_bg_color(&s_style_card_pressed, C_HEX(0x20313D));
    lv_style_set_border_color(&s_style_card_pressed, C_HEX(COLOR_BLUE));
    lv_style_set_transform_zoom(&s_style_card_pressed, 250);

    lv_style_init(&s_style_selected);
    lv_style_set_bg_color(&s_style_selected, C_HEX(0x442211));
    lv_style_set_border_color(&s_style_selected, C_HEX(COLOR_ORANGE));
    lv_style_set_border_width(&s_style_selected, 2);

    lv_style_init(&s_style_text);
    lv_style_set_text_color(&s_style_text, C_HEX(COLOR_TEXT));

    lv_style_init(&s_style_muted);
    lv_style_set_text_color(&s_style_muted, C_HEX(COLOR_MUTED));

    s_styles_ready = true;
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, C_HEX(color), 0);
    return label;
}

static void action_event_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    const action_spec_t *spec = (const action_spec_t *)lv_event_get_user_data(event);
    if (spec != NULL && s_action_cb != NULL) {
        s_action_cb(spec->action, s_action_user_data);
    }
}

static lv_obj_t *make_action_card(lv_obj_t *parent, const action_spec_t *spec,
                                  lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_add_style(button, &s_style_card, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(button, &s_style_card_pressed, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, w, h);
    lv_obj_add_event_cb(button, action_event_cb, LV_EVENT_CLICKED, (void *)spec);

    lv_obj_t *icon = make_label(button, spec->icon, &lv_font_montserrat_26, spec->accent);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 7);

    lv_obj_t *label = make_label(button, spec->label, &lv_font_montserrat_14, COLOR_TEXT);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_obj_t *shortcut = make_label(button, spec->shortcut, &lv_font_montserrat_12, COLOR_MUTED);
    lv_obj_align(shortcut, LV_ALIGN_BOTTOM_MID, 0, -5);
    return button;
}

static void build_header(lv_obj_t *parent)
{
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_remove_style_all(header);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_size(header, 800, 82);
    lv_obj_set_style_bg_color(header, C_HEX(0x09131A), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(header, C_HEX(0x1A2933), 0);
    lv_obj_set_style_border_width(header, 1, 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);

    lv_obj_t *logo = lv_obj_create(header);
    lv_obj_remove_style_all(logo);
    lv_obj_set_pos(logo, 20, 14);
    lv_obj_set_size(logo, 60, 56);
    lv_obj_set_style_bg_color(logo, C_HEX(COLOR_ORANGE), 0);
    lv_obj_set_style_bg_opa(logo, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(logo, 7, 0);
    lv_obj_t *logo_text = make_label(logo, "F", &lv_font_montserrat_26, COLOR_TEXT);
    lv_obj_center(logo_text);

    lv_obj_t *brand = make_label(header, "AUTODESK", &lv_font_montserrat_14, COLOR_TEXT);
    lv_obj_set_pos(brand, 98, 11);
    lv_obj_t *title = make_label(header, "Fusion 360", &lv_font_montserrat_26, COLOR_TEXT);
    lv_obj_set_pos(title, 96, 29);
    lv_obj_t *tagline = make_label(header, "DESIGN  -  MAKE  -  BETTER TOGETHER", &lv_font_montserrat_12, 0x9CABC0);
    lv_obj_set_pos(tagline, 98, 65);

    lv_obj_t *accent = lv_obj_create(header);
    lv_obj_remove_style_all(accent);
    lv_obj_set_pos(accent, 480, 15);
    lv_obj_set_size(accent, 150, 52);
    lv_obj_set_style_bg_color(accent, C_HEX(0x111E27), 0);
    lv_obj_set_style_bg_opa(accent, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(accent, 9, 0);
    lv_obj_t *idea = make_label(accent, "Ideas into Reality", &lv_font_montserrat_16, 0xC6D3E4);
    lv_obj_center(idea);

    lv_obj_t *divider = lv_obj_create(header);
    lv_obj_remove_style_all(divider);
    lv_obj_set_pos(divider, 653, 10);
    lv_obj_set_size(divider, 1, 61);
    lv_obj_set_style_bg_color(divider, C_HEX(0x293842), 0);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, 0);

    s_date_label = make_label(header, "2026-09-17", &lv_font_montserrat_12, 0x9CABC0);
    lv_obj_set_pos(s_date_label, 670, 10);
    s_time_label = make_label(header, "10:24", &lv_font_montserrat_26, COLOR_TEXT);
    lv_obj_set_pos(s_time_label, 668, 28);
    lv_obj_t *wifi = make_label(header, LV_SYMBOL_WIFI, &lv_font_montserrat_26, COLOR_TEXT);
    lv_obj_set_pos(wifi, 753, 22);
    lv_obj_t *status = make_label(header, "Good Design\nBetter Prints", &lv_font_montserrat_12, 0xA9B6C9);
    lv_obj_set_pos(status, 670, 60);
}

static void build_sidebar(lv_obj_t *parent)
{
    lv_obj_t *sidebar = lv_obj_create(parent);
    lv_obj_remove_style_all(sidebar);
    lv_obj_add_style(sidebar, &s_style_panel, 0);
    lv_obj_set_pos(sidebar, 4, 86);
    lv_obj_set_size(sidebar, 124, 340);

    for (size_t i = 0; i < sizeof(s_nav_actions) / sizeof(s_nav_actions[0]); ++i) {
        const action_spec_t *spec = &s_nav_actions[i];
        lv_obj_t *button = lv_btn_create(sidebar);
        lv_obj_remove_style_all(button);
        lv_obj_set_pos(button, 4, 4 + (lv_coord_t)i * 41);
        lv_obj_set_size(button, 116, 38);
        lv_obj_set_style_radius(button, 7, 0);
        lv_obj_set_style_bg_opa(button, i == 0 ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
        lv_obj_set_style_bg_color(button, C_HEX(i == 0 ? 0xD96018 : COLOR_PANEL), 0);
        lv_obj_add_style(button, &s_style_card_pressed, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_add_event_cb(button, action_event_cb, LV_EVENT_CLICKED, (void *)spec);

        lv_obj_t *icon = make_label(button, spec->icon, &lv_font_montserrat_16,
                                    i == 0 ? COLOR_TEXT : spec->accent);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 9, 0);
        lv_obj_t *label = make_label(button, spec->label, &lv_font_montserrat_14,
                                     i == 0 ? COLOR_TEXT : 0xC3D2E8);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 38, 0);
    }
}

static void build_action_grid(lv_obj_t *parent)
{
    lv_obj_t *grid = lv_obj_create(parent);
    lv_obj_remove_style_all(grid);
    lv_obj_set_pos(grid, 132, 86);
    lv_obj_set_size(grid, 484, 340);

    const lv_coord_t gap = 5;
    const lv_coord_t card_w = 92;
    const lv_coord_t card_h = 81;
    for (size_t i = 0; i < sizeof(s_main_actions) / sizeof(s_main_actions[0]); ++i) {
        lv_coord_t col = (lv_coord_t)(i % 5);
        lv_coord_t row = (lv_coord_t)(i / 5);
        make_action_card(grid, &s_main_actions[i], col * (card_w + gap), row * (card_h + gap), card_w, card_h);
    }
}

static lv_obj_t *make_small_action(lv_obj_t *parent, const action_spec_t *spec,
                                   lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                                   bool selected)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_add_style(button, &s_style_card, LV_PART_MAIN | LV_STATE_DEFAULT);
    if (selected) lv_obj_add_style(button, &s_style_selected, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_style(button, &s_style_card_pressed, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, w, h);
    lv_obj_add_event_cb(button, action_event_cb, LV_EVENT_CLICKED, (void *)spec);
    lv_obj_t *icon = make_label(button, spec->icon, &lv_font_montserrat_16, spec->accent);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, 7);
    lv_obj_t *label = make_label(button, spec->label, &lv_font_montserrat_12, COLOR_TEXT);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -7);
    return button;
}

static void build_right_panel(lv_obj_t *parent)
{
    static const action_spec_t view_actions[] = {
        {LV_SYMBOL_HOME, "Home", "", MACRO_ACTION_VIEW_HOME, 0xBED4F7},
        {LV_SYMBOL_IMAGE, "Fit", "", MACRO_ACTION_VIEW_FIT, 0xBED4F7},
        {LV_SYMBOL_PLUS, "Zoom", "", MACRO_ACTION_VIEW_ZOOM, 0xBED4F7},
        {LV_SYMBOL_UP, "Pan", "", MACRO_ACTION_VIEW_PAN, 0xBED4F7},
        {LV_SYMBOL_REFRESH, "Orbit", "", MACRO_ACTION_VIEW_ORBIT, 0xBED4F7},
    };
    static const action_spec_t display_actions[] = {
        {LV_SYMBOL_IMAGE, "Shaded", "", MACRO_ACTION_DISPLAY_SHADED, 0x69B3FF},
        {"[]", "Wireframe", "", MACRO_ACTION_DISPLAY_WIREFRAME, 0xAAB7C8},
        {LV_SYMBOL_EYE_CLOSE, "Hidden", "", MACRO_ACTION_DISPLAY_HIDDEN, 0xAAB7C8},
    };

    lv_obj_t *right = lv_obj_create(parent);
    lv_obj_remove_style_all(right);
    lv_obj_set_pos(right, 620, 86);
    lv_obj_set_size(right, 176, 340);

    lv_obj_t *workspace = lv_obj_create(right);
    lv_obj_remove_style_all(workspace);
    lv_obj_add_style(workspace, &s_style_panel, 0);
    lv_obj_set_pos(workspace, 0, 0);
    lv_obj_set_size(workspace, 176, 68);
    lv_obj_t *ws_title = make_label(workspace, "ACTIVE WORKSPACE", &lv_font_montserrat_12, 0x9CABC0);
    lv_obj_set_pos(ws_title, 11, 8);
    lv_obj_t *ws = make_label(workspace, "[]  Design     " LV_SYMBOL_DOWN, &lv_font_montserrat_16, COLOR_TEXT);
    lv_obj_set_pos(ws, 11, 33);
    lv_obj_set_style_text_color(ws, C_HEX(COLOR_ORANGE), 0);

    lv_obj_t *view = lv_obj_create(right);
    lv_obj_remove_style_all(view);
    lv_obj_add_style(view, &s_style_panel, 0);
    lv_obj_set_pos(view, 0, 73);
    lv_obj_set_size(view, 176, 133);
    lv_obj_t *view_title = make_label(view, "VIEW", &lv_font_montserrat_12, 0x9CABC0);
    lv_obj_set_pos(view_title, 11, 8);
    for (size_t i = 0; i < 3; ++i) {
        make_small_action(view, &view_actions[i], 6 + (lv_coord_t)i * 56, 27, 52, 49, false);
    }
    make_small_action(view, &view_actions[3], 22, 80, 58, 47, false);
    make_small_action(view, &view_actions[4], 96, 80, 58, 47, false);

    lv_obj_t *display = lv_obj_create(right);
    lv_obj_remove_style_all(display);
    lv_obj_add_style(display, &s_style_panel, 0);
    lv_obj_set_pos(display, 0, 211);
    lv_obj_set_size(display, 176, 129);
    lv_obj_t *display_title = make_label(display, "DISPLAY", &lv_font_montserrat_12, 0x9CABC0);
    lv_obj_set_pos(display_title, 11, 8);
    for (size_t i = 0; i < 3; ++i) {
        make_small_action(display, &display_actions[i], 5 + (lv_coord_t)i * 56, 29, 53, 91, i == 0);
    }
}

static void build_bottom_bar(lv_obj_t *parent)
{
    static const action_spec_t profile_actions[] = {
        {"F", "Fusion 360", "", MACRO_ACTION_PROFILE_FUSION, COLOR_ORANGE},
        {"O", "Orca Slicer", "", MACRO_ACTION_PROFILE_ORCA, 0x24D4C1},
        {"B", "Bambu Studio", "", MACRO_ACTION_PROFILE_BAMBU, 0x45C866},
        {LV_SYMBOL_SETTINGS, "System", "", MACRO_ACTION_PROFILE_SYSTEM, 0xBED4F7},
    };

    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_remove_style_all(bar);
    lv_obj_set_pos(bar, 4, 432);
    lv_obj_set_size(bar, 792, 44);
    lv_obj_set_style_bg_color(bar, C_HEX(0x0C151C), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(bar, C_HEX(0x24333D), 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_radius(bar, 8, 0);

    lv_obj_t *apps = make_label(bar, "Apps", &lv_font_montserrat_14, 0xB7C9E8);
    lv_obj_set_pos(apps, 22, 14);
    for (size_t i = 0; i < 4; ++i) {
        const action_spec_t *spec = &profile_actions[i];
        lv_obj_t *button = lv_btn_create(bar);
        lv_obj_remove_style_all(button);
        lv_obj_set_pos(button, 72 + (lv_coord_t)i * 130, 4);
        lv_obj_set_size(button, 126, 36);
        lv_obj_set_style_radius(button, 7, 0);
        lv_obj_set_style_bg_color(button, C_HEX(i == 0 ? 0x23170F : 0x0C151C), 0);
        lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(button, C_HEX(i == 0 ? COLOR_ORANGE : 0x1D2B34), 0);
        lv_obj_set_style_border_width(button, 1, 0);
        lv_obj_add_style(button, &s_style_card_pressed, LV_PART_MAIN | LV_STATE_PRESSED);
        lv_obj_add_event_cb(button, action_event_cb, LV_EVENT_CLICKED, (void *)spec);
        lv_obj_t *icon = make_label(button, spec->icon, &lv_font_montserrat_16, spec->accent);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);
        lv_obj_t *label = make_label(button, spec->label, &lv_font_montserrat_12, COLOR_TEXT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 36, 0);
    }

    lv_obj_t *motto = make_label(bar, "3D IDEAS\nBIGGER TOMORROW", &lv_font_montserrat_12, 0x8E9BAB);
    lv_obj_set_pos(motto, 643, 10);
    lv_obj_t *cube = make_label(bar, "[]", &lv_font_montserrat_26, 0xAFC5E4);
    lv_obj_set_pos(cube, 754, 8);
}

void macro_deck_ui_create(macro_deck_action_cb_t action_cb, void *user_data)
{
    init_styles();
    s_action_cb = action_cb;
    s_action_user_data = user_data;

    s_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_screen);
    lv_obj_add_style(s_screen, &s_style_screen, 0);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

    build_header(s_screen);
    build_sidebar(s_screen);
    build_action_grid(s_screen);
    build_right_panel(s_screen);
    build_bottom_bar(s_screen);

    lv_scr_load(s_screen);
}

void macro_deck_ui_set_clock(const char *date, const char *time)
{
    if (s_date_label != NULL && date != NULL) lv_label_set_text(s_date_label, date);
    if (s_time_label != NULL && time != NULL) lv_label_set_text(s_time_label, time);
}

lv_obj_t *macro_deck_ui_get_screen(void)
{
    return s_screen;
}
