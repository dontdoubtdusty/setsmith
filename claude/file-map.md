# File map

The places where Setsmith differs from upstream. Use `git diff upstream/master..main --stat` for the full list.

## Tools (fork only)

| File | Purpose |
|---|---|
| `tools/check.ps1` | Build, render test and GUI smoke test |
| `tools/make-slim-data.ps1` | Copies M15, planeswalker, token and blend packages into `build/data` |
| `tools/slim-test.mse-set`, `slim-test.reference.txt` | Seven test cards and their render hashes |
| `tools/frames-test.mse-set`, `frames-test.reference.txt` | Saga, DFC, adventure, borderless and etched foil test cards and their render hashes |
| `tools/JelloBoard/`, `Jello Board.bat` | Local task board (copied from birb-box) |
| `tools/make-icon.ps1` | Generates the Setsmith app icon (`resource/icon/app.ico`) |
| `tools/uvs-test.mse-set`, `uvs-test.reference.txt` | UniVersus test cards (generated art) and their render hashes |
| `tools/make-uvs-assets.ps1` | Draws all UVS frame PNGs (boxes, numbers, shields, labels, icons, Character theme panels) into `templates/` |
| `tools/extract-uvs-symbols.ps1` | Cuts the 13 resource symbols (circle + attuned) from the How to Play PDF into `sym-*.png` |
| `tools/install-uvs-fonts.ps1` | Installs `templates/fonts` for the current Windows user (no admin) |
| `tools/card.ps1`, `.claude/skills/card/SKILL.md` | `/card` skill: adds or replaces one card in `Documents\Setsmith Cards\Claude Magic/UVS.mse-set` and renders its PNG (uses `setsmith.com --export-images --zoom`) |

## UniVersus template (fork only, spec: `claude/uvs-template.md`)

| File | Purpose |
|---|---|
| `templates/uvs.mse-game/` | Card and set fields, rules-text filters (labels, `{C}`, reminder text) |
| `templates/uvs-2026-common.mse-include/` | Shared layout (`card-style`), scripts, generated PNGs |
| `templates/uvs-2026.mse-style/`, `uvs-2026-exact.mse-style/`, `uvs-2026-mpc.mse-style/` | Bleed (819 x 1114), exact (744 x 1039) and MPC (822 x 1122) stylesheets; headers only (size, `bleed_x`, `bleed_y`) |
| `templates/uvs-symbols.mse-symbol-font/` | Inline label boxes and the commit icon |
| `templates/fonts/` | Figtree and Barlow (OFL) |

## Engine additions for UVS

| File | Purpose |
|---|---|
| `src/data/font.*`, `src/util/rotation.cpp`, `src/gfx/resample_text.cpp` | Text outlines: `stroke color/width`, `stroke2 color/width` (alpha dilation) |
| `src/render/text/viewer.*` | Masked text that is centered or bottom aligned is fitted against the right mask rows; the mask is checked over each line's full height |

## Branding (Setsmith)

| File | Purpose |
|---|---|
| `resource/about.png`, `resource/icon/app.ico` | Logo (Welcome and About windows) and app icon |
| `data/en.mse-locale/locale` | Titles and menu text |
| `src/main.cpp` | App name: settings folder and CLI banner |
| `src/config.hpp.in` | Version from `PROJECT_VERSION_*` |

## Set symbol upload (replaces the symbol editor)

| File | Purpose |
|---|---|
| `src/gui/symbol_upload_window.*` | Upload dialog: rarity preview, best practices, warnings |
| `src/gui/value/symbol.*` | Set symbol field: Upload button and double click |
| `src/render/symbol/filter.*` | `render_image_symbol`: shape mask, distance border, rarity filter |
| `src/gfx/generated_image.cpp` | `SymbolToImage` renders `.png` symbols for templates |
| `src/render/value/symbol.cpp` | Set Info preview of image symbols |

## Messages (replaces the Console panel)

| File | Purpose |
|---|---|
| `src/gui/util.*` | `show_queued_messages(parent)`: deduplicated popup |
| `src/gui/set/window.cpp`, `src/gui/welcome_window.cpp` | Call it on idle |

## Removed

| Removed | Notes |
|---|---|
| Tabs | Keywords, Statistics, Random Pack, Console |
| Package manager and update checker | Also the networking code |
| HTML export | `export_set()` moved to `src/data/export_template.cpp` |
| Profiler | |
| Symbol editor | `src/gui/symbol/` |

## New look (Phase 4, fork only)

| File | Purpose |
|---|---|
| `src/gui/theme.*` | Dark palette, `theme::colour` (replaces `wxSystemSettings::GetColour` everywhere), UI font, `apply()` that colors a window tree. `main.cpp` calls `theme::init()` at start and `apply()` when a top level window is shown |
| `resource/fonts/` | Figtree (OFL), loaded at start for the UI font |
| `tools/make-icon.ps1` | Draws `resource/icon/app.ico` (exe and window) and `set.ico` (set files) |

## Removed on 2026-11-09 (git history has them)

Update checker and installer (`data/installer.*`, the installed-package database in `util/io/package_manager.*`), `gui/control/graph.*`, `util/vcs*`, export and import formats other than `.mse-set` and images (`data/format/mse1`, `mtg_editor`, `apprentice`, `mws`, `html`), the Directories preferences page, 12 non-English locales, `package.ps1`, `test/`, `CMakeSettings.json`, the macOS bundle files, and the symbol and installer file icons. `CHANGES.md` is now `UPSTREAM-CHANGES.md`.

## Magic data patches

`tools/make-slim-data.ps1` patches the copied M15 pack after copying (no Masterpiece rarity, set code, set symbol or style option) and adds a P/T corner mask to the M15 and etched-foil text boxes. The mask image lives in `tools/patches/`. Each patch must match an exact number of times, or the script stops.
