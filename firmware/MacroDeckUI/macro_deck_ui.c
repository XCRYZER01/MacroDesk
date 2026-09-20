/*
 * Deck engine: draws the profile background, places the touch zones from
 * macro_deck_profiles.c, builds sidebar pages on demand and runs the jog pad.
 * Buttons themselves are defined in macro_deck_profiles.c.
 */
#include "macro_deck_ui.h"

#include <stdbool.h>
#include <string.h>
#include <esp_heap_caps.h>

#if LVGL_VERSION_MAJOR != 8
#error "macro_deck_ui targets LVGL 8.x"
#endif

/* Images are stored as RGB565 so they can be copied straight into the canvas. */
_Static_assert(sizeof(lv_color_t) == 2, "macro_deck_ui expects LV_COLOR_DEPTH 16");

#define C_HEX(hex) lv_color_hex(hex)
#define SCREEN_W 800
#define SCREEN_H 480
#define MAX_PROFILES 4

static const uint32_t COLOR_TEXT = 0xF4F7FA;
static const uint32_t COLOR_MUTED = 0x8E9BAB;
static const uint32_t COLOR_ICON = 0xBED4F7;
static const uint32_t COLOR_CARD = 0x0E191E;
static const uint32_t COLOR_BLUE = 0x3A9BFF;

static lv_obj_t *s_screen;
static macro_deck_button_cb_t s_button_cb;
static void *s_user_data;

static lv_color_t *s_canvas_pixels;
static lv_obj_t *s_canvas;
static lv_obj_t *s_layers[MAX_PROFILES];    /* touch zones of each profile */
static lv_obj_t *s_markers[MAX_PROFILES];   /* sidebar highlight of each profile */
static size_t s_profile;

/* Only the open sidebar page exists: pages are built on demand because the
 * LVGL heap (LV_MEM_SIZE) is too small to hold every page at once. */
static lv_obj_t *s_page;

static lv_obj_t *s_jog_pad;
static lv_obj_t *s_jog_step_label;
static lv_obj_t *s_settings_step_label;
static bool s_jog_fine;

/* Jog arrows: the fine variants add Shift, which makes Orca move 1 mm instead of 10 mm. */
static const macro_button_t s_jog_coarse[] = {
    {LV_SYMBOL_UP, "Y +", "Up", 0x8BE0FF, MACRO_ACTION_JOG_MOVE, 0},
    {LV_SYMBOL_DOWN, "Y -", "Down", 0x8BE0FF, MACRO_ACTION_JOG_MOVE, 1},
    {LV_SYMBOL_LEFT, "X -", "Left", 0x8BE0FF, MACRO_ACTION_JOG_MOVE, 2},
    {LV_SYMBOL_RIGHT, "X +", "Right", 0x8BE0FF, MACRO_ACTION_JOG_MOVE, 3},
};
static const macro_button_t s_jog_fine_keys[] = {
    {LV_SYMBOL_UP, "Y + (1 mm)", "Shift+Up", 0, MACRO_ACTION_JOG_MOVE, 0},
    {LV_SYMBOL_DOWN, "Y - (1 mm)", "Shift+Down", 0, MACRO_ACTION_JOG_MOVE, 1},
    {LV_SYMBOL_LEFT, "X - (1 mm)", "Shift+Left", 0, MACRO_ACTION_JOG_MOVE, 2},
    {LV_SYMBOL_RIGHT, "X + (1 mm)", "Shift+Right", 0, MACRO_ACTION_JOG_MOVE, 3},
};

static void set_active_profile(size_t index);
static void show_page(size_t index);

static lv_obj_t *make_label(lv_obj_t *parent, const char *text, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text != NULL ? text : "");
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, C_HEX(color), 0);
    return label;
}

static void update_step_labels(void)
{
    const char *text = s_jog_fine ? "1 mm" : "10 mm";
    if (s_jog_step_label != NULL) lv_label_set_text(s_jog_step_label, text);
    if (s_settings_step_label != NULL) lv_label_set_text(s_settings_step_label, text);
}

