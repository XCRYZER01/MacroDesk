# MacroDesk

Touch control deck for the Waveshare ESP32-S3-Touch-LCD-7. The current
milestone provides an 800 × 480 Fusion 360 interface with working capacitive
touch input and typed actions ready to connect to USB HID shortcuts.

## Hardware

- Waveshare ESP32-S3-Touch-LCD-7
- 800 × 480 RGB display
- GT911 capacitive touch controller
- 16 MB flash and 8 MB PSRAM

## Firmware

Open [`firmware/MacroDeckUI/MacroDeckUI.ino`](firmware/MacroDeckUI/MacroDeckUI.ino)
with Arduino IDE. Required board settings and verified CLI commands are in
[`firmware/MacroDeckUI/README.md`](firmware/MacroDeckUI/README.md).

The firmware has been compiled, flashed and tested on the target board. Display,
PSRAM, LVGL and touch input are working. USB HID is the next milestone.

## Upstream components

The display/touch adapter and board configuration are based on the official
Waveshare ESP32-S3-Touch-LCD-7 LVGL 8 example.
