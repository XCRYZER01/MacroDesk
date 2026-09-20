/*
 * Every button on the deck is defined in this file.
 *
 * A button is one line: {icon, label, keys}. See macro_button_t in
 * macro_deck_ui.h for the keys syntax ("Ctrl+Shift+G", "F6", "search:Revolve").
 *
 * Buttons on an image page are "zones": a rectangle that must sit exactly over
 * the button painted in the background PNG. The *_CELL / *_ROW macros below
 * give the rectangles of the current artwork; templates/layout_guide_orca.png and
 * templates/layout_guide_fusion.png show every zone drawn over the artwork.
 *
 * Buttons on sidebar pages are drawn by LVGL, so they need no artwork and no
 * coordinates: add a line to the page's list and it appears.
 */
#include "macro_deck_ui.h"

extern const uint16_t ui_fusion_clean_rgb565[];
extern const uint16_t ui_orca_clean_rgb565[];

/* ---- Rectangles shared by both profiles (x, y, w, h) ---- */
#define SIDEBAR_ROW(i)   4, (lv_coord_t)(84 + (i) * 42), 123, 41
#define BOTTOM_FUSION    78, 432, 128, 40
#define BOTTOM_ORCA      208, 432, 128, 40
#define BOTTOM_SYSTEM    337, 432, 118, 40
#define RIGHT_TOP_BOX    620, 88, 168, 63

/* Profile numbers used by the bottom bar (index into macro_profiles[]). */
#define PROFILE_FUSION 0
#define PROFILE_ORCA   1

#define PAGE(i)          NULL, 0, MACRO_ACTION_PAGE, (i)
#define BOTTOM_BAR \
    {BOTTOM_FUSION, {"F", "Fusion 360", NULL, 0, MACRO_ACTION_PROFILE, PROFILE_FUSION}}, \
    {BOTTOM_ORCA,   {"O", "Orca Slicer", NULL, 0, MACRO_ACTION_PROFILE, PROFILE_ORCA}}, \
    {BOTTOM_SYSTEM, {LV_SYMBOL_SETTINGS, "System", NULL, 0, MACRO_ACTION_NONE}}

/* ======================================================================== */
/*  Fusion 360                                                              */
/* ======================================================================== */

#define FUSION_CELL(c, r) (lv_coord_t)(133 + (c) * 97), (lv_coord_t)(88 + (r) * 85), 93, 82
#define FUSION_VIEW(c, r) (lv_coord_t)(626 + (c) * 54), (lv_coord_t)(128 + (r) * 90), 52, 84

