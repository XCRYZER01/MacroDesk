#!/usr/bin/env python3
"""Draw the touch zones of macro_deck_profiles.c over the background images.

    python tools/make_layout_guides.py

writes templates/layout_guide_orca.png, templates/layout_guide_fusion.png and
the brand-free templates/deck_template_800x480.svg. Run it after moving zones
to check that every rectangle still sits on its painted button. If you add a
new *_CELL-style macro in macro_deck_profiles.c, add its formula to MACROS.
Requires Pillow (pip install pillow).
"""
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "templates"
OUT.mkdir(exist_ok=True)
PROFILES = (ROOT / "firmware/MacroDeckUI/macro_deck_profiles.c").read_text(encoding="utf-8")

MACROS = {
    "SIDEBAR_ROW": lambda i: (4, 84 + i * 42, 123, 41),
    "FUSION_CELL": lambda c, r: (133 + c * 97, 88 + r * 85, 93, 82),
    "ORCA_CELL": lambda c, r: (134 + c * 80, 88 + r * 85, 76, 82),
    "ORCA_VIEW": lambda c, r: (626 + c * 54, 186 + r * 70, 52, 64),
    "FUSION_VIEW": lambda c, r: (626 + c * 54, 128 + r * 90, 52, 84),
}
CONST = {
    "BOTTOM_FUSION": (78, 432, 128, 40),
    "BOTTOM_ORCA": (208, 432, 128, 40),
    "BOTTOM_SYSTEM": (337, 432, 118, 40),
    "RIGHT_TOP_BOX": (620, 88, 168, 63),
}
BOTTOM = [(CONST["BOTTOM_FUSION"], "Fusion 360"), (CONST["BOTTOM_ORCA"], "Orca Slicer"),
          (CONST["BOTTOM_SYSTEM"], "System")]


def zones(array_name):
    """Parse {RECT, {icon, "Label", ...}} lines of one zone array."""
    body = PROFILES.split(f"static const macro_zone_t {array_name}[] = {{", 1)[1].split("\n};", 1)[0]
    found = []
    for line in body.splitlines():
        line = line.strip()
        if not line.startswith("{") or line.startswith("/*"):
            continue
        label = re.search(r'\{[^{}]*?,\s*"([^"]*)"', line)
        label = label.group(1) if label else "?"
        m = re.match(r"\{(\w+)\(([^)]*)\)", line)
        if m and m.group(1) in MACROS:
            args = [int(a) for a in m.group(2).split(",")]
            found.append((MACROS[m.group(1)](*args), label))
            continue
        m = re.match(r"\{(\w+),", line)
        if m and m.group(1) in CONST:
            found.append((CONST[m.group(1)], label))
            continue
        m = re.match(r"\{(\d+),\s*(\d+),\s*(\d+),\s*(\d+),", line)
        if m:
            found.append((tuple(int(v) for v in m.groups()), label))
    if "BOTTOM_BAR" in body:
        found += BOTTOM
    return found