static void button_event_cb(lv_event_t *event)
{
    const lv_event_code_t code = lv_event_get_code(event);
    if (code != LV_EVENT_CLICKED && code != LV_EVENT_LONG_PRESSED_REPEAT) return;
    const macro_button_t *button = (const macro_button_t *)lv_event_get_user_data(event);
    if (button == NULL) return;

    switch (button->action) {
        case MACRO_ACTION_PROFILE:
            set_active_profile(button->arg);
            break;
        case MACRO_ACTION_PAGE:
            show_page(button->arg);
            break;
        case MACRO_ACTION_JOG_OPEN:
            if (s_jog_pad != NULL) {
                lv_obj_clear_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(s_jog_pad);
            }
            break;
        case MACRO_ACTION_JOG_CLOSE:
            if (s_jog_pad != NULL) lv_obj_add_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);
            break;
        case MACRO_ACTION_JOG_STEP:
            s_jog_fine = !s_jog_fine;
            update_step_labels();
            break;
        case MACRO_ACTION_JOG_MOVE:
            if (s_jog_fine && button->arg < sizeof(s_jog_fine_keys) / sizeof(s_jog_fine_keys[0])) {
                button = &s_jog_fine_keys[button->arg];
            }
            break;
        default:
            break;
    }

    if (s_button_cb != NULL) s_button_cb(button, s_user_data);
}

