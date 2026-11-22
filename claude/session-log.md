# Session log

Newest first. Keep each entry short: what changed, what was verified, what's next.

## 2026-11-22 (editor restyle, speed and dead-code pass)

- **Editor restyle (f1039e15):** editor and dashboard open full screen by default (F11, header buttons or double-clicking the header for a window; `set_window_fullscreen`). Header band (`gui/set/header_bar.*`) replaces the native toolbar, tab bar and status bar; menu button in full screen. Line icons drawn in code (`gui/icons.*`), blue-black palette, gold list selection. Added `resource/setsmith.manifest`: without it there were no visual styles, so the dark themes never applied.
- **Speed (d168a837):** directory-package files were read one byte per system call (unbuffered `wxFileInputStream` in `Package::openIn`). Now buffered, and whole-file script includes read in 64 KB blocks: set open and CLI export about 1.3 s faster. Not changed: PNG level 3 (14% faster save, 6% bigger files).
- **Dead code (d168a837):** keyword list, tree list, symbol editor actions, image-to-symbol import, 73 images, 95 window ids removed; undo/redo tooltips set only on change.
- **White flash (c956f6fc):** windows are themed and full screen before the first show; editor frame erases dark; editor opens on the dashboard's monitor. Owner to confirm.
- Verified: check.ps1 passes after each step, renders byte-identical.
- Next: editor polish (search-box border, column header text, list and notes borders, notes panel), then the MTGA collection-style card grid.

## 2026-11-22 (MTGA-style dashboard)

- **Dashboard:** the welcome window is rebuilt as an MTGA-style home screen (set tiles with art, New set tile, game filters, Folders and Open buttons, gold Continue button, hover lift and glow, picture fade-in, smooth scroll). New `src/gui/set_library.*` reads sets cheaply. New settings key `watched_folder`, new set key `tile card` (written only when picked).
- **Dark title bars** for every window (DWM attribute 20, falling back to 19).
- **CLI no longer touches recent sets** (`settings.remember_recent_sets`); the two mtg-reskin work folders were removed from the owner's recent list, and `mtg-reskin\build` was added as a watched folder.
- Verified: check.ps1 passes (no render changes), dashboard screenshotted with hover on a tile and on Continue.
- Next: Stage 5, the editor restyle to match.

## 2026-11-22 (printed text size, margins and P/T shape, for mtg-reskin)

- **Text:** M15 and M15 Altered rules text largest size 15.45 (was 14), measured line heights, padding left 4, holo-stamp margin 2 (was 12). Name and type fonts `scale down to: 6`. Picard renders within 1 px of the scan.
- **Station:** badge and row sizes re-measured; station cards drop the P/T text mask; P/T box redrawn (straight edges, curved rounded ends), numerals centred.
- **Verified:** `check.ps1` passes; slim-test and frames-test hashes regenerated on purpose (8 cards). Committed e7dad5e2, pushed.
- **Next:** a full-width last flavor line can touch the holo-stamp arch; the mask only trims line ends.

## 2026-11-21 (alias line, station rows, for mtg-reskin)

- **Alias line:** a filled-in `alias` prints the real Oracle name small under the title, on every Magic frame (patch in `make-slim-data.ps1`). Frames with their own alias box (Altered, adventure, DFC, planeswalker, tokens) have it replaced: the engine keeps the first box with a name.
- **Station rows:** new `templates/setsmith-station.mse-include` (fields, layout script, bands, badges; PNGs from `tools/make-station-assets.ps1`), hooked into M15 and M15 Altered. Up to two rows; the text box shortens to fit; row font follows the card's total text. `/card` skill documents `alias` and the station fields.
- **Render test:** `frames-test.mse-set` gained Rustbucket Skiff (two rows, alias) and Gilded Orrery (gold, one row, no P/T). Ashwood Pilgrim's hash regenerated on purpose: only its collector line changed (the set has two more cards).
- **Verified:** `check.ps1` passes; every existing render unchanged before the new cards were added. Real Star Trek cards rendered and looked at: Oberth-Class Starship, D-7 Battle Cruiser (two rows), Borg Cube (gold), Deep Space 9 (no P/T), a saga and a token with aliases. One render takes about 2.7 s through the CLI, mostly startup.
- **Next:** the mtg-reskin viewer renders through `card.ps1` or the CLI. Very wordy station cards squeeze the main text; tune `station_size` if real cards look off.

## 2026-11-12 (/card skill, more Magic frames, Character art fix)

