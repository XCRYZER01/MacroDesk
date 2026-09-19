# MacroDesk

A 7-inch touch control deck for **Fusion 360** and **OrcaSlicer**, built on the
Waveshare ESP32-S3-Touch-LCD-7. It plugs into a PC over USB and shows up as a
normal keyboard: every button on the screen sends that program's own keyboard
shortcut, so the PC needs no driver and no companion app.

| OrcaSlicer | Fusion 360 |
|---|---|
| ![OrcaSlicer page](firmware/MacroDeckUI/assets/ui_orca_800x480.png) | ![Fusion 360 page](firmware/MacroDeckUI/assets/ui_reference_800x480.png) |

## Features

- **Two profiles**, switched from the bottom bar: OrcaSlicer and Fusion 360.
- **OrcaSlicer**: a 6 × 4 grid of 23 tools, a Slice Plate button, eight camera
  views, and a jog pad that moves the selected model 10 mm or 1 mm per tap.
- **Fusion 360**: sketch and solid tools, visual styles, Fit (F6), workspace
  switching. Commands with no default shortcut, such as Revolve or Shell, are
  typed into Fusion's `S` command search automatically.
- **Sidebar sub-pages** on both profiles: seven extra pages each, for example
  Modify, Filament (set filament 1–9) and Printer (Print Plate, Export G-code)
  on Orca, and Sketch, Solid, Surface, Mesh and Sheet Metal on Fusion.
- Plug-and-play USB HID keyboard, with no driver or companion app. The shortcuts are the
  Windows/Linux ones (`Ctrl`). On macOS most of them would need `Cmd` instead.

## Hardware

| Part | Notes |
|---|---|
| Waveshare ESP32-S3-Touch-LCD-7 | 800 × 480 RGB LCD, GT911 capacitive touch, 16 MB flash, 8 MB PSRAM |
| 2 × USB-C data cables | One for the keyboard, one for flashing and logs |

The board has **two USB-C ports**, and they do different things:

| Port | Used for |
|---|---|
| `USB TO UART` (CH343) | Flashing and the serial log. Shows up as a COM port. |
| `USB` (native ESP32-S3) | **The keyboard.** Plug this one into the PC you want to control. |

Keep both plugged in while developing. For daily use only the native port is
needed.

> **Board quirk:** the native USB data lines (GPIO19/20) are shared with the CAN
> transceiver through the CH422G IO expander. The firmware drives `EXIO5` low at
> boot to route them to USB. Without that step Windows never sees the keyboard,
> even though TinyUSB reports that it started.

## Getting started

### 1. Toolchain

| Component | Version tested |
|---|---|
| Arduino IDE 2.x or `arduino-cli` | – |
| esp32 board package (Espressif) | 3.0.7 |
| `ESP32_Display_Panel` | 1.0.0 |
| `ESP32_IO_Expander` | 1.0.1 |
| `esp-lib-utils` | 0.1.2 |
| `lvgl` | **8.4.0** (LVGL 9 is not supported) |

Install the libraries from the Arduino Library Manager.

### 2. `lv_conf.h`

LVGL reads its configuration from `lv_conf.h` placed next to the `lvgl` folder
in your Arduino `libraries` directory. Start from the template shipped with
`ESP32_Display_Panel` and check these values:

```c
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0            // the background images depend on this
#define LV_MEM_SIZE (48U * 1024U)     // see "Memory" below
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_26 1
```

### 3. Board settings

| Setting | Value |
|---|---|
| Board | `Waveshare ESP32-S3-Touch-LCD-7` |
| Flash Size | `16MB (128Mb)` |
| Partition Scheme | `16M Flash (3MB APP/9.9MB FATFS)` |
| PSRAM | `Enabled` (**required**, otherwise the framebuffer allocation fails and the board reboots) |
| USB Mode | default (USB-OTG / TinyUSB) |

### 4. Build and flash

In the Arduino IDE, open `firmware/MacroDeckUI/MacroDeckUI.ino`, pick the
`USB TO UART` COM port and press Upload.

With `arduino-cli`:

```sh
FQBN="esp32:esp32:waveshare_esp32_s3_touch_lcd_7:PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB"

arduino-cli compile --fqbn "$FQBN" --export-binaries firmware/MacroDeckUI
arduino-cli upload  --fqbn "$FQBN" --port <COM port or /dev/tty...> \
  --input-dir firmware/MacroDeckUI/build/esp32.esp32.waveshare_esp32_s3_touch_lcd_7
arduino-cli monitor --port <same port> --config baudrate=115200
```

A good boot prints:

```text
USB HID keyboard ready (EXIO5 LOW)
Macro Deck UI ready
```

