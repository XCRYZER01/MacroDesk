#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <esp_err.h>
#include <lvgl.h>

#include "esp_lv_adapter_arduino.h"
#include "macro_deck_ui.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

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
        default: return "Unknown";
    }
}

static void on_macro_action(macro_action_t action, void *user_data)
{
    (void)user_data;
    Serial.printf("Touch action: %s (%d)\n", action_name(action), static_cast<int>(action));
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
