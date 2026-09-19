# MacroDeckUI

Arduino sketch for the MacroDesk touch control deck on the Waveshare
ESP32-S3-Touch-LCD-7.

Setup, board settings, build commands, how to change the background images and
how to add buttons are all in the [project README](../../README.md).

Quick reference:

- Open `MacroDeckUI.ino` in the Arduino IDE with **PSRAM enabled** and the
  `16M Flash (3MB APP/9.9MB FATFS)` partition scheme.
- Flash through the `USB TO UART` port. The keyboard output comes from the
  native `USB` port.
- Buttons, pages and touch areas are all in `macro_deck_profiles.c`.
- `ui_*_rgb565.c` are generated from `assets/*.png` with
  `tools/png_to_rgb565.py`. Don't edit them by hand.