Every tap prints the action it triggered, for example
`Touch action: Orca View Top (78)`, which is the quickest way to check touch
zones.

## Using it

1. Plug the native USB port into the PC. It appears as a USB keyboard named
   `MacroDesk Control Deck`.
2. Pick **Fusion 360** or **Orca Slicer** in the bottom bar.
3. Click once inside the program's 3D view so that it has keyboard focus, then
   use the deck.

Notes:

- **Orca → Move** opens the jog pad. The arrows move the selected model, the
  centre button switches between 10 mm and 1 mm, and holding an arrow repeats
  it.
- **Orca → View** page: the orange buttons only make sense in Preview. In
  Prepare the same keys run other commands (`L` = support painting,
  `C` = cut, arrows = move the model).
- **Fusion `S search` buttons** need Fusion's user interface in English, because
  they type the command name.

## How the firmware is put together

```text
firmware/MacroDeckUI/
├── MacroDeckUI.ino          board + USB bring-up, keyboard shortcut tables
├── macro_deck_ui.h          macro_action_t: one enum value per button
├── macro_deck_ui.c          screen, touch zones, sub-pages, jog pad
├── ui_orca_rgb565.c         Orca background image as a C array (generated)
├── ui_reference_rgb565.c    Fusion background image as a C array (generated)
├── assets/                  editable 800×480 PNG sources of the backgrounds
├── esp_lv_adapter_arduino.* LVGL display/touch adapter (from Waveshare)
└── esp_panel_board_custom_conf.h   board pin/timing config (from Waveshare)
tools/png_to_rgb565.py       PNG → C array converter
```

The screen has three layers:

1. **Background image.** Each profile is one full-screen 800 × 480 picture
   drawn by a designer, stored as an RGB565 array in flash and copied into a
   PSRAM canvas with `memcpy` when the profile changes.
2. **Invisible touch zones ("hotspots").** Transparent LVGL buttons placed
   exactly over the buttons painted in the image. They only highlight while
   pressed. This is why a button's position in the image and its coordinates in
   `macro_deck_ui.c` must match.
3. **LVGL overlays.** Real LVGL widgets on top: the sidebar highlight, the
   sub-pages and the jog pad. Sub-pages are built when opened and deleted when
   closed, because the LVGL heap is small.

A tap flows like this:

```text
touch → hotspot (action_spec_t) → action_event_cb() → on_macro_action() in the .ino
      → send_orca_shortcut() / send_fusion_shortcut() → USB HID keyboard
```

## Changing a background

1. Edit the PNG in `firmware/MacroDeckUI/assets/`. It must stay **exactly
   800 × 480**. Any editor works: Figma, Photoshop, GIMP and so on.
2. Convert it:

   ```sh
   pip install pillow
   python tools/png_to_rgb565.py firmware/MacroDeckUI/assets/ui_orca_800x480.png
   ```

   This rewrites `firmware/MacroDeckUI/ui_orca_rgb565.c`. Use `--name` and
   `--out` for a new image.
3. If you moved, resized or added buttons in the picture, update their
   touch-zone coordinates in `macro_deck_ui.c` (next section).
4. Build and flash.

Things to know:

- Each image is 768,000 bytes (800 × 480 × 2) of the 3 MB app partition. The
  sketch currently uses about 71%, so about one more full-screen image fits.
  For more, switch to a larger app partition, or load images from the FAT
  partition or the SD card into PSRAM at boot.
- If red and blue come out swapped on the panel, `LV_COLOR_16_SWAP` does not
  match. The converter writes plain little-endian RGB565 for
  `LV_COLOR_16_SWAP 0`.
- Coordinates of the current layout:

  | Area | Position (x, y) and size |
  |---|---|
  | Orca grid 6 × 4 | x = 134 + col × 80, y = 88 + row × 85, 76 × 82 |
  | Fusion grid 5 × 4 | x = 133 + col × 97, y = 88 + row × 85, 93 × 82 |
  | Sidebar rows | x 4, y = 84 + row × 42, 123 × 41 |
  | Orca view panel 3 × 3 | x = 626 + col × 54, y = 186 + row × 70, 52 × 64 |
  | Orca Slice Plate / Fusion workspace box | (620, 88), 168 × 63 |
  | Sub-page area | (131, 84), 486 × 344 |
  | Bottom bar | Fusion (78, 432), Orca (208, 432), System (337, 432), height 40 |

## Adding or changing a button

Example: an Orca "Print Plate" button in the empty grid slot.

1. **Action** – add a value to `macro_action_t` in `macro_deck_ui.h`, or reuse
   one that already exists (`MACRO_ACTION_ORCA_PRINT_PLATE` does).
