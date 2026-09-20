#!/usr/bin/env python3
"""Turn a .macrodesk bundle from MacroDesk Studio into macro_deck_profiles.c.

    py tools/macrodesk_to_c.py my-deck.macrodesk

writes firmware/MacroDeckUI/macro_deck_profiles.c, so the next

    arduino-cli compile --fqbn ... firmware/MacroDeckUI

builds the deck the browser designed. Flash the result with web/flash.html.

The rectangles below are the ones the artwork was drawn to; they are the same
numbers the Studio preview positions its overlay with, so what the editor shows
is what the board touches.
"""

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FIRMWARE_DIR = ROOT / "firmware" / "MacroDeckUI"
OUT_DEFAULT = FIRMWARE_DIR / "macro_deck_profiles.c"

BUNDLE_VERSION = 4
SIDEBAR_ROWS = 8
PANEL_SLOTS = 9

# A background is only usable once it exists as an RGB565 array. The legacy
# fallbacks are the arrays this repository already ships; they carry the older
# artwork with labels painted in, so they are a stopgap, not the destination.
LEGACY_IMAGES = {"orca": "ui_orca_rgb565", "fusion": "ui_reference_rgb565"}

# Studio draws with glyphs the board's Montserrat fonts do not carry, so those
# become LVGL symbols. Plain ASCII is left alone: the board renders "S" as the
# letter S, exactly as the preview does, and guessing that it meant "save" would
# turn a sidebar initial into a floppy disk.
ICON_MAP = {
    "↶": "LV_SYMBOL_LEFT", "↷": "LV_SYMBOL_RIGHT",
    "＋": "LV_SYMBOL_PLUS", "−": "LV_SYMBOL_MINUS",
    "⌂": "LV_SYMBOL_HOME", "◫": "LV_SYMBOL_COPY", "✂": "LV_SYMBOL_CUT",
    "◉": "LV_SYMBOL_EYE_OPEN", "⚙": "LV_SYMBOL_SETTINGS", "▶": "LV_SYMBOL_PLAY",
    "⌕": "LV_SYMBOL_LIST", "↔": '"<->"', "◇": '"[]"', "⌒": '"R"',
}


def fail(message):
    sys.exit(f"macrodesk_to_c: {message}")


def c_string(value):
    escaped = value.replace("\\", "\\\\").replace('"', '\\"')
    return f'"{escaped}"'


def icon_literal(button):
    icon = (button.get("icon") or "").strip()
    if icon in ICON_MAP:
        mapped = ICON_MAP[icon]
        return mapped if mapped.startswith("LV_SYMBOL") else mapped
    if icon and all(ord(c) < 128 for c in icon):
        return c_string(icon[:4])
    label = (button.get("label") or "").strip()
    return c_string(label[:1].upper() or "?")


def colour_literal(value):
    return f"0x{value.lstrip('#').upper()}"


def darker(value, factor=0.62):
    raw = int(value.lstrip("#"), 16)
    parts = [(raw >> 16) & 0xFF, (raw >> 8) & 0xFF, raw & 0xFF]
    return "0x%02X%02X%02X" % tuple(int(p * factor) for p in parts)


def button_body(button, warnings, where, with_accent):
    """Render the macro_button_t initialiser fields after the icon."""
    label = c_string(button.get("label") or "")
    action = button.get("action", "keys")
    value = button.get("value") or ""
    accent = colour_literal(button.get("color", "#FFFFFF")) if with_accent else "0"

    if not button.get("enabled", False) or action == "none":
        return f"{label}, NULL, {accent}, MACRO_ACTION_NONE"
    if action == "page":
        try:
            target = int(value)
        except ValueError:
            target = 1
        target = min(8, max(1, target)) - 1
        return f"{label}, NULL, {accent}, MACRO_ACTION_PAGE, {target}"
    if action == "search":
        return f"{label}, {c_string('search:' + value)}, {accent}"
    if action == "text":
        warnings.append(f"{where}: action \"text\" has no firmware equivalent; sent as no action")
        return f"{label}, NULL, {accent}, MACRO_ACTION_NONE"
    if not value:
        warnings.append(f"{where}: no shortcut set; sent as no action")
        return f"{label}, NULL, {accent}, MACRO_ACTION_NONE"
    return f"{label}, {c_string(value)}, {accent}"


def zone(rect, button, warnings, where):
    return f"    {{{rect}, {{{icon_literal(button)}, {button_body(button, warnings, where, False)}}}}},"


