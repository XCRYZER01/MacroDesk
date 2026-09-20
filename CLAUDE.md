# CLAUDE.md

Guidance for Claude (and other AI coding assistants) working in this repository.
Many people who open this project are makers, not programmers. Explain what you
change in plain words, keep changes small, and say clearly what you verified on
hardware and what you did not.

## What this is

MacroDesk is Arduino firmware for the Waveshare ESP32-S3-Touch-LCD-7 (800 × 480
touch screen, ESP32-S3, 16 MB flash, 8 MB PSRAM). It acts as a USB keyboard: each
on-screen button sends a keyboard shortcut to Fusion 360 or OrcaSlicer on the PC.
UI: LVGL 8.4 on the Arduino-ESP32 core 3.0.7.

## Where things live

| File | Purpose |
|---|---|
| `firmware/MacroDeckUI/macro_deck_profiles.c` | **All buttons, pages, touch rectangles and profiles.** Almost every customization happens here. |
| `firmware/MacroDeckUI/macro_deck_ui.h` | Types: `macro_button_t`, `macro_zone_t`, `macro_page_t`, `macro_profile_t`, `macro_action_t`. |
| `firmware/MacroDeckUI/macro_deck_ui.c` | Engine: canvas, touch zones, on-demand sidebar pages, jog pad, profile switching. |
| `firmware/MacroDeckUI/MacroDeckUI.ino` | Board, display and USB bring-up; `send_keys()` parses key strings like `"Ctrl+Shift+G"`. |
| `firmware/MacroDeckUI/ui_*_rgb565.c` | Generated background images. **Never edit by hand**; regenerate with `tools/png_to_rgb565.py`. |
| `firmware/MacroDeckUI/assets/*.png` | 800 × 480 background sources. |
| `firmware/prebuilt/*.bin` | Ready-to-flash image; regenerate with `tools/make_prebuilt.py` after firmware changes. |
| `templates/` | Brand-free SVG design template and layout guides (touch rectangles drawn over the art). |
| `tools/` | `png_to_rgb565.py`, `make_layout_guides.py`, `make_prebuilt.py` (Python + Pillow). |

## How buttons work

- A button is `{icon, label, keys, accent, action, arg}`; usually only the first
  three fields are set: `{LV_SYMBOL_SAVE, "Save", "Ctrl+S"}`.
- `keys` syntax: modifiers `Ctrl+ Shift+ Alt+ Win+`, then one character or a named
  key (`Del Esc Tab Enter Space Backspace Insert Home End PgUp PgDn Up Down Left
  Right F1-F12`). `"search:Name"` = press S, type Name, Enter (Fusion command
  search, English UI only). Single letters are sent lowercase: `"Ctrl+N"` is
  Ctrl+n, not Ctrl+Shift+n.
- Buttons on image pages are `macro_zone_t`: an invisible rectangle that must sit
  over the button painted in the PNG. Use the `*_CELL` / `*_VIEW` / `SIDEBAR_ROW`
  macros. If you add a macro, also add its formula to `MACROS` in
  `tools/make_layout_guides.py`.
- Buttons on sidebar pages are drawn by LVGL; adding a line to the page array is
  enough.
- Non-key behaviour uses `action`: `MACRO_ACTION_PROFILE` / `PAGE` (with `arg`),
  `JOG_OPEN` (Orca Move opens the jog pad and still sends its keys), `JOG_STEP`,
  `NONE` (placeholder).

## Build, flash, verify

```sh
FQBN="esp32:esp32:waveshare_esp32_s3_touch_lcd_7:PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,USBMode=default"
arduino-cli compile --fqbn "$FQBN" --export-binaries firmware/MacroDeckUI
arduino-cli upload  --fqbn "$FQBN" --port <COM port> --input-dir firmware/MacroDeckUI/build/esp32.esp32.waveshare_esp32_s3_touch_lcd_7
arduino-cli monitor --port <COM port> --config baudrate=115200
```

- `USBMode=default` is USB-OTG (TinyUSB). Without it the board defaults to
  hardware CDC and the sketch stops at its `#error` guard.
- Flash through the `USB TO UART` port (CH343). The keyboard comes out of the
  other, native `USB` port. If taps appear in the serial log but nothing happens
  on the PC, the native port is usually not connected.
- A good boot ends with `USB HID keyboard ready (EXIO5 LOW)` and `Macro Deck UI ready`.
- Each tap logs `Touch: <label> -> <keys>`; page and profile switches also log
  `LVGL heap: N% used`. Ask the user to tap the buttons you changed and read the
  log. You cannot see the screen, so do not claim a visual result you have not
  checked.
- After changing firmware that is meant to be published, run
  `python tools/make_prebuilt.py` so the prebuilt image matches the source.

## Hard constraints: don't break these

- **EXIO5 LOW before `USB.begin()`**: native USB shares GPIO19/20 with CAN
  through the CH422G expander. Removing this makes the keyboard vanish on the PC.
- **PSRAM must be enabled**, or the board reboot-loops.
- **LVGL 8.4 only**; `lv_conf.h` needs `LV_COLOR_DEPTH 16`, `LV_COLOR_16_SWAP 0`
  and fonts Montserrat 12/14/16/26. The images are little-endian RGB565 for
  `LV_COLOR_16_SWAP 0`; flipping that swaps red and blue.
- **LVGL heap is 48 KB** (`LV_MEM_SIZE`). Sidebar pages are built on open and
  deleted on close; the largest page (12 buttons) uses about 80%. Keep pages at
  about 12 buttons or fewer, and check the `LVGL heap` log line after adding any.
- **Flash**: app partition 3 MB; each background image is 768,000 bytes and the
  sketch uses about 71%. One more full-screen image fits.
- The `search:` helper blocks the UI for about 0.7 s (it waits for Fusion's
  search box). Don't shorten the delays in `run_search()` without testing on
  Fusion.

## Verified facts (don't re-guess these)

- OrcaSlicer shortcuts were checked against Orca's source (`KBShortcutsDialog.cpp`,
  `GLCanvas3D.cpp`, `Gizmos/`). Orca has **no** shortcut for Add Plate, Split to
  Objects/Parts, Variable Layer Height, rotate/scale by value, or Z moves;
  arrows move the selection 10 mm in X/Y and Shift+arrows 1 mm.
- Fusion 360: Fit = F6 was confirmed on a real install. The Home view has no
  known default key. Visual styles: Ctrl+4 shaded, Ctrl+5 shaded with hidden
  edges, Ctrl+7 wireframe.

## Style

- Match the surrounding C style: 4-space indent, `static` helpers, short
  comments that explain *why*.
- Don't reformat or reorder unrelated code, and don't edit generated
  `ui_*_rgb565.c` files.
- The project is GPL-3.0. Don't copy icons or code from projects with
  incompatible licenses (OrcaSlicer's own icons are AGPL: draw new ones instead).