/* Commands with no default Fusion shortcut use "search:" (English UI only). */
static const macro_zone_t fusion_zones[] = {
    {SIDEBAR_ROW(0), {LV_SYMBOL_HOME, "Home", PAGE(0)}},
    {SIDEBAR_ROW(1), {LV_SYMBOL_EDIT, "Sketch", PAGE(1)}},
    {SIDEBAR_ROW(2), {"[]", "Solid", PAGE(2)}},
    {SIDEBAR_ROW(3), {"~", "Surface", PAGE(3)}},
    {SIDEBAR_ROW(4), {"#", "Mesh", PAGE(4)}},
    {SIDEBAR_ROW(5), {"///", "Sheet Metal", PAGE(5)}},
    {SIDEBAR_ROW(6), {LV_SYMBOL_SETTINGS, "Tools", PAGE(6)}},
    {SIDEBAR_ROW(7), {LV_SYMBOL_SETTINGS, "Settings", PAGE(7)}},

    {FUSION_CELL(0, 0), {LV_SYMBOL_FILE, "New Design", "Ctrl+N"}},
    {FUSION_CELL(1, 0), {LV_SYMBOL_DIRECTORY, "Open", "Ctrl+O"}},
    {FUSION_CELL(2, 0), {LV_SYMBOL_SAVE, "Save", "Ctrl+S"}},
    {FUSION_CELL(3, 0), {LV_SYMBOL_LEFT, "Undo", "Ctrl+Z"}},
    {FUSION_CELL(4, 0), {LV_SYMBOL_RIGHT, "Redo", "Ctrl+Y"}},
    {FUSION_CELL(0, 1), {"/", "Line", "L"}},
    {FUSION_CELL(1, 1), {"[]", "Rectangle", "R"}},
    {FUSION_CELL(2, 1), {"O", "Circle", "C"}},
    {FUSION_CELL(3, 1), {")", "Arc", "search:3-Point Arc"}},
    {FUSION_CELL(4, 1), {"<->", "Dimension", "D"}},
    {FUSION_CELL(0, 2), {LV_SYMBOL_UP, "Extrude", "E"}},
    {FUSION_CELL(1, 2), {LV_SYMBOL_REFRESH, "Revolve", "search:Revolve"}},
    {FUSION_CELL(2, 2), {"R", "Fillet", "F"}},
    {FUSION_CELL(3, 2), {"/_", "Chamfer", "search:Chamfer"}},
    {FUSION_CELL(4, 2), {"[]", "Shell", "search:Shell"}},
    {FUSION_CELL(0, 3), {LV_SYMBOL_PLUS, "Move", "M"}},
    {FUSION_CELL(1, 3), {"++", "Combine", "search:Combine"}},
    {FUSION_CELL(2, 3), {"O", "Hole", "H"}},
    {FUSION_CELL(3, 3), {"::", "Pattern", "search:Rectangular Pattern"}},
    {FUSION_CELL(4, 3), {"|", "Mirror", "search:Mirror"}},

    {FUSION_VIEW(0, 0), {LV_SYMBOL_HOME, "Home", NULL, 0, MACRO_ACTION_NONE}},  /* no default key yet */
    {FUSION_VIEW(1, 0), {LV_SYMBOL_IMAGE, "Fit", "F6"}},
    {FUSION_VIEW(2, 0), {LV_SYMBOL_EYE_OPEN, "Visibility", "V"}},
    {FUSION_VIEW(0, 1), {LV_SYMBOL_NEW_LINE, "Full Screen", "Ctrl+Shift+F"}},
    {FUSION_VIEW(1, 1), {LV_SYMBOL_LIST, "4 Views", "Shift+1"}},
    {FUSION_VIEW(2, 1), {LV_SYMBOL_IMAGE, "Shaded", "Ctrl+4"}},
    {FUSION_VIEW(0, 2), {"[]", "Wireframe", "Ctrl+7"}},
    {FUSION_VIEW(1, 2), {LV_SYMBOL_EYE_CLOSE, "Hidden Edges", "Ctrl+5"}},
    /* FUSION_VIEW(2, 2) is free */

    BOTTOM_BAR,
};

static const macro_button_t fusion_sketch[] = {
    {"L", "Line", "L", 0x3A9BFF},
    {"R", "Rectangle", "R", 0x3A9BFF},
    {"C", "Circle", "C", 0x3A9BFF},
    {"A", "Arc", "search:3-Point Arc", 0x8CC8FF},
    {"D", "Dimension", "D", 0xFF9138},
    {"T", "Trim", "T", 0x3A9BFF},
    {"O", "Offset", "O", 0x3A9BFF},
    {"P", "Project", "P", 0x3A9BFF},
    {"X", "Construction", "X", 0xFFAA2A},
    {"F", "Sketch Fillet", "search:Fillet", 0x8CC8FF},
};
static const macro_button_t fusion_solid[] = {
    {"E", "Extrude", "E", 0x55A9FF},
    {"Q", "Press Pull", "Q", 0x55A9FF},
    {"F", "Fillet", "F", 0x5AAEFF},
    {"H", "Hole", "H", 0x82BFFF},
    {"M", "Move", "M", 0xFF9138},
    {"J", "Joint", "J", 0xFF9138},
    {LV_SYMBOL_REFRESH, "Revolve", "search:Revolve", 0x8CC8FF},
    {"/", "Chamfer", "search:Chamfer", 0x8CC8FF},
    {"[]", "Shell", "search:Shell", 0x8CC8FF},
    {"+", "Combine", "search:Combine", 0x8CC8FF},
    {"::", "Pattern", "search:Rectangular Pattern", 0x8CC8FF},
    {"|", "Mirror", "search:Mirror", 0x8CC8FF},
};
static const macro_button_t fusion_surface[] = {
    {"~", "Patch", "search:Patch", 0x8CC8FF},
    {"+", "Stitch", "search:Stitch", 0x8CC8FF},
    {"-", "Unstitch", "search:Unstitch", 0x8CC8FF},
    {"=", "Thicken", "search:Thicken", 0x8CC8FF},
    {LV_SYMBOL_CUT, "Trim", "search:Trim", 0x8CC8FF},
};
static const macro_button_t fusion_mesh[] = {
    {LV_SYMBOL_DOWNLOAD, "Insert Mesh", "search:Insert Mesh", 0x8CC8FF},
    {LV_SYMBOL_SHUFFLE, "Convert Mesh", "search:Convert Mesh", 0x8CC8FF},
    {LV_SYMBOL_MINUS, "Reduce", "search:Reduce", 0x8CC8FF},
    {LV_SYMBOL_REFRESH, "Remesh", "search:Remesh", 0x8CC8FF},
};
static const macro_button_t fusion_sheet_metal[] = {
    {"L", "Flange", "search:Flange", 0x8CC8FF},
    {LV_SYMBOL_RIGHT, "Unfold", "search:Unfold", 0x8CC8FF},
    {LV_SYMBOL_LEFT, "Refold", "search:Refold Faces", 0x8CC8FF},
    {"[]", "Flat Pattern", "search:Create Flat Pattern", 0x8CC8FF},
    {LV_SYMBOL_LIST, "Rules", "search:Sheet Metal Rules", 0x8CC8FF},
};
static const macro_button_t fusion_tools[] = {
    {"I", "Measure", "I", 0x68B7FF},
    {"A", "Appearance", "A", 0xFF9138},
    {LV_SYMBOL_EYE_OPEN, "Visibility", "V"},
    {LV_SYMBOL_LOOP, "Repeat Last", "Space", 0x30E57B},
    {"|", "Section", "search:Section Analysis", 0x8CC8FF},
    {LV_SYMBOL_WARNING, "Interference", "search:Interference", 0x8CC8FF},
};
static const macro_button_t fusion_settings[] = {
    {LV_SYMBOL_LIST, "Browser", "Ctrl+Alt+B"},
    {LV_SYMBOL_DIRECTORY, "Data Panel", "Ctrl+Alt+P"},
    {LV_SYMBOL_IMAGE, "ViewCube", "Ctrl+Alt+V"},
    {LV_SYMBOL_NEW_LINE, "Full Screen", "Ctrl+Shift+F"},
    {LV_SYMBOL_REFRESH, "Reset Layout", "Ctrl+Alt+R", 0xFFAA2A},
    {"4", "4 Views", "Shift+1"},
    {"W", "Next Workspace", "Ctrl+]", 0xFF9138},
};