2. **Touch zone** – in `macro_deck_ui.c`, add an `action_spec_t` to the
   `orca_actions[]` list. Entries map to grid cells in reading order, so the
   24th entry becomes the bottom-right cell.

   ```c
   {LV_SYMBOL_UPLOAD, "Print Plate", "Ctrl + Shift + G", MACRO_ACTION_ORCA_PRINT_PLATE, 0x31C8F5},
   ```

   For a button outside a grid, call `make_hotspot(parent, &spec, x, y, w, h)`.
   The icon, label and colour of an `action_spec_t` are only drawn on
   sub-pages. On image pages the picture shows the button.
3. **Key** – in `MacroDeckUI.ino`, map the action in `send_orca_shortcut()` or
   `send_fusion_shortcut()` with `hid_tap(key)`, `hid_combo(mod, key)` or
   `hid_combo3(mod1, mod2, key)`. For a Fusion command without a shortcut, add
   its name to `fusion_search_term()` instead.
4. Optionally add a readable name in `action_name()` for the serial log.
5. Paint the button into the PNG and convert it (see above).

Sub-page buttons need no image work. Add an entry to the page's list in
`orca_page_def()` or `fusion_page_def()` and it is drawn automatically.

## Keyboard mapping sources

- OrcaSlicer: the official
  [keyboard shortcuts](https://github.com/OrcaSlicer/OrcaSlicer/wiki/keyboard-shortcuts)
  page, checked against the source code (`KBShortcutsDialog.cpp`,
  `GLCanvas3D.cpp`, `Gizmos/GLGizmo*.cpp`).
- Fusion 360: default shortcuts; Fit = F6 was confirmed on a real install.

The full per-button tables are in `send_orca_shortcut()`,
`send_fusion_shortcut()` and `fusion_search_term()`.

## Memory

- **LVGL heap (`LV_MEM_SIZE`, 48 KB)** is the tightest limit. The largest
  sub-page (12 buttons) brings it to about 81% used. The firmware prints
  `LVGL heap: …` on every page switch, so watch that line when adding buttons.
- **Flash**: the sketch uses about 2.24 MB of the 3 MB app partition.
- **PSRAM** holds the framebuffers and the 768 KB background canvas.

## Limitations and ideas

- A keyboard cannot rotate or scale by an angle or percentage, move along Z,
  add a plate or split objects in Orca. Those need a mouse (HID mouse) or a
  helper program on the PC.
- The Fusion *Home* view button is not mapped yet.
- Ideas: a System page with media and volume keys (USB Consumer Control), an
  HID mouse for orbit/pan/zoom, a Bambu Lab / Klipper print-status dashboard
  over Wi-Fi, and loading images and button layouts from the SD card.

## Troubleshooting

| Symptom | Fix |
|---|---|
| Taps show in the serial log but nothing happens on the PC | The native USB port is not connected, or the program's window does not have focus. |
| Windows does not list the keyboard | Use a data cable on the native port. Check the log for `USB HID keyboard ready (EXIO5 LOW)`. |
| Board reboot loop at start | PSRAM is not enabled in the board settings. |
| Red and blue swapped | `LV_COLOR_16_SWAP` must be `0`, or regenerate the images to match. |
| A button reacts in the wrong place | Its hotspot coordinates don't match the picture. Tap it and read the serial log. |
| Crash after adding many sub-page buttons | The LVGL heap is out of memory. Check the `LVGL heap` log line. |

## Credits

- Display/touch adapter and board configuration are based on the official
  Waveshare ESP32-S3-Touch-LCD-7 LVGL 8 example.
- Built on [LVGL](https://lvgl.io), Espressif's
  [ESP32_Display_Panel](https://github.com/esp-arduino-libs/ESP32_Display_Panel)
  and the Arduino-ESP32 core.
## License

MacroDesk is licensed under the **GNU General Public License v3.0**. See
[LICENSE](LICENSE).

You may use, modify and share it. If you distribute a modified version, for
example by publishing a fork or selling or giving away devices running your
firmware, you must release your source code under the same license.

`esp_panel_board_custom_conf.h` is © Espressif Systems and keeps its original
Apache-2.0 license, which is compatible with GPL-3.0.

## Trademarks

MacroDesk is an independent project. It is not affiliated with, endorsed by or
sponsored by Autodesk, Inc., the OrcaSlicer project, Bambu Lab, Waveshare or
Espressif. Fusion 360, Autodesk and all other product names and logos shown in
the screenshots and artwork belong to their respective owners and are used
only to identify the software the deck controls.
