#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <esp_err.h>
#include <lvgl.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

#include "esp_lv_adapter_arduino.h"
#include "macro_deck_ui.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

#if !defined(ARDUINO_USB_MODE) || ARDUINO_USB_MODE != 0
#error "MacroDeck USB HID requires Tools > USB Mode > USB-OTG (TinyUSB)"
#endif

static USBHIDKeyboard Keyboard;
static macro_action_t s_active_profile = MACRO_ACTION_PROFILE_FUSION;

static void hid_tap(uint8_t key)
{
    Keyboard.press(key);
    delay(12);
    Keyboard.releaseAll();
}

static void hid_combo(uint8_t modifier, uint8_t key)
{
    Keyboard.press(modifier);
    Keyboard.press(key);
    delay(12);
    Keyboard.releaseAll();
}

static void hid_combo3(uint8_t modifier1, uint8_t modifier2, uint8_t key)
{
    Keyboard.press(modifier1);
    Keyboard.press(modifier2);
    Keyboard.press(key);
    delay(12);
    Keyboard.releaseAll();
}

static void send_orca_shortcut(macro_action_t action)
{
    switch (action) {
        case MACRO_ACTION_ORCA_NEW_PROJECT: hid_combo(KEY_LEFT_CTRL, 'n'); break;
        case MACRO_ACTION_ORCA_OPEN_PROJECT: hid_combo(KEY_LEFT_CTRL, 'o'); break;
        case MACRO_ACTION_ORCA_SAVE_PROJECT: hid_combo(KEY_LEFT_CTRL, 's'); break;
        case MACRO_ACTION_ORCA_IMPORT_MODEL: hid_combo(KEY_LEFT_CTRL, 'i'); break;
        case MACRO_ACTION_ORCA_PREFERENCES: hid_combo(KEY_LEFT_CTRL, 'p'); break;
        case MACRO_ACTION_ORCA_ARRANGE: hid_tap('a'); break;
        case MACRO_ACTION_ORCA_AUTO_ORIENT: hid_tap('q'); break;
        case MACRO_ACTION_ORCA_LAY_FLAT: hid_tap('f'); break;
        case MACRO_ACTION_MOVE: hid_tap('m'); break;
        case MACRO_ACTION_ORCA_ROTATE: hid_tap('r'); break;
        case MACRO_ACTION_ORCA_SCALE: hid_tap('s'); break;
        case MACRO_ACTION_ORCA_CUT: hid_tap('c'); break;
        case MACRO_ACTION_ORCA_MESH_BOOLEAN: hid_tap('b'); break;
        case MACRO_ACTION_ORCA_ADD_TEXT: hid_tap('t'); break;
        case MACRO_ACTION_ORCA_SEAM_PAINTING: hid_tap('p'); break;
        case MACRO_ACTION_UNDO: hid_combo(KEY_LEFT_CTRL, 'z'); break;
        case MACRO_ACTION_REDO: hid_combo(KEY_LEFT_CTRL, 'y'); break;
        case MACRO_ACTION_ORCA_DELETE: hid_tap(KEY_DELETE); break;
        case MACRO_ACTION_ORCA_SLICE: hid_combo(KEY_LEFT_CTRL, 'r'); break;
        case MACRO_ACTION_ORCA_PRINT_PLATE:
            hid_combo3(KEY_LEFT_CTRL, KEY_LEFT_SHIFT, 'g');
            break;

        case MACRO_ACTION_ORCA_VIEW_DEFAULT: hid_combo(KEY_LEFT_CTRL, '0'); break;
        case MACRO_ACTION_ORCA_VIEW_TOP: hid_combo(KEY_LEFT_CTRL, '1'); break;
        case MACRO_ACTION_ORCA_VIEW_BOTTOM: hid_combo(KEY_LEFT_CTRL, '2'); break;
        case MACRO_ACTION_ORCA_VIEW_FRONT: hid_combo(KEY_LEFT_CTRL, '3'); break;
        case MACRO_ACTION_ORCA_VIEW_BEHIND: hid_combo(KEY_LEFT_CTRL, '4'); break;
        case MACRO_ACTION_ORCA_VIEW_LEFT: hid_combo(KEY_LEFT_CTRL, '5'); break;
        case MACRO_ACTION_ORCA_VIEW_RIGHT: hid_combo(KEY_LEFT_CTRL, '6'); break;
        case MACRO_ACTION_ORCA_VIEW_PREVIEW: hid_tap(KEY_TAB); break;
        default: break;
    }
}