- **/card skill:** `.claude/skills/card/SKILL.md` + `tools/card.ps1`. Claude designs a Magic or UVS card from an idea (fields the user gives are kept), adds or replaces it by name in `Documents\Setsmith Cards\Claude Magic.mse-set` / `Claude UVS.mse-set` (with `.bak`), and renders a PNG next to it. Optional art is center-cropped to the art box; `-Art2` for a DFC back.
- **Engine:** `--export-images ... --zoom N` (Magic exports at 750 px); saved settings untouched.
- **UVS:** empty art exports as a dark backdrop (`art-blank.png`), not the checkerboard. Character art could not be loaded: the card type field covered the whole card above the art. Shading is now the non-editable `character shade` field; on Characters the card type is clicked in the footer.
- **Magic frames added:** saga, DFC, adventure, borderless art (`m15-altered` frame option; the pack has no true extended art), etched foil. New render test `tools/frames-test.mse-set`. `make-slim-data.ps1` now reads dependencies from the right descriptor file.
- **Verified:** `check.ps1` passes (slim-test and uvs-test unchanged); each new frame looked at with art.
- **Next:** click-test loading art on a UVS Character in the GUI. Cloud `/card` was discussed and put on hold (board idea).

## 2026-11-08 (UniVersus: all main types, Bonus Stage set)

