#include <Arduino.h>
#include <esp_display_panel.hpp>
#include <esp_err.h>
#include <lvgl.h>
#include <strings.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

#include "esp_lv_adapter_arduino.h"
#include "macro_deck_runtime.h"
#include "macro_deck_ui.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

#if !defined(ARDUINO_USB_MODE) || ARDUINO_USB_MODE != 0
#error "MacroDeck USB HID requires Tools > USB Mode > USB-OTG (TinyUSB)"
#endif

static USBHIDKeyboard Keyboard;

/* ---- Key strings --------------------------------------------------------
 * Buttons describe their keys as text, e.g. "Ctrl+Shift+G", "F6", "Del",
 * "+", or "search:Revolve". See macro_button_t in macro_deck_ui.h.
 */

struct NamedKey {
    const char *name;
    uint8_t code;
};

static const NamedKey kModifiers[] = {
    {"Ctrl", KEY_LEFT_CTRL}, {"Shift", KEY_LEFT_SHIFT}, {"Alt", KEY_LEFT_ALT}, {"Win", KEY_LEFT_GUI},
};

static const NamedKey kNamedKeys[] = {
    {"Del", KEY_DELETE}, {"Delete", KEY_DELETE}, {"Esc", KEY_ESC}, {"Tab", KEY_TAB},
    {"Enter", KEY_RETURN}, {"Return", KEY_RETURN}, {"Space", ' '}, {"Backspace", KEY_BACKSPACE},
    {"Insert", KEY_INSERT}, {"Home", KEY_HOME}, {"End", KEY_END},
    {"PgUp", KEY_PAGE_UP}, {"PgDn", KEY_PAGE_DOWN},
    {"Up", KEY_UP_ARROW}, {"Down", KEY_DOWN_ARROW}, {"Left", KEY_LEFT_ARROW}, {"Right", KEY_RIGHT_ARROW},
    {"F1", KEY_F1}, {"F2", KEY_F2}, {"F3", KEY_F3}, {"F4", KEY_F4}, {"F5", KEY_F5}, {"F6", KEY_F6},
    {"F7", KEY_F7}, {"F8", KEY_F8}, {"F9", KEY_F9}, {"F10", KEY_F10}, {"F11", KEY_F11}, {"F12", KEY_F12},
};

static void hid_tap(uint8_t key)
{
    Keyboard.press(key);
    delay(12);
    Keyboard.releaseAll();
}

/* Fusion 360: commands without a default key run through the S command search box. */
static void run_search(const char *term)
{
    hid_tap('s');
    delay(400);   // let the search box open before typing
    Keyboard.print(term);
    delay(300);   // let the results list settle
    hid_tap(KEY_RETURN);
}

static bool send_keys(const char *keys)
{
    if (strncmp(keys, "search:", 7) == 0) {
        run_search(keys + 7);
        return true;
    }

    uint8_t mods[4];
    size_t mod_count = 0;
    const char *p = keys;
    bool matched = true;
    while (matched) {
        matched = false;
        for (const NamedKey &mod : kModifiers) {
            const size_t len = strlen(mod.name);
            if (strncasecmp(p, mod.name, len) == 0 && p[len] == '+' && p[len + 1] != '\0') {
                if (mod_count < sizeof(mods)) mods[mod_count++] = mod.code;
                p += len + 1;
                matched = true;
                break;
            }
        }
    }

    uint8_t code = 0;
    if (p[0] != '\0' && p[1] == '\0') {
        char c = p[0];
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');   // "Ctrl+N" means n, not Shift+n
        code = static_cast<uint8_t>(c);
    } else {
        for (const NamedKey &key : kNamedKeys) {
            if (strcasecmp(p, key.name) == 0) {
                code = key.code;
                break;
            }
        }
    }
    if (code == 0) {
        Serial.printf("Unknown key \"%s\" in \"%s\"\n", p, keys);
        return false;
    }

    for (size_t i = 0; i < mod_count; ++i) Keyboard.press(mods[i]);
    Keyboard.press(code);
    delay(12);
    Keyboard.releaseAll();
    return true;
}

static void on_button(const macro_button_t *button, void *user_data)
{
    (void)user_data;
    static const char *const kActionNames[] = {
        "keys", "none", "profile", "page", "jog open", "jog move", "jog step", "jog close", "text",
    };
    const bool has_label = button->label != nullptr && button->label[0] != '\0';
    const char *name = has_label ? button->label
                       : button->action < sizeof(kActionNames) / sizeof(kActionNames[0])
                           ? kActionNames[button->action] : "?";
    Serial.printf("Touch: %s", name);
    if (button->keys != nullptr) Serial.printf(" -> %s", button->keys);
    Serial.println();

    if (button->action == MACRO_ACTION_PAGE || button->action == MACRO_ACTION_PROFILE) {
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        Serial.printf("LVGL heap: %u%% used, %u bytes free, %u%% fragmented\n",
                      mon.used_pct, static_cast<unsigned>(mon.free_size), mon.frag_pct);
    }

    if (button->keys != nullptr) {
        if (button->action == MACRO_ACTION_TEXT) Keyboard.print(button->keys);
        else send_keys(button->keys);
    }
}
void setup()
{
    // Runtime profile uploads arrive as sustained binary data at 115200 baud.
    // The default UART ring is too small while FFat writes and drops bytes.
    Serial.setRxBufferSize(64 * 1024);
    Serial.begin(115200);
    Serial.println("Macro Deck UI start");
    macro_deck_runtime_begin();

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
    macro_deck_ui_create(on_button, nullptr);
    esp_lv_adapter_unlock();

    Serial.println("Macro Deck UI ready");
}

void loop()
{
    macro_deck_runtime_poll();
    delay(2);
}
