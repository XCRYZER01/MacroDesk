#!/usr/bin/env python3
"""Turn the Arduino build output into a browser-ready merged image in root/prebuilt/.

Build first with `arduino-cli compile ... --export-binaries firmware/MacroDeckUI`, then:

    python tools/make_prebuilt.py

The build's *.merged.bin is a full 16 MB flash image. Everything after the app
is erased flash (0xFF), so it is cut off; the result is flashed at 0x0.
"""

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "firmware/MacroDeckUI/build/esp32.esp32.waveshare_esp32_s3_touch_lcd_7"
OUT = ROOT / "prebuilt/MacroDesk-esp32s3-touch-lcd-7.bin"
LEGACY_OUT = ROOT / "firmware/prebuilt/MacroDesk-esp32s3-touch-lcd-7.bin"
APP_OFFSET = 0x10000


def main() -> None:
    merged = BUILD / "MacroDeckUI.ino.merged.bin"
    app = BUILD / "MacroDeckUI.ino.bin"
    if not merged.exists() or not app.exists():
        sys.exit(f"Build output not found in {BUILD}. Compile with --export-binaries first.")

    image = merged.read_bytes()
    app_bytes = app.read_bytes()
    end = APP_OFFSET + len(app_bytes)
    end = (end + 0xFFF) & ~0xFFF   # round up to a 4 KB flash sector

    if image[APP_OFFSET:APP_OFFSET + len(app_bytes)] != app_bytes:
        sys.exit("merged.bin does not contain the app at 0x10000; is the build stale?")
    if any(b != 0xFF for b in image[end:]):
        sys.exit("merged.bin has data after the app; refusing to trim it.")

    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_bytes(image[:end])
    print(f"{OUT} ({end:,} bytes) - flash at 0x0")
    LEGACY_OUT.parent.mkdir(parents=True, exist_ok=True)
    LEGACY_OUT.write_bytes(image[:end])
    print(f"{LEGACY_OUT} compatibility copy")


if __name__ == "__main__":
    main()