static const char *action_name(macro_action_t action)
{
    switch (action) {
        case MACRO_ACTION_NEW_DESIGN: return "New Design";
        case MACRO_ACTION_OPEN: return "Open";
        case MACRO_ACTION_SAVE: return "Save";
        case MACRO_ACTION_UNDO: return "Undo";
        case MACRO_ACTION_REDO: return "Redo";
        case MACRO_ACTION_LINE: return "Line";
        case MACRO_ACTION_RECTANGLE: return "Rectangle";
        case MACRO_ACTION_CIRCLE: return "Circle";
        case MACRO_ACTION_ARC: return "Arc";
        case MACRO_ACTION_DIMENSION: return "Dimension";
        case MACRO_ACTION_EXTRUDE: return "Extrude";
        case MACRO_ACTION_REVOLVE: return "Revolve";
        case MACRO_ACTION_FILLET: return "Fillet";
        case MACRO_ACTION_CHAMFER: return "Chamfer";
        case MACRO_ACTION_SHELL: return "Shell";
        case MACRO_ACTION_MOVE: return "Move";
        case MACRO_ACTION_COMBINE: return "Combine";
        case MACRO_ACTION_HOLE: return "Hole";
        case MACRO_ACTION_PATTERN: return "Pattern";
        case MACRO_ACTION_MIRROR: return "Mirror";
        case MACRO_ACTION_VIEW_HOME: return "View Home";
        case MACRO_ACTION_VIEW_FIT: return "View Fit";
        case MACRO_ACTION_VIEW_ZOOM: return "View Zoom";
        case MACRO_ACTION_VIEW_PAN: return "View Pan";
        case MACRO_ACTION_VIEW_ORBIT: return "View Orbit";
        case MACRO_ACTION_DISPLAY_SHADED: return "Display Shaded";
        case MACRO_ACTION_DISPLAY_WIREFRAME: return "Display Wireframe";
        case MACRO_ACTION_DISPLAY_HIDDEN: return "Display Hidden";
        case MACRO_ACTION_PROFILE_FUSION: return "Profile Fusion 360";
        case MACRO_ACTION_PROFILE_ORCA: return "Profile Orca Slicer";
        case MACRO_ACTION_PROFILE_BAMBU: return "Profile Bambu Studio";
        case MACRO_ACTION_PROFILE_SYSTEM: return "Profile System";
        case MACRO_ACTION_NAV_HOME: return "Nav Home";
        case MACRO_ACTION_NAV_SKETCH: return "Nav Sketch";
        case MACRO_ACTION_NAV_SOLID: return "Nav Solid";
        case MACRO_ACTION_NAV_SURFACE: return "Nav Surface";
        case MACRO_ACTION_NAV_MESH: return "Nav Mesh";
        case MACRO_ACTION_NAV_SHEET_METAL: return "Nav Sheet Metal";
        case MACRO_ACTION_NAV_TOOLS: return "Nav Tools";
        case MACRO_ACTION_NAV_SETTINGS: return "Nav Settings";
        case MACRO_ACTION_ORCA_PREPARE: return "Orca Prepare";
        case MACRO_ACTION_ORCA_MODIFY: return "Orca Modify";
        case MACRO_ACTION_ORCA_VIEW: return "Orca View";
        case MACRO_ACTION_ORCA_SUPPORT: return "Orca Support";
        case MACRO_ACTION_ORCA_FILAMENT: return "Orca Filament";
        case MACRO_ACTION_ORCA_PRINTER: return "Orca Printer";
        case MACRO_ACTION_ORCA_TOOLS: return "Orca Tools";
        case MACRO_ACTION_ORCA_SETTINGS: return "Orca Settings";
        case MACRO_ACTION_ORCA_IMPORT_MODEL: return "Orca Import Model";
        case MACRO_ACTION_ORCA_ARRANGE: return "Orca Arrange";
        case MACRO_ACTION_ORCA_AUTO_ORIENT: return "Orca Auto Orient";
        case MACRO_ACTION_ORCA_LAY_FLAT: return "Orca Lay Flat";
        case MACRO_ACTION_ORCA_SPLIT_PARTS: return "Orca Split to Parts";
        case MACRO_ACTION_ORCA_ROTATE: return "Orca Rotate";
        case MACRO_ACTION_ORCA_SCALE: return "Orca Scale";
        case MACRO_ACTION_ORCA_NEGATIVE_VOLUME: return "Orca Negative Volume";
        case MACRO_ACTION_ORCA_ADD_SUPPORT: return "Orca Add Support";
        case MACRO_ACTION_ORCA_SUPPORT_PAINTING: return "Orca Support Painting";
        case MACRO_ACTION_ORCA_SUPPORT_BLOCKER: return "Orca Support Blocker";
        case MACRO_ACTION_ORCA_ADD_TEXT: return "Orca Add Text";
        case MACRO_ACTION_ORCA_SEAM_PAINTING: return "Orca Seam Painting";
        case MACRO_ACTION_ORCA_DELETE: return "Orca Delete";
        case MACRO_ACTION_ORCA_SLICE: return "Orca Slice";
        case MACRO_ACTION_ORCA_SEND_TO_PRINTER: return "Orca Send to Printer";
        case MACRO_ACTION_ORCA_DISPLAY_PREPARE: return "Orca Display Prepare";
        case MACRO_ACTION_ORCA_DISPLAY_PREVIEW: return "Orca Display Preview";
        case MACRO_ACTION_ORCA_DISPLAY_DEVICE: return "Orca Display Device";
        case MACRO_ACTION_ORCA_NEW_PROJECT: return "Orca New Project";
        case MACRO_ACTION_ORCA_OPEN_PROJECT: return "Orca Open Project";
        case MACRO_ACTION_ORCA_SAVE_PROJECT: return "Orca Save Project";
        case MACRO_ACTION_ORCA_PREFERENCES: return "Orca Preferences";
        case MACRO_ACTION_ORCA_CUT: return "Orca Cut";
        case MACRO_ACTION_ORCA_MESH_BOOLEAN: return "Orca Mesh Boolean";
        case MACRO_ACTION_ORCA_PRINT_PLATE: return "Orca Print Plate";
        case MACRO_ACTION_ORCA_VIEW_DEFAULT: return "Orca View Default";
        case MACRO_ACTION_ORCA_VIEW_TOP: return "Orca View Top";
        case MACRO_ACTION_ORCA_VIEW_BOTTOM: return "Orca View Bottom";
        case MACRO_ACTION_ORCA_VIEW_FRONT: return "Orca View Front";
        case MACRO_ACTION_ORCA_VIEW_BEHIND: return "Orca View Behind";
        case MACRO_ACTION_ORCA_VIEW_LEFT: return "Orca View Left";
        case MACRO_ACTION_ORCA_VIEW_RIGHT: return "Orca View Right";
        case MACRO_ACTION_ORCA_VIEW_PREVIEW: return "Orca View Preview";
        default: return "Unknown";
    }
}

