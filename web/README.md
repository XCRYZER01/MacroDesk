# MacroDesk Studio

Static, dependency-free prototype of the browser profile editor. It currently
supports:

- choosing two app templates;
- editing labels, actions, shortcuts, colours and icons;
- uploading a PNG, JPG or SVG background, cropped to 800 x 480 in-browser;
- clean built-in backgrounds for PrusaSlicer, OrcaSlicer, Fusion 360, Onshape
  and Blender;
- right-sidebar layout controls for its title, 2/3 columns, 2/3 rows and optional
  quick-action button;
- eight independent decks, one per left-sidebar page, each with its own keys;
- editable left-sidebar pages, including label, icon, destination page, colour,
  enabled state and drag-to-reorder;
- dragging buttons to swap positions;
- automatic browser-local saves;
- importing and exporting versioned `.macrodesk` JSON bundles.

The preview follows the original application artwork: OrcaSlicer and PrusaSlicer
use a 6 x 4 main grid, while Fusion 360 retains its original 5 x 4 main grid and
full-height right panel. Every layout also includes eight sidebar rows and the
bottom profile bar.

Open `index.html` directly, or serve the repository root with any static web
server. Device transfer is intentionally not presented as working yet: the
firmware still needs a runtime profile loader and a serial transfer protocol.

The built-in backgrounds contain only framing, colour and header photography.
Button cards, labels, shortcuts and icons are rendered by the configurator, so
they are never duplicated when a user customizes a profile.

Choose **Right sidebar** in the button-area tabs, or click a sidebar key in the
preview, to edit that key's label, action, shortcut, icon, colour and enabled
state. Reducing the sidebar grid keeps the unused key definitions in the profile
so they return when the grid is expanded again.

Each of the eight sidebar pages owns a separate deck of keys, matching the
firmware's sidebar sub-pages. Tapping a page in the preview opens that deck, and
the numbered strip above the button list switches between them; a dimmed number
means that page has no keys in use yet. A template fills page 1 and leaves the
rest for you to build.

Choose **Left sidebar** in the button-area tabs to edit a page key's own label,
icon and destination page. Sidebar items can also be reordered from the button
list without editing source code. A key opens whichever page its destination
number points at, so the preview follows the value you type.

Version 2 bundles still import: their single deck becomes page 1 and the other
seven pages start empty.

The exported format starts with:

```json
{
  "format": "macrodesk-profile",
  "version": 3,
  "device": {
    "model": "esp32-s3-touch-lcd-7",
    "width": 800,
    "height": 480,
    "layout": "deck-template-v1"
  }
}
```

Custom icons are resized to 96 x 96 PNG data URLs in the browser. A later
device-transfer step will convert these assets to RGB565 before writing them to
the board's FAT filesystem.

## Template shortcut sources

- PrusaSlicer: [Prusa Knowledge Base keyboard shortcuts](https://help.prusa3d.com/article/keyboard-shortcuts_1764)
- Onshape: [Onshape keyboard shortcuts and hotkeys](https://cad.onshape.com/help/Content/Home/keyboard_shortcuts_and_hotkeys.htm)

Onshape tools are context-sensitive. Sketch commands require an open sketch,
while Part Studio commands such as Extrude require a compatible selection.
Onshape also lets each account customize or disable shortcuts, so the editor
keeps every mapping editable.