#define ITEMS(a) a, (uint8_t)(sizeof(a) / sizeof((a)[0]))

static const macro_page_t fusion_pages[] = {
    {NULL, NULL, NULL, 0, 0},
    {"SKETCH", "", NULL, 0, 4},
    {"SOLID", "", NULL, 0, 4},
    {"SURFACE", "", NULL, 0, 4},
    {"MESH", "", NULL, 0, 4},
    {"SHEET METAL", "", NULL, 0, 4},
    {"TOOLS", "", NULL, 0, 4},
    {"SETTINGS", "", NULL, 0, 4},
};

/* ======================================================================== */
/*  OrcaSlicer                                                              */
/* ======================================================================== */

#define ORCA_CELL(c, r) (lv_coord_t)(134 + (c) * 80), (lv_coord_t)(88 + (r) * 85), 76, 82
#define ORCA_VIEW(c, r) (lv_coord_t)(626 + (c) * 54), (lv_coord_t)(186 + (r) * 70), 52, 64

static const macro_zone_t orca_zones[] = {
    {SIDEBAR_ROW(0), {"P", "Prepare", PAGE(0)}},
    {SIDEBAR_ROW(1), {"M", "Modify", PAGE(1)}},
    {SIDEBAR_ROW(2), {"V", "View", PAGE(2)}},
    {SIDEBAR_ROW(3), {"S", "Support", PAGE(3)}},
    {SIDEBAR_ROW(4), {"F", "Filament", PAGE(4)}},
    {SIDEBAR_ROW(5), {"P", "Printer", PAGE(5)}},
    {SIDEBAR_ROW(6), {"T", "Tools", PAGE(6)}},
    {SIDEBAR_ROW(7), {LV_SYMBOL_SETTINGS, "Settings", PAGE(7)}},

    {ORCA_CELL(0, 0), {LV_SYMBOL_FILE, "New", "Ctrl+N"}},
    {ORCA_CELL(1, 0), {LV_SYMBOL_DIRECTORY, "Open", "Ctrl+O"}},
    {ORCA_CELL(2, 0), {LV_SYMBOL_SAVE, "Save", "Ctrl+S"}},
    {ORCA_CELL(3, 0), {LV_SYMBOL_DIRECTORY, "Import", "Ctrl+I"}},
    {ORCA_CELL(4, 0), {"A", "Arrange", "A"}},
    {ORCA_CELL(5, 0), {"Q", "Orient", "Q"}},
    {ORCA_CELL(0, 1), {"+", "Instance +", "+"}},
    {ORCA_CELL(1, 1), {"-", "Instance -", "-"}},
    {ORCA_CELL(2, 1), {"M", "Move", "M", 0, MACRO_ACTION_JOG_OPEN}},
    {ORCA_CELL(3, 1), {"R", "Rotate", "R"}},
    {ORCA_CELL(4, 1), {"S", "Scale", "S"}},
    {ORCA_CELL(5, 1), {"F", "Lay Flat", "F"}},
    {ORCA_CELL(0, 2), {"C", "Cut", "C"}},
    {ORCA_CELL(1, 2), {"L", "Support Paint", "L"}},
    {ORCA_CELL(2, 2), {"P", "Seam Paint", "P"}},
    {ORCA_CELL(3, 2), {"H", "Fuzzy Skin", "H"}},
    {ORCA_CELL(4, 2), {"N", "Color Paint", "N"}},
    {ORCA_CELL(5, 2), {"T", "Add Text", "T"}},
    {ORCA_CELL(0, 3), {"U", "Measure", "U"}},
    {ORCA_CELL(1, 3), {LV_SYMBOL_LEFT, "Undo", "Ctrl+Z"}},
    {ORCA_CELL(2, 3), {LV_SYMBOL_RIGHT, "Redo", "Ctrl+Y"}},
    {ORCA_CELL(3, 3), {LV_SYMBOL_TRASH, "Delete", "Del"}},
    {ORCA_CELL(4, 3), {"K", "Clone", "Ctrl+K"}},
    /* ORCA_CELL(5, 3) is free */

    {RIGHT_TOP_BOX, {"S", "Slice Plate", "Ctrl+R"}},
    {ORCA_VIEW(0, 0), {"0", "View Default", "Ctrl+0"}},
    {ORCA_VIEW(1, 0), {"1", "View Top", "Ctrl+1"}},
    {ORCA_VIEW(2, 0), {"2", "View Bottom", "Ctrl+2"}},
    {ORCA_VIEW(0, 1), {"3", "View Front", "Ctrl+3"}},
    {ORCA_VIEW(1, 1), {"4", "View Behind", "Ctrl+4"}},
    {ORCA_VIEW(2, 1), {"5", "View Left", "Ctrl+5"}},
    {ORCA_VIEW(0, 2), {"6", "View Right", "Ctrl+6"}},
    {ORCA_VIEW(1, 2), {"P", "Prepare / Preview", "Tab"}},

    BOTTOM_BAR,
};

