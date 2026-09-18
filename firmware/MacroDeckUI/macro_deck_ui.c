#include "macro_deck_ui.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <esp_heap_caps.h>

#if LVGL_VERSION_MAJOR != 8
#error "macro_deck_ui currently targets LVGL 8.x (Waveshare demo 09 uses LVGL 8.4.0)"
#endif

#define C_HEX(hex) lv_color_hex(hex)

extern const uint16_t ui_reference_rgb565[];
extern const uint16_t ui_orca_rgb565[];

/* Images are stored as RGB565 so they can be copied straight into the canvas. */
_Static_assert(sizeof(lv_color_t) == 2, "macro_deck_ui expects LV_COLOR_DEPTH 16");

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
static lv_color_t *s_reference_pixels;
static lv_obj_t *s_reference_canvas;
static lv_obj_t *s_fusion_hotspots;
static lv_obj_t *s_orca_hotspots;
static lv_obj_t *s_jog_pad;
static lv_obj_t *s_jog_step_label;
static bool s_jog_fine;
static macro_action_t s_profile = MACRO_ACTION_PROFILE_FUSION;

static lv_style_t s_style_screen;
static lv_style_t s_style_panel;
static lv_style_t s_style_card;
static lv_style_t s_style_card_pressed;
static lv_style_t s_style_selected;
static lv_style_t s_style_text;
static lv_style_t s_style_muted;
static bool s_styles_ready;

static void load_reference_image(const uint16_t *source_pixels);
static void set_active_profile(macro_action_t profile);
static void build_jog_pad(lv_obj_t *parent);

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
    const lv_event_code_t code = lv_event_get_code(event);
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_LONG_PRESSED_REPEAT) return;
    const action_spec_t *spec = (const action_spec_t *)lv_event_get_user_data(event);
    if (spec == NULL) return;

    if (spec->action == MACRO_ACTION_PROFILE_FUSION ||
        spec->action == MACRO_ACTION_PROFILE_ORCA) {
        set_active_profile(spec->action);
    }

    /* The Move button doubles as the jog pad opener while Orca is active. */
    if (spec->action == MACRO_ACTION_MOVE && s_profile == MACRO_ACTION_PROFILE_ORCA &&
        s_jog_pad != NULL) {
        lv_obj_clear_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(s_jog_pad);
    } else if (spec->action == MACRO_ACTION_JOG_CLOSE && s_jog_pad != NULL) {
        lv_obj_add_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);
    }

    macro_action_t action = spec->action;
    if (action == MACRO_ACTION_JOG_STEP_FINE || action == MACRO_ACTION_JOG_STEP_COARSE) {
        s_jog_fine = !s_jog_fine;
        action = s_jog_fine ? MACRO_ACTION_JOG_STEP_FINE : MACRO_ACTION_JOG_STEP_COARSE;
        if (s_jog_step_label != NULL) {
            lv_label_set_text(s_jog_step_label, s_jog_fine ? "1 mm" : "10 mm");
        }
    }

    if (s_action_cb != NULL) {
        s_action_cb(action, s_action_user_data);
    }
}