def image_symbol(profile, warnings):
    background = profile.get("backgroundImage")
    if not isinstance(background, str) or background.startswith("data:"):
        fail(f"profile \"{profile['id']}\" uses an uploaded background. Convert it first:\n"
             f"       py tools/png_to_rgb565.py <your 800x480 png>\n"
             f"       then point the profile at a built-in background, or edit the symbol by hand.")
    stem = Path(background).stem                      # ui_orca_clean_800x480
    base = stem.rsplit("_", 1)[0] if stem.endswith("800x480") else stem
    symbol = f"{base}_rgb565"
    if (FIRMWARE_DIR / f"{symbol}.c").exists():
        return symbol
    legacy = LEGACY_IMAGES.get(profile["id"])
    if legacy and (FIRMWARE_DIR / f"{legacy}.c").exists():
        warnings.append(
            f"profile \"{profile['id']}\": {symbol}.c does not exist, falling back to {legacy}. "
            f"That is the older artwork with labels painted in. Generate the clean one with:\n"
            f"       py tools/png_to_rgb565.py firmware/MacroDeckUI/assets/{Path(background).name}")
        return legacy
    fail(f"profile \"{profile['id']}\": no image array for {background}.\n"
         f"       py tools/png_to_rgb565.py firmware/MacroDeckUI/assets/{Path(background).name}")


def used_keys(keys):
    """Keys up to the last one in use -- trailing blanks are not worth flashing."""
    last = -1
    for index, button in enumerate(keys):
        if button.get("enabled") and button.get("action") != "none":
            last = index
    return keys[:last + 1]


def render_zones(profile, index, warnings):
    name = f"{profile['id']}_zones"
    main = profile["pageButtons"][0]
    columns = 5 if len(main) == 20 else 6
    quick = profile.get("showQuickAction", True)
    lines = [f"static const macro_zone_t {name}[] = {{"]

    for row, button in enumerate(profile["sidebarButtons"][:SIDEBAR_ROWS]):
        rect = f"4, {84 + row * 42}, 123, 41"
        lines.append(zone(rect, button, warnings, f"{profile['id']} sidebar {row + 1}"))
    lines.append("")

    left, pitch, width = (133, 97, 93) if columns == 5 else (134, 80, 76)
    for slot, button in enumerate(main):
        col, row = slot % columns, slot // columns
        rect = f"{left + col * pitch}, {88 + row * 85}, {width}, 82"
        lines.append(zone(rect, button, warnings, f"{profile['id']} key {slot + 1}"))
    lines.append("")

    if quick:
        lines.append(zone("620, 88, 168, 63", profile["quickAction"], warnings, f"{profile['id']} quick action"))
    top, height = (186, 64) if quick else (128, 84)
    pitch_y = 70 if quick else 90
    for slot, button in enumerate(profile["panelButtons"][:PANEL_SLOTS]):
        col, row = slot % 3, slot // 3
        rect = f"{626 + col * 54}, {top + row * pitch_y}, 52, {height}"
        lines.append(zone(rect, button, warnings, f"{profile['id']} panel {slot + 1}"))
    lines.append("")
    lines.append("    BOTTOM_BAR,")
    lines.append("};")
    return name, "\n".join(lines)


def render_pages(profile, warnings):
    blocks, entries = [], ["    {NULL, NULL, NULL, 0, 0},"]
    for index, page in enumerate(profile["pages"][1:], start=1):
        keys = used_keys(profile["pageButtons"][index])
        name = f"{profile['id']}_page{index}"
        if not keys:
            entries.append(f"    {{{c_string(page['title'])}, {c_string(page['hint'])}, NULL, 0, 0}},")
            continue
        rows = [f"static const macro_button_t {name}[] = {{"]
        for slot, button in enumerate(keys):
            where = f"{profile['id']} page {index + 1} key {slot + 1}"
            rows.append(f"    {{{icon_literal(button)}, {button_body(button, warnings, where, True)}}},")
        rows.append("};")
        blocks.append("\n".join(rows))
        entries.append(f"    {{{c_string(page['title'])}, {c_string(page['hint'])}, "
                       f"ITEMS({name}), {page['columns']}}},")
    array = f"static const macro_page_t {profile['id']}_pages[] = {{\n" + "\n".join(entries) + "\n};"
    return "\n\n".join(blocks + [array])