/* Touch area for a button; its face is painted onto the canvas by paint_zone(). */
static void make_zone(lv_obj_t *parent, const macro_zone_t *zone)
{
    lv_obj_t *hit = lv_btn_create(parent);
    lv_obj_remove_style_all(hit);
    lv_obj_set_pos(hit, zone->x, zone->y);
    lv_obj_set_size(hit, zone->w, zone->h);
    lv_obj_set_style_radius(hit, 7, 0);
    lv_obj_set_style_bg_color(hit, C_HEX(COLOR_BLUE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(hit, LV_OPA_30, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(hit, C_HEX(0xC8E5FF), LV_STATE_PRESSED);
    lv_obj_set_style_border_width(hit, 2, LV_STATE_PRESSED);
    lv_obj_add_event_cb(hit, button_event_cb, LV_EVENT_CLICKED, (void *)&zone->button);
}

/* ---- Sidebar pages ---------------------------------------------------- */

static const char *shortcut_text(const macro_button_t *button)
{
    if (button->keys == NULL) return "";
    if (strncmp(button->keys, "search:", 7) == 0) return "S search";
    return button->keys;
}

/* Returns the icon label so callers can turn it into a live value. */
static lv_obj_t *make_page_button(lv_obj_t *parent, const macro_button_t *button,
                                  lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
    lv_obj_t *card = lv_btn_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_set_style_bg_color(card, C_HEX(COLOR_CARD), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, C_HEX(0x1D2B34), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_bg_color(card, C_HEX(COLOR_BLUE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(card, LV_OPA_40, LV_STATE_PRESSED);
    lv_obj_set_style_border_color(card, C_HEX(0xC8E5FF), LV_STATE_PRESSED);
    lv_obj_add_event_cb(card, button_event_cb, LV_EVENT_CLICKED, (void *)button);

    lv_obj_t *icon = make_label(card, button->icon, &lv_font_montserrat_26,
                                button->accent != 0 ? button->accent : COLOR_ICON);
    lv_obj_align(icon, LV_ALIGN_TOP_MID, 0, h >= 84 ? 12 : 6);
    lv_obj_t *label = make_label(card, button->label, &lv_font_montserrat_14, COLOR_TEXT);
    lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_t *shortcut = make_label(card, shortcut_text(button), &lv_font_montserrat_12, COLOR_MUTED);
    lv_obj_align(shortcut, LV_ALIGN_BOTTOM_MID, 0, -7);
    return icon;
}

static lv_obj_t *build_page(lv_obj_t *parent, const macro_page_t *def)
{
    /* Covers the image grid (and blocks its zones) while the page is open. */
    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_pos(page, 131, 84);
    lv_obj_set_size(page, 486, 344);
    lv_obj_set_style_bg_color(page, C_HEX(0x010408), 0);
    lv_obj_set_style_bg_opa(page, LV_OPA_COVER, 0);
    lv_obj_clear_flag(page, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = make_label(page, def->title, &lv_font_montserrat_16, 0xE6F2FF);
    lv_obj_set_pos(title, 8, 6);
    lv_obj_t *hint = make_label(page, def->hint, &lv_font_montserrat_12, COLOR_MUTED);
    lv_obj_set_pos(hint, 8, 28);

    if (def->count == 0 || def->cols == 0) return page;
    const lv_coord_t gap = 8;
    const lv_coord_t top = 50;
    const uint8_t rows = (uint8_t)((def->count + def->cols - 1) / def->cols);
    const lv_coord_t w = (lv_coord_t)((474 - (def->cols - 1) * gap) / def->cols);
    lv_coord_t h = (lv_coord_t)((288 - (rows - 1) * gap) / rows);
    if (h > 100) h = 100;

    for (uint8_t i = 0; i < def->count; ++i) {
        const lv_coord_t col = (lv_coord_t)(i % def->cols);
        const lv_coord_t row = (lv_coord_t)(i / def->cols);
        lv_obj_t *icon = make_page_button(page, &def->buttons[i],
                                          6 + col * (w + gap), top + row * (h + gap), w, h);
        if (def->buttons[i].action == MACRO_ACTION_JOG_STEP) {
            s_settings_step_label = icon;
            update_step_labels();
        }
    }
    return page;
}

static void show_page(size_t index)
{
    if (s_page != NULL) {
        lv_obj_del(s_page);
        s_page = NULL;
        s_settings_step_label = NULL;
    }
    const macro_profile_t *profile = &macro_profiles[s_profile];
    if (index > 0 && index < profile->page_count) {
        s_page = build_page(s_layers[s_profile], &profile->pages[index]);
    }
    if (s_markers[s_profile] != NULL) {
        lv_obj_set_pos(s_markers[s_profile], 5, (lv_coord_t)(84 + index * 42));
    }
}

static lv_obj_t *make_marker(lv_obj_t *parent, const macro_profile_t *profile)
{
    lv_obj_t *marker = lv_obj_create(parent);
    lv_obj_remove_style_all(marker);
    lv_obj_set_size(marker, 121, 42);
    lv_obj_set_pos(marker, 5, 84);
    lv_obj_set_style_radius(marker, 9, 0);
    lv_obj_set_style_bg_color(marker, C_HEX(profile->highlight_fill), 0);
    lv_obj_set_style_bg_opa(marker, LV_OPA_50, 0);
    lv_obj_set_style_border_color(marker, C_HEX(profile->highlight_border), 0);
    lv_obj_set_style_border_width(marker, 2, 0);
    lv_obj_clear_flag(marker, LV_OBJ_FLAG_CLICKABLE);
    return marker;
}

/* ---- Jog pad (Orca: arrow keys move the selected model) ----------------- */

static lv_obj_t *make_jog_button(lv_obj_t *parent, const macro_button_t *button,
                                 lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                                 bool repeat)
{
    lv_obj_t *card = lv_btn_create(parent);
    lv_obj_remove_style_all(card);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_radius(card, 10, 0);
    lv_obj_set_style_bg_color(card, C_HEX(0x152129), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(card, C_HEX(0x24333D), 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_bg_color(card, C_HEX(COLOR_BLUE), LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(card, LV_OPA_40, LV_STATE_PRESSED);
    lv_obj_add_event_cb(card, button_event_cb, LV_EVENT_CLICKED, (void *)button);
    if (repeat) {
        lv_obj_add_event_cb(card, button_event_cb, LV_EVENT_LONG_PRESSED_REPEAT, (void *)button);
    }

    const bool has_label = button->label != NULL && button->label[0] != '\0';
    lv_obj_t *icon = make_label(card, button->icon, &lv_font_montserrat_26, button->accent);
    lv_obj_align(icon, LV_ALIGN_CENTER, 0, has_label ? -9 : 0);
    if (has_label) {
        lv_obj_t *label = make_label(card, button->label, &lv_font_montserrat_12, COLOR_MUTED);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 17);
    }
    return card;
}

static void build_jog_pad(lv_obj_t *parent)
{
    /* No visible label: the step button draws its own, the close button is just an X. */
    static const macro_button_t jog_step = {"", "", NULL, 0x30E57B, MACRO_ACTION_JOG_STEP};
    static const macro_button_t jog_close = {LV_SYMBOL_CLOSE, "", NULL, 0xFF5964, MACRO_ACTION_JOG_CLOSE};

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
    make_jog_button(s_jog_pad, &s_jog_coarse[0], 116, 66, 96, 72, true);
    make_jog_button(s_jog_pad, &s_jog_coarse[2], 14, 146, 96, 72, true);
    make_jog_button(s_jog_pad, &s_jog_coarse[3], 218, 146, 96, 72, true);
    make_jog_button(s_jog_pad, &s_jog_coarse[1], 116, 226, 96, 72, true);

    lv_obj_t *step = make_jog_button(s_jog_pad, &jog_step, 116, 146, 96, 72, false);
    s_jog_step_label = make_label(step, "10 mm", &lv_font_montserrat_16, 0x30E57B);
    lv_obj_align(s_jog_step_label, LV_ALIGN_CENTER, 0, -9);
    lv_obj_t *step_hint = make_label(step, "step", &lv_font_montserrat_12, COLOR_MUTED);
    lv_obj_align(step_hint, LV_ALIGN_CENTER, 0, 15);
    update_step_labels();

    lv_obj_add_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);
}

/* ---- Painting the image page ------------------------------------------ */

/* The deck used to rely on its labels being painted into the background PNG,
 * which made a button impossible to change without redrawing the artwork. The
 * artwork now carries only the frame and the card shapes, and the text is
 * painted here from the profile itself.
 *
 * It is drawn into the canvas rather than built from LVGL objects on purpose:
 * a profile has around forty zones, and a card plus an icon and two labels each
 * would need several times the 48 KB LVGL heap. Painting into the image buffer
 * costs no LVGL memory at all and happens once per profile switch. */
static void paint_text(lv_coord_t x, lv_coord_t y, lv_coord_t w,
                       const lv_font_t *font, uint32_t colour, const char *text)
{
    if (text == NULL || text[0] == '\0') return;
    lv_draw_label_dsc_t dsc;
    lv_draw_label_dsc_init(&dsc);
    dsc.color = C_HEX(colour);
    dsc.font = font;
    dsc.align = LV_TEXT_ALIGN_CENTER;
    lv_canvas_draw_text(s_canvas, x, y, w, &dsc, text);
}

static void paint_zone(const macro_zone_t *zone)
{
    const macro_button_t *button = &zone->button;
    if (button->label == NULL || button->label[0] == '\0') return;

    const bool dead = button->action == MACRO_ACTION_NONE;
    const uint32_t colour = dead ? COLOR_MUTED : COLOR_TEXT;
    const lv_font_t *label_font = zone->w >= 110 ? &lv_font_montserrat_14 : &lv_font_montserrat_12;
    const lv_coord_t lh = (lv_coord_t)lv_font_get_line_height(label_font);
    const lv_coord_t sh = (lv_coord_t)lv_font_get_line_height(&lv_font_montserrat_12);

    /* Sidebar rows and the bottom bar are one short line of text. */
    if (zone->h < 50) {
        paint_text(zone->x, zone->y + (zone->h - lh) / 2, zone->w, label_font, colour, button->label);
        return;
    }

    /* Keys: an icon on top where there is room, then the label, then the key it
     * sends. The label is given two lines so a long one wraps instead of
     * running over the shortcut. */
    const lv_coord_t bottom = zone->y + zone->h;
    if (zone->h >= 76 && button->icon != NULL && button->icon[0] != '\0') {
        paint_text(zone->x, zone->y + 8, zone->w, &lv_font_montserrat_16,
                   button->accent != 0 ? button->accent : COLOR_ICON, button->icon);
        paint_text(zone->x, bottom - sh - 2 * lh - 6, zone->w, label_font, colour, button->label);
    } else {
        paint_text(zone->x, zone->y + 4, zone->w, label_font, colour, button->label);
    }
    paint_text(zone->x, bottom - sh - 4, zone->w, &lv_font_montserrat_12, COLOR_MUTED,
               dead ? "" : shortcut_text(button));
}

static void paint_profile(const macro_profile_t *profile)
{
    if (s_canvas == NULL) return;
    for (size_t i = 0; i < profile->zone_count; ++i) paint_zone(&profile->zones[i]);
}

/* ---- Profiles --------------------------------------------------------- */

static void set_active_profile(size_t index)
{
    if (index >= macro_profile_count || index >= MAX_PROFILES) return;

    show_page(0);   /* closes the open page of the current profile */
    if (s_jog_pad != NULL) lv_obj_add_flag(s_jog_pad, LV_OBJ_FLAG_HIDDEN);

    s_profile = index;
    if (s_canvas_pixels != NULL) {
        memcpy(s_canvas_pixels, macro_profiles[index].image, SCREEN_W * SCREEN_H * sizeof(lv_color_t));
        paint_profile(&macro_profiles[index]);
        lv_obj_invalidate(s_canvas);
    }
    for (size_t i = 0; i < macro_profile_count && i < MAX_PROFILES; ++i) {
        if (s_layers[i] == NULL) continue;
        if (i == index) {
            lv_obj_clear_flag(s_layers[i], LV_OBJ_FLAG_HIDDEN);
            lv_obj_move_foreground(s_layers[i]);
        } else {
            lv_obj_add_flag(s_layers[i], LV_OBJ_FLAG_HIDDEN);
        }
    }
    show_page(0);   /* puts the new profile's sidebar highlight on row 0 */
}

void macro_deck_ui_create(macro_deck_button_cb_t button_cb, void *user_data)
{
    s_button_cb = button_cb;
    s_user_data = user_data;

    s_screen = lv_obj_create(NULL);
    lv_obj_remove_style_all(s_screen);
    lv_obj_set_style_bg_color(s_screen, C_HEX(0x071017), 0);
    lv_obj_set_style_bg_opa(s_screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);

    s_canvas_pixels = (lv_color_t *)heap_caps_malloc(SCREEN_W * SCREEN_H * sizeof(lv_color_t),
                                                     MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    LV_ASSERT_MALLOC(s_canvas_pixels);
    if (s_canvas_pixels != NULL) {
        s_canvas = lv_canvas_create(s_screen);
        lv_canvas_set_buffer(s_canvas, s_canvas_pixels, SCREEN_W, SCREEN_H, LV_IMG_CF_TRUE_COLOR);
        lv_obj_clear_flag(s_canvas, LV_OBJ_FLAG_CLICKABLE);
    }

    for (size_t p = 0; p < macro_profile_count && p < MAX_PROFILES; ++p) {
        const macro_profile_t *profile = &macro_profiles[p];
        lv_obj_t *layer = lv_obj_create(s_screen);
        lv_obj_remove_style_all(layer);
        lv_obj_set_size(layer, SCREEN_W, SCREEN_H);
        lv_obj_clear_flag(layer, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
        for (size_t z = 0; z < profile->zone_count; ++z) make_zone(layer, &profile->zones[z]);
        s_markers[p] = make_marker(layer, profile);
        s_layers[p] = layer;
    }

    build_jog_pad(s_screen);
    set_active_profile(0);
    lv_scr_load(s_screen);
}

lv_obj_t *macro_deck_ui_get_screen(void)
{
    return s_screen;
}
