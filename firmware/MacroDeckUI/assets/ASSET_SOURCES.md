# Background image sources

The `ui_*_clean_800x480.png` files deliberately retain each app's header and
non-interactive bottom-right artwork over a neutral dark field. They contain no
cards, panels, footer buttons, interactive labels, shortcuts, icons or selection
states; those belong to the runtime UI. Run
`tools/New-CleanBackgrounds.ps1` to rebuild the final 800 x 480 files.

## PrusaSlicer

- Source: [Original Prusa MK4 media assets](https://www.prusa3d.com/page/media-assets_987/)
- Header photo: `MK4_perfect_first_layer.jpg` from `OriginalPrusaMK4.zip`;
  local copy: `source_photos/prusa_mk4_header_official.jpg`.
- Approved lower artwork: `source_photos/prusa_lower_artwork_master.png`.
- Official logo: `logo-kit/rgb/white/prusaresearch-logo-rgb-white.png` from the
  Prusa Research press kit; local reference:
  `source_photos/prusa_research_official_logo.png`
- Clean master: `source_photos/prusa_header_only_master.png`
- Changes: the rebuild script preserves the approved lower artwork and replaces
  only the header with the unmodified official Prusa Research wordmark and a
  deterministic crop of the official MK4 photograph. The center remains free
  of buttons, cards, panels and interactive labels.
- Note: Original Prusa and PrusaSlicer names and marks belong to Prusa Research.

## OrcaSlicer

- Source: the project's existing `ui_orca_800x480.png` artwork.
- Clean edit master: `source_photos/orca_original_clean_master.png`
- Changes: retained the original Orca header art, palette and bottom-right
  mascot artwork; removed all baked-in cards, panels, footer buttons, icons,
  labels and shortcuts.

## Fusion 360

- Source: the project's existing `ui_reference_800x480.png` artwork.
- Clean edit master: `source_photos/fusion_original_clean_master.png`
- Changes: retained the original Fusion header art, palette and bottom-right
  cube artwork; removed all baked-in cards, panels, footer buttons, icons,
  labels and shortcuts.
- Note: Autodesk Fusion 360 is a trademark of Autodesk, Inc.

## Onshape

- Brand reference: [PTC Onshape Brand Guide](https://www.ptc.com/en/brand-guide/logos/onshape)
- Local brand reference: `source_photos/onshape_official_logo.jpg`
- Clean edit master: `source_photos/onshape_clean_master.png`
- Changes: retained the Onshape green header and bottom-right cloud/CAD
  decoration; removed all baked-in cards, panels and footer buttons.
- Note: Onshape and the Onshape logo are trademarks of PTC Inc. and/or its
  subsidiaries.

## Blender

- Brand reference: [Blender Logo](https://www.blender.org/about/logo/)
- Local brand reference: `source_photos/blender_official_logo.png`
- Clean edit master: `source_photos/blender_clean_master.png`
- Changes: retained the Blender orange/blue header, stylized Suzanne render and
  bottom-right geometry artwork; removed all baked-in cards, panels and footer buttons.
- Note: Blender and the Blender logo are registered properties of the Blender
  Foundation; the logo is used here only to identify the Blender application.