- **Types:** Action, Asset, Backup (stamina heart), Attack (speed circle + damage burst), Character (art to the footer, shaded; name/traits/text anchored above the footer using a length estimate; hand size + health). One shared layout; positions are per-type scripts.
- **Also:** watermark option, inline resource symbols (`{good}`), symbol slots 5-6, numbers typed with their own total, reminder text only at a paragraph start, keyword line in caps.
- **Gotchas:** `set_alpha` takes `input`, not `image` (a named `image:` is ignored and the field value is loaded as a file). GDI+ gradient fills draw a 1 px line at the brush's start point.
- **Bonus Stage:** 34 cards rebuilt from the user's fan sheet (`bonusstageuvs.png`), art cropped from it; set and 34 print PNGs (819 x 1114) in `D:\My Documents\rpret\GitHub\bonus-stage\`. Old baked-in names still show slightly at the left of some non-character names; clean art would fix it.
- **Verified:** `check.ps1` passes; UVS test set now has a Character, Backup and Attack (hashes regenerated; the old Plain and Bleed cards are unchanged).
- **Full-art Characters:** art to the card edge and bleed; Set info `character theme` (standard fade, or Bonus Stage: salmon name slab + angular ink panel). Bonus Stage set rebuilt with it: Rya and Julie from the user's clean art, the others zoomed from the clean top of the sheet (zoom picked per card so the art reaches the panel). Prints re-exported. Test Character hash updated on purpose; `check.ps1` passes.
- **Characters, second pass (after the user's official refs):** dropped the black ink panel. The text now has an outline and a drop shadow on the art (a light scrim), with a torn salmon brush behind Bonus Stage names, and no watermark on Characters. For Bonus Stage, the sheet art fades into a salmon halftone background, baked in by the set builder. The builder also paints out the sheet's baked-in difficulty numbers, shields and names, which had shown doubled under the new ones. The B logo's ragged edge was smoothed. `check.ps1` passes (only the test Character changed).
- **Cutouts (third pass):** new optional, non-editable `cutout` image field on cards, drawn above the Character shading so the body stays bright. **Engine:** non-editable image fields no longer draw the checkerboard placeholder when empty (`src/render/value/image.cpp`); Magic renders unchanged. Bonus Stage (outside the repo):
  - Cutouts of the user's HD stills (`bonus-stage\refs`) in `bonus-stage\cutouts`, made with BiRefNet plus an outline-based flood fill. The ML mask alone loses white shirts on white walls.
  - LaMa inpainting replaces the smooth fill for the old numbers and names.
  - Alternate old-style Craig, a "SO?!" Foundation, and a FCC-notice joke card (`print\000 FCC NOTICE.png`).
- **Gotcha:** `check.ps1` does not refresh `build\data`. Run `make-slim-data.ps1` after template changes, or the checks test stale templates.
- **Next:** Token, Arena, holo masks, clean art from the user.

## 2026-11-08 (UniVersus footer symbols)

- **Symbols:** the 13 official resource symbols, cut from the How to Play guide's vector page at 1200 dpi (`tools/extract-uvs-symbols.ps1`). Air and Water attuned come from the guide; the other 11 attuned are built from the circle art.
- **Footer:** ring, divider bar and symbol positions re-measured on FT01 2x scans.
- **Fix:** choosing a symbol no longer shows "File not found: sym-*.png".
- **Verified:** `check.ps1` passes. The UVS test set now has symbols, and its reference hashes were regenerated on purpose. Footer checked side by side with FT01-032.
- **IP logo:** the Set info logo is drawn in the footer ring (`set_mask` with a ring-shaped mask; no engine change). The test set has a neutral diamond logo; an empty logo renders black with no errors. Made a Bonus Stage logo (the "B" from its wordmark, white) outside the repo.
- **Crash fix:** loading an image into the Set info ip logo crashed (the field had no size, and the crop dialog divides by it). The UVS style now gives it a 256 x 256 `set info style`, and `src/gui/value/image.cpp` falls back to the image's own size for any sizeless image field. Not yet clicked through in the GUI.
- **Next:** Action / Asset / Backup.

## 2026-11-08 (UniVersus template, Foundation)

- **Spec:** `claude/uvs-template.md`, from the uvsrefs.txt cards, official PNGs and the How to Play PDF.
- **Engine:**
  - Text outlines (`stroke`/`stroke2` font options).
  - Masked + centered text now wraps around the mask correctly.
  - M15 renders are byte-identical.
- **Template:** a UVS game package, a shared 2026 layout, bleed and exact stylesheets, a label symbol font, and generated frame art. The Foundation frame is done.
- **Fonts:** the How to Play PDF only contains its layout fonts. Using closest free matches: Barlow Condensed (rules), Figtree (name, numbers, footer).
- **Verified:**
  - `check.ps1` passes, now including `uvs-test` (3 cards).
  - The GUI opens the UVS set with no errors.
  - Side-by-side checks against TK802-140 and FT01-032.
- **Next:**
  - Footer resource symbols at print resolution: harvest from 2x scans, or trace.
  - IP logo from the set field.
  - Then Action / Asset / Backup.

## 2026-11-08

- **Phase 2 removals:** removed the Keywords, Statistics and Random Pack tabs, the package manager, the update checker, HTML export, the profiler and the symbol editor. Replaced the Console with popups.
- **Set symbol upload (`0d366599`):**
  - Added an upload dialog with a four-rarity preview and best-practice tips.
  - PNG symbols render on cards through the normal rarity filters.
- **Verified:**
  - `check.ps1` passes.
  - Rendered cards show the image symbol in all rarities.
  - The dialog preview works.
- **Not yet verified:** the final "Use this symbol" click, because the desktop was locked.
- **Framework:** copied from birb-box: CLAUDE.md, `claude/` docs and the Jello Board.
- **Rename to Setsmith:**
  - Exe, titles, About box, logo and settings folder are renamed. `check.ps1` passes.
  - Rendering is unchanged and the version still reports 2.5.6.
  - New gold diamond app icon. Verified in the title bar and in Explorer's exe icon.
- **Next:**
  - Hand-check the upload.
  - Phase 3: packaging.

## 2026-11-09

- **UVS print sizes:** new `2026 (MPC)` stylesheet, 822 x 1122 px at 300 dpi (MakePlayingCards poker template), next to `2026 (bleed)` and `2026 (exact)`. `bleed` is split into `bleed_x` and `bleed_y` in the shared layout. The bonus-stage cards also render correctly with it.
- **New look, stage 1 (dark theme):** `src/gui/theme.*` holds the palette (charcoal, logo gold) and `apply()`; every `wxSystemSettings::GetColour` call goes through `theme::colour`. Figtree (from `resource/fonts`) is the UI font. Windows dark scrollbars and list headers are asked for with `SetWindowTheme`. Title bar and menu bar stay native.
- **Divorced from MSE2 (credit kept):** README, bundled README, About window, locale and CLI strings now say Setsmith and credit Magic Set Editor 2 and its authors. The inherited changelog is `UPSTREAM-CHANGES.md`. Kept on purpose, because templates and old sets use them: `.mse-*` extensions, package names, the `magicseteditor.exe` dependency name, `installer group` and `apprentice_code` keys. The settings file is now `setsmith.config`, so settings reset once.
- **New icons:** `tools/make-icon.ps1` now draws `app.ico` (card with a gold diamond) and `set.ico` (stacked cards). Exe icon verified after touching `win32_res.rc`.
- **Features stripped (approved audit):** update checker and installer (with the installed-package database in `package_manager.*`), statistics graph control, version control support (old sets' `version_control` key is read and ignored), Magic Workstation / Apprentice / MTG Editor / MSE1 / HTML formats, the Directories preferences page, the Help menu's Index and Website items, 12 non-English locales and the language picker, macOS and Visual Studio files, `package.ps1`, `test/`, and the symbol and installer file icons. Printing and the spell checker stay.
- **Masterpiece removed from the Magic data:** no rarity choice, set code, set symbol or style option. Done as patches in `tools/make-slim-data.ps1` because the pack is not in this repo (each patch must match exactly, or the script stops).
- **Verified:** `check.ps1` passes after each stage, Magic and UVS renders are byte-identical, the real bonus-stage set (with old keys) loads with zero messages, About window and main window checked in screenshots.
- **Next:** stage 2 of the new look (Cards tab layout, row color stripes, backdrop), then icons for the toolbar, then the Welcome window.

## 2026-11-12

- **Text clear of the P/T box (permanent fix):** the text engine checks the mask over each line's full height, and M15 and etched foil get a P/T corner mask when the card has a P/T. Planeswalker and UVS wrapping improved too (two reference hashes regenerated). Verified with check.ps1 and a long-flavor Jensen render.
- **P/T fix on every frame:** borderless, adventure and DFC (both faces, plus the hovering back-face P/T) now use one card-sized mask cropped per text box. Engine: mask scripts run after the box size is final, and masks no longer lose 1 px on the right. All checks pass with no reference changes.
- **UVS Jensen card:** made as a Character in Claude UVS.mse-set.