static const macro_button_t orca_modify[] = {
    {LV_SYMBOL_COPY, "Clone", "Ctrl+K", 0x55BFFF},
    {LV_SYMBOL_PLUS, "Instance +", "+", 0x30E57B},
    {LV_SYMBOL_MINUS, "Instance -", "-", 0xFF5964},
    {"B", "Mesh Boolean", "B", 0x55BFFF},
    {"Y", "Assembly", "Y", 0x55BFFF},
    {LV_SYMBOL_EYE_OPEN, "Printable", "V", 0x8BE0FF},
    {LV_SYMBOL_OK, "Select All", "Ctrl+A", 0x30E57B},
    {LV_SYMBOL_CLOSE, "Deselect", "Esc", 0xAEB8C6},
    {LV_SYMBOL_COPY, "Copy", "Ctrl+C"},
    {LV_SYMBOL_PASTE, "Paste", "Ctrl+V"},
    {LV_SYMBOL_CUT, "Cut", "Ctrl+X"},
    {LV_SYMBOL_TRASH, "Delete All", "Ctrl+D", 0xFF5964},
};
/* Orange buttons only make sense in Preview: in Prepare, L / C / arrows do other things. */
static const macro_button_t orca_view[] = {
    {LV_SYMBOL_PLUS, "Zoom In", "I"},
    {LV_SYMBOL_MINUS, "Zoom Out", "O"},
    {LV_SYMBOL_BARS, "Sidebar", "Shift+Tab"},
    {LV_SYMBOL_SHUFFLE, "Prep / Preview", "Tab", 0x24D4C1},
    {LV_SYMBOL_LIST, "One Layer", "L", 0xFFAA2A},
    {LV_SYMBOL_FILE, "G-code", "C", 0xFFAA2A},
    {LV_SYMBOL_UP, "Layer Up", "Up", 0xFFAA2A},
    {LV_SYMBOL_DOWN, "Layer Down", "Down", 0xFFAA2A},
    {LV_SYMBOL_LEFT, "Move Back", "Left", 0xFFAA2A},
    {LV_SYMBOL_RIGHT, "Move Fwd", "Right", 0xFFAA2A},
    {LV_SYMBOL_PREV, "Start", "Home", 0xFFAA2A},
    {LV_SYMBOL_NEXT, "End", "End", 0xFFAA2A},
};
static const macro_button_t orca_support[] = {
    {LV_SYMBOL_EDIT, "Support Paint", "L", 0x8BE0FF},
    {"E", "Brim Ears", "E", 0x8BE0FF},
    {LV_SYMBOL_EDIT, "Seam Paint", "P", 0xA14CFF},
    {"~", "Fuzzy Skin", "H", 0xA14CFF},
};
/* Orca waits 0.5 s after "1" in case a two-digit filament number follows. */
static const macro_button_t orca_filament[] = {
    {"1", "Filament 1", "1", 0xFF5964},
    {"2", "Filament 2", "2", 0xFFAA2A},
    {"3", "Filament 3", "3", 0xFFE14D},
    {"4", "Filament 4", "4", 0x30E57B},
    {"5", "Filament 5", "5", 0x24D4C1},
    {"6", "Filament 6", "6", 0x55BFFF},
    {"7", "Filament 7", "7", 0xA14CFF},
    {"8", "Filament 8", "8", 0xFF7AC6},
    {"9", "Filament 9", "9", 0xE6F2FF},
    {LV_SYMBOL_TINT, "Color Paint", "N", 0x68B7FF},
};
static const macro_button_t orca_printer[] = {
    {LV_SYMBOL_UPLOAD, "Print Plate", "Ctrl+Shift+G", 0x31C8F5},
    {LV_SYMBOL_DOWNLOAD, "Export G-code", "Ctrl+G", 0x55BFFF},
    {LV_SYMBOL_REFRESH, "Slice", "Ctrl+R", 0x30E57B},
    {LV_SYMBOL_SAVE, "Save", "Ctrl+S", 0x55BFFF},
    {LV_SYMBOL_SAVE, "Save As", "Ctrl+Shift+S", 0x55BFFF},
};
static const macro_button_t orca_tools[] = {
    {"U", "Measure", "U", 0x68B7FF},
    {"Y", "Assembly", "Y", 0x55BFFF},
    {"T", "Add Text", "T", 0x68B7FF},
    {LV_SYMBOL_KEYBOARD, "Shortcuts", "?"},
    {LV_SYMBOL_GPS, "3Dconnexion", "Ctrl+M"},
};
static const macro_button_t orca_settings[] = {
    {LV_SYMBOL_SETTINGS, "Preferences", "Ctrl+P"},
    {LV_SYMBOL_LOOP, "Switch Tab", "Ctrl+Tab"},
    {"", "Jog Step", NULL, 0x30E57B, MACRO_ACTION_JOG_STEP},
};

