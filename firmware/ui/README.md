# Macro Deck UI (LVGL 8.4)

This folder contains the first 800 × 480 UI milestone for the Waveshare
ESP32-S3-Touch-LCD-7. Hardware, touch and USB HID code are deliberately kept
outside this module.

For the complete, hardware-tested Arduino sketch, use
`../MacroDeckUI/MacroDeckUI.ino`.

## Integrate with the Waveshare Arduino example

1. Install/open Waveshare's `examples/Arduino/examples/09_lvgl_v8_demo`.
2. Copy `macro_deck_ui.c` and `macro_deck_ui.h` into that sketch folder.
3. Remove the call that creates the stock LVGL demo.
4. Include the header and create the UI only after the LVGL adapter has started:

```cpp
#include "macro_deck_ui.h"

static void on_macro_action(macro_action_t action, void *user_data)
{
    Serial.printf("UI action: %d\n", static_cast<int>(action));
    // The USB HID mapping will be connected here in the next milestone.
}

// After ESP_ERROR_CHECK(esp_lv_adapter_start());
ESP_ERROR_CHECK(esp_lv_adapter_lock(-1));
macro_deck_ui_create(on_macro_action, nullptr);
esp_lv_adapter_unlock();
```

The UI uses only LVGL widgets and the Montserrat 12, 14, 16 and 26 fonts that
are already enabled in Waveshare's bundled LVGL 8.4 `lv_conf.h`.

## Current behavior

- All command, navigation, view and profile buttons are touchable.
- A single callback reports a strongly typed `macro_action_t` action.
- The layout is fixed to the panel's native landscape resolution (800 × 480).
- No command is sent to the PC yet; this milestone is UI-only.