def font(size):
    for name in ("seguisb.ttf", "segoeui.ttf", "arial.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            pass
    return ImageFont.load_default()


def guide(png_name, array_name, out_name, colour):
    base = Image.open(ROOT / "firmware/MacroDeckUI/assets" / png_name).convert("RGBA")
    over = Image.new("RGBA", base.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(over)
    small = font(10)
    for (x, y, w, h), label in zones(array_name):
        d.rectangle([x, y, x + w - 1, y + h - 1], fill=colour + (60,), outline=colour + (255,), width=2)
        d.rectangle([x + 2, y + 2, x + 2 + d.textlength(label, font=small) + 4, y + 15], fill=(0, 0, 0, 200))
        d.text((x + 4, y + 3), label, font=small, fill=(255, 255, 255, 255))
    Image.alpha_composite(base, over).convert("RGB").save(OUT / out_name)
    print(out_name, len(zones(array_name)), "zones")


guide("ui_orca_800x480.png", "orca_zones", "layout_guide_orca.png", (255, 64, 160))
guide("ui_reference_800x480.png", "fusion_zones", "layout_guide_fusion.png", (255, 64, 160))

# ---- brand-free SVG template (matches the Orca-style 6 x 4 layout) ----------
BG, PANEL, CARD, BORDER = "#010408", "#0A151B", "#0E191E", "#1D2B34"
TEXT, MUTED, ACCENT = "#F4F7FA", "#8E9BAB", "#24D4C1"
svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="800" height="480" viewBox="0 0 800 480" '
       f'font-family="Segoe UI, Inter, Arial, sans-serif">',
       '<g id="frame">',
       f'<rect width="800" height="480" fill="{BG}"/>',
       f'<rect x="4" y="4" width="792" height="74" rx="10" fill="{PANEL}" stroke="{BORDER}"/>',
       f'<rect x="20" y="16" width="50" height="50" rx="10" fill="{ACCENT}" opacity="0.85"/>',
       f'<text x="45" y="49" text-anchor="middle" font-size="24" font-weight="700" fill="{BG}">A</text>',
       f'<text x="86" y="44" font-size="28" font-weight="700" fill="{TEXT}">App Name</text>',
       f'<text x="88" y="64" font-size="11" letter-spacing="2" fill="{MUTED}">YOUR TAGLINE HERE</text>',
       f'<rect x="2" y="82" width="127" height="346" rx="8" fill="{PANEL}" stroke="{BORDER}"/>',
       f'<rect x="617" y="82" width="179" height="346" rx="8" fill="{BG}"/>',
       f'<rect x="4" y="430" width="792" height="46" rx="8" fill="{PANEL}" stroke="{BORDER}"/>',
       f'<text x="44" y="458" text-anchor="middle" font-size="13" fill="{MUTED}">Apps</text>',
       '</g>', '<g id="buttons">']


def card(x, y, w, h, title, sub="", highlight=False, rx=8):
    fill = ACCENT if highlight else CARD
    svg.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" fill="{fill}" '
               f'fill-opacity="{0.35 if highlight else 1}" stroke="{ACCENT if highlight else BORDER}"/>')
    cx = x + w / 2
    if sub:
        svg.append(f'<circle cx="{cx}" cy="{y + h * 0.36}" r="{min(w, h) * 0.16:.1f}" fill="none" '
                   f'stroke="{MUTED}" stroke-dasharray="3 3"/>')
        svg.append(f'<text x="{cx}" y="{y + h * 0.72}" text-anchor="middle" font-size="11" '
                   f'font-weight="600" fill="{TEXT}">{title}</text>')
        svg.append(f'<text x="{cx}" y="{y + h * 0.88}" text-anchor="middle" font-size="9" fill="{MUTED}">{sub}</text>')
    else:
        svg.append(f'<text x="{cx}" y="{y + h / 2 + 4}" text-anchor="middle" font-size="12" '
                   f'font-weight="600" fill="{TEXT}">{title}</text>')


for i in range(8):
    x, y, w, h = MACROS["SIDEBAR_ROW"](i)
    card(x + 1, y + 1, w - 2, h - 2, f"Page {i}" if i else "Main", highlight=(i == 0))
n = 1
for r in range(4):
    for c in range(6):
        card(*MACROS["ORCA_CELL"](c, r), f"Button {n}", "Ctrl + ?")
        n += 1
card(*CONST["RIGHT_TOP_BOX"], "Big action", rx=10)
svg.append(f'<rect x="620" y="158" width="166" height="240" rx="10" fill="{PANEL}" stroke="{BORDER}"/>')
svg.append(f'<text x="631" y="176" font-size="10" letter-spacing="1" fill="{MUTED}">PANEL</text>')
n = 1
for r in range(3):
    for c in range(3):
        card(*MACROS["ORCA_VIEW"](c, r), f"View {n}", "key")
        n += 1
for rect, _ in BOTTOM:
    x, y, w, h = rect
    card(x, y + 2, w - 2, h - 4, f"Profile {BOTTOM.index((rect, _)) + 1}")
svg.append("</g></svg>")
(OUT / "deck_template_800x480.svg").write_text("\n".join(svg), encoding="utf-8")
print("deck_template_800x480.svg written")
