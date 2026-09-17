# MacroDeckUI

Complete Arduino sketch for the Waveshare ESP32-S3-Touch-LCD-7 control-deck
UI. It is based on Waveshare's LVGL 8 adapter and custom board configuration.

## Required board settings

- Board: `Waveshare ESP32-S3-Touch-LCD-7`
- Flash Size: `16MB (128Mb)`
- Partition Scheme: `16M Flash (3MB APP/9.9MB FATFS)`
- PSRAM: `Enabled`
- Upload port: `COM13` (current machine)

PSRAM must be enabled. The 800 × 480 RGB framebuffer cannot be allocated from
internal SRAM and the board will reboot if this option is disabled.

## Verified command

```powershell
$cli = "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
$fqbn = 'esp32:esp32:waveshare_esp32_s3_touch_lcd_7:PSRAM=enabled,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB'

& $cli compile --fqbn $fqbn --export-binaries 'D:\Macro Dash\firmware\MacroDeckUI'
& $cli upload --port COM13 --fqbn $fqbn --input-dir 'D:\Macro Dash\firmware\MacroDeckUI\build\esp32.esp32.waveshare_esp32_s3_touch_lcd_7'
```

## Runtime verification

At 115200 baud the successful boot ends with:

```text
Board begin success
Macro Deck UI ready
```

Touching a control prints its action name and numeric action ID. USB HID is not
enabled in this UI milestone.