static const macro_page_t orca_pages[] = {
    {NULL, NULL, NULL, 0, 0},
    {"MODIFY", "", NULL, 0, 4},
    {"VIEW", "", NULL, 0, 4},
    {"SUPPORT", "", NULL, 0, 4},
    {"FILAMENT", "", NULL, 0, 4},
    {"PRINTER", "", NULL, 0, 4},
    {"TOOLS", "", NULL, 0, 4},
    {"SETTINGS", "", NULL, 0, 4},
};

/* ======================================================================== */

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

const macro_profile_t macro_builtin_profiles[] = {
    [PROFILE_FUSION] = {"Fusion 360", ui_fusion_clean_rgb565, fusion_zones, COUNT(fusion_zones),
                        fusion_pages, COUNT(fusion_pages), 0xCB5B18, 0xF47721},
    [PROFILE_ORCA] = {"Orca Slicer", ui_orca_clean_rgb565, orca_zones, COUNT(orca_zones),
                      orca_pages, COUNT(orca_pages), 0x019C9E, 0x00E5D8},
};
const size_t macro_builtin_profile_count = COUNT(macro_builtin_profiles);
const macro_profile_t *macro_profiles = macro_builtin_profiles;
size_t macro_profile_count = COUNT(macro_builtin_profiles);
