#!/usr/bin/env python3
"""Convert an 800x480 PNG background into the RGB565 C array the firmware uses.

The output matches LVGL's LV_COLOR_DEPTH 16 / LV_COLOR_16_SWAP 0 layout, so the
firmware can memcpy it straight into the canvas buffer.

    python tools/png_to_rgb565.py firmware/MacroDeckUI/assets/ui_orca_800x480.png

writes firmware/MacroDeckUI/ui_orca_rgb565.c with the symbol ui_orca_rgb565[].
Requires Pillow (pip install pillow).
"""

import argparse
import re
import sys
from pathlib import Path

from PIL import Image

WIDTH, HEIGHT = 800, 480
FIRMWARE_DIR = Path(__file__).resolve().parent.parent / "firmware" / "MacroDeckUI"


def default_name(png: Path) -> str:
    """ui_orca_800x480.png -> ui_orca"""
    return re.sub(r"_?\d+x\d+$", "", png.stem)


def convert(png: Path, out: Path, symbol: str) -> None:
    image = Image.open(png).convert("RGB")
    if image.size != (WIDTH, HEIGHT):
        sys.exit(f"{png}: expected {WIDTH}x{HEIGHT}, got {image.size[0]}x{image.size[1]}")

    pixels = image.load()
    lines = ["#include <stdint.h>", "", f"const uint16_t {symbol}[] = {{"]
    for y in range(HEIGHT):
        row = []
        for x in range(WIDTH):
            r, g, b = pixels[x, y]
            row.append("0x%04X," % (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)))
        lines.append("    " + "".join(row))
    lines.append("};")
    out.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")
    print(f"{png} -> {out} ({symbol}, {WIDTH * HEIGHT * 2:,} bytes of image data)")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("png", type=Path, help="800x480 PNG to convert")
    parser.add_argument("--name", help="base name, e.g. ui_orca (default: from the file name)")
    parser.add_argument("--out", type=Path, help="output .c file (default: firmware/MacroDeckUI/<name>_rgb565.c)")
    args = parser.parse_args()

    name = args.name or default_name(args.png)
    symbol = f"{name}_rgb565"
    out = args.out or FIRMWARE_DIR / f"{symbol}.c"
    convert(args.png, out, symbol)


if __name__ == "__main__":
    main()