static lv_obj_t *make_hotspot(lv_obj_t *parent, const action_spec_t *spec,
                              lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, w, h);
    lv_obj_set_style_radius(button, 7, 0);
    lv_obj_set_style_bg_color(button, C_HEX(COLOR_BLUE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_30, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(button, C_HEX(0xC8E5FF), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(button, 2, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button, action_event_cb, LV_EVENT_CLICKED, (void *)spec);
    return button;
}

static void build_reference_ui(lv_obj_t *parent)
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
    static const action_spec_t profile_actions[] = {
        {"F", "Fusion 360", "", MACRO_ACTION_PROFILE_FUSION, COLOR_ORANGE},
        {"O", "Orca Slicer", "", MACRO_ACTION_PROFILE_ORCA, 0x24D4C1},
        {LV_SYMBOL_SETTINGS, "System", "", MACRO_ACTION_PROFILE_SYSTEM, 0xBED4F7},
    };

    if (s_reference_pixels == NULL) {
        s_reference_pixels = (lv_color_t *)heap_caps_malloc(
            800U * 480U * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
        );
        LV_ASSERT_MALLOC(s_reference_pixels);
        if (s_reference_pixels == NULL) return;

    }

    s_reference_canvas = lv_canvas_create(parent);
    lv_canvas_set_buffer(s_reference_canvas, s_reference_pixels, 800, 480, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(s_reference_canvas, 0, 0);
    lv_obj_clear_flag(s_reference_canvas, LV_OBJ_FLAG_CLICKABLE);
    load_reference_image(ui_reference_rgb565);

    s_fusion_hotspots = lv_obj_create(parent);
    lv_obj_remove_style_all(s_fusion_hotspots);
    lv_obj_set_size(s_fusion_hotspots, 800, 480);
    lv_obj_clear_flag(s_fusion_hotspots, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < sizeof(s_nav_actions) / sizeof(s_nav_actions[0]); ++i) {
        make_hotspot(s_fusion_hotspots, &s_nav_actions[i], 4, 84 + (lv_coord_t)i * 42, 123, 41);
    }

    for (size_t i = 0; i < sizeof(s_main_actions) / sizeof(s_main_actions[0]); ++i) {
        const lv_coord_t col = (lv_coord_t)(i % 5);
        const lv_coord_t row = (lv_coord_t)(i / 5);
        make_hotspot(s_fusion_hotspots, &s_main_actions[i], 133 + col * 97, 88 + row * 85, 93, 82);
    }

    make_hotspot(s_fusion_hotspots, &view_actions[0], 626, 174, 48, 58);
    make_hotspot(s_fusion_hotspots, &view_actions[1], 681, 174, 48, 58);
    make_hotspot(s_fusion_hotspots, &view_actions[2], 737, 174, 49, 58);
    make_hotspot(s_fusion_hotspots, &view_actions[3], 638, 242, 56, 58);
    make_hotspot(s_fusion_hotspots, &view_actions[4], 713, 242, 57, 58);

    for (size_t i = 0; i < 3; ++i) {
        make_hotspot(s_fusion_hotspots, &display_actions[i], 628 + (lv_coord_t)i * 55, 334, 50, 69);
    }

    make_hotspot(s_fusion_hotspots, &profile_actions[0], 78, 432, 128, 40);
    make_hotspot(s_fusion_hotspots, &profile_actions[1], 208, 432, 128, 40);
    make_hotspot(s_fusion_hotspots, &profile_actions[2], 337, 432, 118, 40);

    static const action_spec_t orca_nav_actions[] = {
        {"P", "Prepare", "", MACRO_ACTION_ORCA_PREPARE, 0x24D4C1},
        {"M", "Modify", "", MACRO_ACTION_ORCA_MODIFY, 0xB7C9E8},
        {"V", "View", "", MACRO_ACTION_ORCA_VIEW, 0xB7C9E8},
        {"S", "Support", "", MACRO_ACTION_ORCA_SUPPORT, 0xB7C9E8},
        {"F", "Filament", "", MACRO_ACTION_ORCA_FILAMENT, 0xB7C9E8},
        {"P", "Printer", "", MACRO_ACTION_ORCA_PRINTER, 0xB7C9E8},
        {"T", "Tools", "", MACRO_ACTION_ORCA_TOOLS, 0xB7C9E8},
        {LV_SYMBOL_SETTINGS, "Settings", "", MACRO_ACTION_ORCA_SETTINGS, 0xB7C9E8},
    };
    /* Reading order of the 6 x 4 grid drawn in ui_orca_rgb565. */
    static const action_spec_t orca_actions[] = {
        {LV_SYMBOL_FILE, "New", "Ctrl + N", MACRO_ACTION_ORCA_NEW_PROJECT, 0x55BFFF},
        {LV_SYMBOL_DIRECTORY, "Open", "Ctrl + O", MACRO_ACTION_ORCA_OPEN_PROJECT, 0x55BFFF},
        {LV_SYMBOL_SAVE, "Save", "Ctrl + S", MACRO_ACTION_ORCA_SAVE_PROJECT, 0x55BFFF},
        {LV_SYMBOL_DIRECTORY, "Import", "Ctrl + I", MACRO_ACTION_ORCA_IMPORT_MODEL, 0x55BFFF},
        {"A", "Arrange", "A", MACRO_ACTION_ORCA_ARRANGE, 0x55BFFF},
        {"Q", "Orient", "Q", MACRO_ACTION_ORCA_AUTO_ORIENT, 0x55BFFF},
        {"+", "Instance +", "+", MACRO_ACTION_ORCA_INSTANCE_ADD, 0x30E57B},
        {"-", "Instance -", "-", MACRO_ACTION_ORCA_INSTANCE_REMOVE, 0xFF5964},
        {"M", "Move", "M", MACRO_ACTION_MOVE, 0x3A9BFF},
        {"R", "Rotate", "R", MACRO_ACTION_ORCA_ROTATE, 0x3A9BFF},
        {"S", "Scale", "S", MACRO_ACTION_ORCA_SCALE, 0x3A9BFF},
        {"F", "Lay Flat", "F", MACRO_ACTION_ORCA_LAY_FLAT, 0x55BFFF},
        {"C", "Cut", "C", MACRO_ACTION_ORCA_CUT, 0x55BFFF},
        {"L", "Support", "L", MACRO_ACTION_ORCA_SUPPORT_PAINTING, 0x8BE0FF},
        {"P", "Seam", "P", MACRO_ACTION_ORCA_SEAM_PAINTING, 0xA14CFF},
        {"H", "Fuzzy Skin", "H", MACRO_ACTION_ORCA_FUZZY_SKIN, 0xA14CFF},
        {"N", "Color", "N", MACRO_ACTION_ORCA_COLOR_PAINTING, 0x68B7FF},
        {"T", "Add Text", "T", MACRO_ACTION_ORCA_ADD_TEXT, 0x68B7FF},
        {"U", "Measure", "U", MACRO_ACTION_ORCA_MEASURE, 0x68B7FF},
        {LV_SYMBOL_LEFT, "Undo", "Ctrl + Z", MACRO_ACTION_UNDO, 0xBED4F7},
        {LV_SYMBOL_RIGHT, "Redo", "Ctrl + Y", MACRO_ACTION_REDO, 0xBED4F7},
        {LV_SYMBOL_TRASH, "Delete", "Del", MACRO_ACTION_ORCA_DELETE, 0xFF5964},
        {"K", "Clone", "Ctrl + K", MACRO_ACTION_ORCA_CLONE, 0x55BFFF},
    };
    static const action_spec_t orca_bottom_actions[] = {
        {"F", "Fusion 360", "", MACRO_ACTION_PROFILE_FUSION, COLOR_ORANGE},
        {"O", "Orca Slicer", "", MACRO_ACTION_PROFILE_ORCA, 0x24D4C1},
        {LV_SYMBOL_SETTINGS, "System", "", MACRO_ACTION_PROFILE_SYSTEM, 0xBED4F7},
    };
    static const action_spec_t orca_view_actions[] = {
        {"0", "Default", "Ctrl + 0", MACRO_ACTION_ORCA_VIEW_DEFAULT, 0xBED4F7},
        {"1", "Top", "Ctrl + 1", MACRO_ACTION_ORCA_VIEW_TOP, 0xBED4F7},
        {"2", "Bottom", "Ctrl + 2", MACRO_ACTION_ORCA_VIEW_BOTTOM, 0xBED4F7},
        {"3", "Front", "Ctrl + 3", MACRO_ACTION_ORCA_VIEW_FRONT, 0xBED4F7},
        {"4", "Behind", "Ctrl + 4", MACRO_ACTION_ORCA_VIEW_BEHIND, 0xBED4F7},
    };
    static const action_spec_t orca_display_actions[] = {
        {"5", "Left", "Ctrl + 5", MACRO_ACTION_ORCA_VIEW_LEFT, 0x24D4C1},
        {"6", "Right", "Ctrl + 6", MACRO_ACTION_ORCA_VIEW_RIGHT, 0xBED4F7},
        {"P", "Preview", "Tab", MACRO_ACTION_ORCA_VIEW_PREVIEW, 0xBED4F7},
    };

    s_orca_hotspots = lv_obj_create(parent);
    lv_obj_remove_style_all(s_orca_hotspots);
    lv_obj_set_size(s_orca_hotspots, 800, 480);
    lv_obj_clear_flag(s_orca_hotspots, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < 8; ++i) {
        make_hotspot(s_orca_hotspots, &orca_nav_actions[i], 4, 84 + (lv_coord_t)i * 42, 123, 41);
    }
    for (size_t i = 0; i < sizeof(orca_actions) / sizeof(orca_actions[0]); ++i) {
        const lv_coord_t col = (lv_coord_t)(i % 6);
        const lv_coord_t row = (lv_coord_t)(i / 6);
        make_hotspot(s_orca_hotspots, &orca_actions[i],
                     134 + col * 80, 88 + row * 85, 76, 82);
    }
    /* The ACTIVE PRINTER panel was repainted as the Slice Plate button. */
    static const action_spec_t orca_slice = {"S", "Slice Plate", "Ctrl + R", MACRO_ACTION_ORCA_SLICE, 0x30E57B};
    make_hotspot(s_orca_hotspots, &orca_slice, 620, 88, 168, 63);
    /* VIEW and DISPLAY were merged into one 3 x 3 panel: 8 view buttons. */
    for (size_t i = 0; i < 5; ++i) {
        const lv_coord_t col = (lv_coord_t)(i % 3);
        const lv_coord_t row = (lv_coord_t)(i / 3);
        make_hotspot(s_orca_hotspots, &orca_view_actions[i],
                     626 + col * 54, 186 + row * 70, 52, 64);
    }
    for (size_t i = 0; i < 3; ++i) {
        const lv_coord_t slot = (lv_coord_t)(i + 5);
        make_hotspot(s_orca_hotspots, &orca_display_actions[i],
                     626 + (slot % 3) * 54, 186 + (slot / 3) * 70, 52, 64);
    }
    make_hotspot(s_orca_hotspots, &orca_bottom_actions[0], 78, 432, 128, 40);
    make_hotspot(s_orca_hotspots, &orca_bottom_actions[1], 208, 432, 128, 40);
    make_hotspot(s_orca_hotspots, &orca_bottom_actions[2], 337, 432, 118, 40);
    make_hotspot(s_orca_hotspots, &orca_nav_actions[7], 744, 12, 44, 44);
    lv_obj_add_flag(s_orca_hotspots, LV_OBJ_FLAG_HIDDEN);

    build_jog_pad(parent);
}

static lv_obj_t *make_jog_button(lv_obj_t *parent, const action_spec_t *spec,
                                 lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                                 bool repeat)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_remove_style_all(button);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, w, h);
    lv_obj_set_style_radius(button, 10, 0);
    lv_obj_set_style_bg_color(button, C_HEX(COLOR_CARD), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(button, C_HEX(0x24333D), 0);
    lv_obj_set_style_border_width(button, 1, 0);
    lv_obj_set_style_bg_color(button, C_HEX(COLOR_BLUE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(button, LV_OPA_40, LV_STATE_PRESSED);
    lv_obj_add_event_cb(button, action_event_cb, LV_EVENT_CLICKED, (void *)spec);
    if (repeat) {
        lv_obj_add_event_cb(button, action_event_cb, LV_EVENT_LONG_PRESSED_REPEAT, (void *)spec);
    }

    lv_obj_t *icon = make_label(button, spec->icon, &lv_font_montserrat_26, spec->accent);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, spec->label[0] == '\0' ? 0 : -9);
    if (spec->label[0] != '\0') {
        lv_obj_t *label = make_label(button, spec->label, &lv_font_montserrat_12, COLOR_MUTED);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 17);
    }
    return button;
}

static void build_jog_pad(lv_obj_t *parent)
{
    static const action_spec_t jog_up = {LV_SYMBOL_UP, "Y +", "", MACRO_ACTION_JOG_Y_PLUS, 0x8BE0FF};
    static const action_spec_t jog_down = {LV_SYMBOL_DOWN, "Y -", "", MACRO_ACTION_JOG_Y_MINUS, 0x8BE0FF};
    static const action_spec_t jog_left = {LV_SYMBOL_LEFT, "X -", "", MACRO_ACTION_JOG_X_MINUS, 0x8BE0FF};
    static const action_spec_t jog_right = {LV_SYMBOL_RIGHT, "X +", "", MACRO_ACTION_JOG_X_PLUS, 0x8BE0FF};
    static const action_spec_t jog_step = {"", "", "", MACRO_ACTION_JOG_STEP_COARSE, 0x30E57B};
    static const action_spec_t jog_close = {LV_SYMBOL_CLOSE, "", "", MACRO_ACTION_JOG_CLOSE, 0xFF5964};

    s_jog_pad = lv_obj_create(parent);
    lv_obj_remove_style_all(s_jog_pad);
    lv_obj_set_pos(s_jog_pad, 236, 96);
    lv_obj_set_size(s_jog_pad, 328, 300);
    lv_obj_set_style_radius(s_jog_pad, 14, 0);
    lv_obj_set_style_bg_color(s_jog_pad, C_HEX(0x0A141A), 0);
    lv_obj_set_style_bg_opa(s_jog_pad, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(s_jog_pad, C_HEX(0x2B4453), 0);
    lv_obj_set_style_border_width(s_jog_pad, 2, 0);
    lv_obj_clear_flag(s_jog_pad, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = make_label(s_jog_pad, "MOVE SELECTION", &lv_font_montserrat_14, 0xE6F2FF);
    lv_obj_set_pos(title, 20, 16);
    lv_obj_t *hint = make_label(s_jog_pad, "Arrow keys - select a model in Orca first",
                                &lv_font_montserrat_12, COLOR_MUTED);
    lv_obj_set_pos(hint, 20, 38);

    make_jog_button(s_jog_pad, &jog_close, 266, 12, 46, 40, false);
    make_jog_button(s_jog_pad, &jog_up, 116, 66, 96, 72, true);
    make_jog_button(s_jog_pad, &jog_left, 14, 146, 96, 72, true);
    make_jog_button(s_jog_pad, &jog_right, 218, 146, 96, 72, true);
    make_jog_button(s_jog_pad, &jog_down, 116, 226, 96, 72, true);

    lv_obj_t *step = make_jog_button(s_jog_pad, &jog_step, 116, 146, 96, 72, false);
    s_jog_step_label = make_label(step, "10 mm", &lv_font_montserrat_16, 0x30E57B);
    lv_obj_align(s_jog_step_label, LV_ALIGN_CENTER, 0, -9);
    lv_obj_t *step_hint = make_label(step, "step", &lv_font_montserrat_12, COLOR_MUTED);
    lv_obj_align(step_hint, LV_ALIGN_CENTER, 0, 15);

    lv_obj_add_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);
}

static void load_reference_image(const uint16_t *source_pixels)
{
    if (s_reference_pixels == NULL || source_pixels == NULL) return;
    memcpy(s_reference_pixels, source_pixels, 800U * 480U * sizeof(lv_color_t));
    if (s_reference_canvas != NULL) lv_obj_invalidate(s_reference_canvas);
}

static void set_active_profile(macro_action_t profile)
{
    if (s_fusion_hotspots == NULL || s_orca_hotspots == NULL) return;
    s_profile = profile;
    if (s_jog_pad != NULL) lv_obj_add_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);
    if (profile == MACRO_ACTION_PROFILE_ORCA) {
        load_reference_image(ui_orca_rgb565);
        lv_obj_add_flag(s_fusion_hotspots, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_orca_hotspots, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(s_orca_hotspots);
    } else {
        load_reference_image(ui_reference_rgb565);
        lv_obj_add_flag(s_orca_hotspots, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(s_fusion_hotspots, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(s_fusion_hotspots);
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
    for (size_t i = 0; i < sizeof(profile_actions) / sizeof(profile_actions[0]); ++i) {
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

    build_reference_ui(s_screen);

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