def generate(bundle, source_name):
    warnings = []
    profiles = bundle["profiles"]
    ids = [p["id"] for p in profiles]
    if len(set(ids)) != len(ids):
        fail("both profiles use the same template; give them different apps so their C symbols differ")

    out = [
        "/*",
        f" * Generated by tools/macrodesk_to_c.py from {source_name}.",
        " * Edit the deck in web/index.html and run the tool again; hand edits are lost.",
        " */",
        '#include "macro_deck_ui.h"',
        "",
    ]
    for profile in profiles:
        out.append(f"extern const uint16_t {image_symbol(profile, warnings)}[];")
    out += [
        "",
        f"#define PROFILE_{ids[0].upper()} 0",
        f"#define PROFILE_{ids[1].upper()} 1",
        "",
        "#define BOTTOM_BAR \\",
        f"    {{78, 432, 128, 40, {{{c_string(profiles[0]['badge'])}, {c_string(profiles[0]['name'])}, "
        f"NULL, 0, MACRO_ACTION_PROFILE, PROFILE_{ids[0].upper()}}}}}, \\",
        f"    {{208, 432, 128, 40, {{{c_string(profiles[1]['badge'])}, {c_string(profiles[1]['name'])}, "
        f"NULL, 0, MACRO_ACTION_PROFILE, PROFILE_{ids[1].upper()}}}}}, \\",
        '    {337, 432, 118, 40, {LV_SYMBOL_SETTINGS, "System", NULL, 0, MACRO_ACTION_NONE}}',
        "",
        "#define ITEMS(a) (a), (uint8_t)(sizeof(a) / sizeof((a)[0]))",
        "#define COUNT(a) (sizeof(a) / sizeof((a)[0]))",
        "",
    ]

    zone_names = []
    for index, profile in enumerate(profiles):
        name, block = render_zones(profile, index, warnings)
        zone_names.append(name)
        out += ["/* " + "=" * 70 + " */", f"/*  {profile['name']}", " */", block, "", render_pages(profile, warnings), ""]

    out.append("const macro_profile_t macro_profiles[] = {")
    for index, profile in enumerate(profiles):
        out.append(
            f"    [PROFILE_{ids[index].upper()}] = {{{c_string(profile['name'])}, "
            f"{image_symbol(profile, [])}, {zone_names[index]}, COUNT({zone_names[index]}),\n"
            f"                     {profile['id']}_pages, COUNT({profile['id']}_pages),\n"
            f"                     {darker(profile['accent'])}, {colour_literal(profile['accent'])}}},")
    out += ["};", "const size_t macro_profile_count = COUNT(macro_profiles);", ""]
    return "\n".join(out), warnings


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("bundle", type=Path, help=".macrodesk file exported from the Studio")
    parser.add_argument("--out", type=Path, default=OUT_DEFAULT, help="output .c file")
    args = parser.parse_args()

    if not args.bundle.exists():
        fail(f"{args.bundle} does not exist")
    try:
        bundle = json.loads(args.bundle.read_text(encoding="utf-8"))
    except json.JSONDecodeError as error:
        fail(f"{args.bundle} is not valid JSON: {error}")

    if bundle.get("format") != "macrodesk-profile":
        fail(f"{args.bundle} is not a MacroDesk bundle")
    if bundle.get("version") != BUNDLE_VERSION:
        fail(f"{args.bundle} is version {bundle.get('version')}; this tool reads version {BUNDLE_VERSION}. "
             f"Open it in the Studio and export it again.")
    if len(bundle.get("profiles", [])) != 2:
        fail("a bundle must hold exactly two profiles")
    for profile in bundle["profiles"]:
        if profile.get("panelColumns") != 3 or profile.get("panelRows") != 3:
            fail(f"profile \"{profile['id']}\" uses a {profile.get('panelColumns')}x{profile.get('panelRows')} "
                 f"right sidebar. The artwork only has rectangles for 3x3.")

    source, warnings = generate(bundle, args.bundle.name)
    args.out.write_text(source, encoding="utf-8", newline="\n")

    for warning in warnings:
        print(f"  warning: {warning}")
    keys = sum(len(used_keys(page)) for profile in bundle["profiles"] for page in profile["pageButtons"][1:])
    print(f"{args.bundle} -> {args.out}")
    print(f"  {len(bundle['profiles'])} profiles, {keys} keys on sidebar pages, {len(warnings)} warnings")
    empty = sum(1 for profile in bundle["profiles"] for page in profile["pageButtons"][1:] if not used_keys(page))
    if empty:
        print(f"  note: {empty} sidebar pages have no keys. The hand-written profiles this")
        print("        replaces fill seven pages per profile, so flashing this is a downgrade")
        print("        unless you filled them in the Studio first.")
    print("  build:  arduino-cli compile --fqbn <fqbn> firmware/MacroDeckUI")
    print("  flash:  web/flash.html")


if __name__ == "__main__":
    main()