static void on_macro_action(macro_action_t action, void *user_data)
{
    (void)user_data;
    Serial.printf("Touch action: %s (%d)\n", action_name(action), static_cast<int>(action));

    if (action == MACRO_ACTION_PROFILE_FUSION || action == MACRO_ACTION_PROFILE_ORCA) {
        s_active_profile = action;
        return;
    }
    if (s_active_profile == MACRO_ACTION_PROFILE_ORCA) {
        send_orca_shortcut(action);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Macro Deck UI start");

    Board *board = new Board();
    if ((board == nullptr) || !board->init()) {
        Serial.println("Board init failed");
        while (true) delay(1000);
    }

    const esp_lv_adapter_rotation_t rotation = ESP_LV_ADAPTER_ROTATE_0;
    const esp_lv_adapter_tear_avoid_mode_t tear_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_DEFAULT_RGB;
    const uint8_t frame_buffer_count = esp_lv_adapter_get_required_frame_buffer_count(tear_mode, rotation);

    LCD *lcd = board->getLCD();
    if (lcd == nullptr) {
        Serial.println("LCD device is not available");
        while (true) delay(1000);
    }

    auto *lcd_bus = lcd->getBus();
    if (lcd_bus->getBasicAttributes().type == ESP_PANEL_BUS_TYPE_RGB) {
        lcd->configFrameBufferNumber(frame_buffer_count);
        static_cast<BusRGB *>(lcd_bus)->configRGB_BounceBufferSize(lcd->getFrameWidth() * 10);
    }

    assert(board->begin());

    // GPIO19/20 are shared by native USB and CAN on this board. CH422G EXIO5
    // must be LOW to physically route the Type-C USB port to the ESP32-S3.
    constexpr uint8_t USB_CAN_SELECT_EXIO = 5;
    auto *io_expander = board->getIO_Expander();
    if (io_expander == nullptr ||
        !io_expander->getBase()->digitalWrite(USB_CAN_SELECT_EXIO, LOW)) {
        Serial.println("USB/CAN mux switch failed");
    } else {
        delay(20);
        USB.productName("MacroDesk Control Deck");
        USB.manufacturerName("XCRYZER01");
        Keyboard.begin();
        if (!USB.begin()) {
            Serial.println("USB HID start failed");
        } else {
            Serial.println("USB HID keyboard ready (EXIO5 LOW)");
        }
    }

    esp_lv_adapter_config_t adapter_config = ESP_LV_ADAPTER_DEFAULT_CONFIG();
    adapter_config.task_stack_size = 12 * 1024;
    adapter_config.task_priority = 2;
    adapter_config.task_core_id = ARDUINO_RUNNING_CORE;
    ESP_ERROR_CHECK(esp_lv_adapter_init(&adapter_config));

    esp_lv_adapter_display_config_t disp_config = ESP_LV_ADAPTER_DISPLAY_RGB_DEFAULT_CONFIG(
        lcd, lcd->getFrameWidth(), lcd->getFrameHeight(), rotation
    );
    disp_config.profile.use_psram = true;

    lv_display_t *display = esp_lv_adapter_register_display(&disp_config);
    assert(display != nullptr);

    if (board->getTouch() != nullptr) {
        esp_lv_adapter_touch_config_t touch_config =
            ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(display, board->getTouch());
        lv_indev_t *touch = esp_lv_adapter_register_touch(&touch_config);
        assert(touch != nullptr);
    }

    ESP_ERROR_CHECK(esp_lv_adapter_start());
    ESP_ERROR_CHECK(esp_lv_adapter_lock(-1));
    macro_deck_ui_create(on_macro_action, nullptr);
    macro_deck_ui_set_clock("2026-09-17", "10:24");
    esp_lv_adapter_unlock();

    Serial.println("Macro Deck UI ready");
}

void loop()
{
    delay(1000);
}
