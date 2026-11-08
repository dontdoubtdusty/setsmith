# MSE Slim: roadmap

A personal, slimmed-down build of Magic Set Editor 2. It's based on
[haganbmj/MagicSetEditor2](https://github.com/haganbmj/MagicSetEditor2) 2.5.6 "(Unofficial)", the
community-maintained version that the M15 template pack is built for. That version is itself a fork of
[twanvl/MagicSetEditor2](https://github.com/twanvl/MagicSetEditor2). GPL v2; this fork stays GPL.

- **Branch:** `slim` (local only, not pushed yet)
- **Remotes:** `upstream` = haganbmj (base), `twanvl` = original upstream (reference only)
- **Goal:** a smaller install and a simpler UI, focused on making Magic cards in the M15 frame.
- **Related:** `custom-uvs` (the UniVersus web app) borrows feature ideas from MSE. See its `ROADMAP.md`.

## Decisions

| Date | Decision |
|---|---|
| 2026-11-08 | "Slim" means smaller template data plus a simpler UI. The rendering engine is kept as is. |
| 2026-11-08 | Toolchain: MSYS2 (UCRT64) at `D:\msys64`, with CMake, Ninja, wxWidgets 3.2, Boost and Hunspell from pacman. |
| 2026-11-08 | Local only for now. Push to GitHub once it builds. |
| 2026-11-08 | `custom-uvs` stays a web app. The slim MSE stays Magic-only. |
| 2026-11-08 | Re-based from twanvl 2.1.2 onto haganbmj 2.5.6. 2.1.2 printed `Variable not set: extra_card` on M15 cards, because the template pack targets 2.5.6. The old work is kept on branch `slim-twanvl`. |

## Phase 0: Baseline

- [x] Clone into `D:\My Documents\rpret\GitHub\MagicSetEditor2`
- [x] Create the `slim` branch (now based on haganbmj `release: 2.5.6`, `6b0d311`)
- [x] Install MSYS2 at `D:\msys64` (2026-11-08, installer 20260927). An older 2022 MSYS2 at `C:\msys64` is not used by this project.
- [x] Install the build packages: GCC 16.2, CMake 4.4, Ninja, wxWidgets 3.2, Boost, Hunspell (UCRT64)
- [x] twanvl 2.1.2 trial build: it compiled after linking `uxtheme` and rendered a test card, but printed template errors. That led to the re-base.
- [x] Build haganbmj 2.5.6 (2026-11-08). Same one fix as before: link `uxtheme` on Windows. Output: `build/magicseteditor.exe`, 8.4 MB, reports `2.5.6 (Unofficial)`.
- [x] Render a test card from the CLI (`--export-images`). It matches the installed 2.5.6 with no template errors.
- [x] Launch the GUI from Explorer (outside MSYS2). This needed two fixes:
  - The 35 MSYS2 DLLs it uses are copied next to the exe (list in `build/dll-list.txt`). This is a manual copy for now; automate it in Phase 3.
  - GCC builds load GUI images from `resource/` on disk instead of embedding them (`src/gui/util.cpp`, `load_resource_image`). `CMakeLists.txt` now copies `resource/` next to the exe after each build.

## Phase 1: Slim data (667 MB -> 180 MB) — done 2026-11-08

`build/data` is now a real folder (no longer a link to the full pack), built by
`tools/make-slim-data.ps1`. Rerun the script after a clean build:

```powershell
powershell -ExecutionPolicy Bypass -File tools\make-slim-data.ps1
```

Frames kept: **M15**, **M15 planeswalkers**, **M15 tokens**. The script follows declared dependencies and
copies 11 packages (180 MB, including the English spell-check dictionary):

`magic.mse-game`, `magic-m15`, `magic-m15-mainframe-planeswalker`, `magic-m15-mainframe-tokens`,
`magic-modules` (145 MB, the bulk), `magic-mainframe-extras`, `magic-default-image`, `magic-mana-large`,
`magic-mana-small`, `magic-pride`, `magic-blends`, plus `en.mse-locale` and `dictionaries/en_US`

- [x] Copy the needed packages into a slim `data/` folder (scripted)
- [x] Find packages that load at runtime without being declared: `magic-blends` (added).
  `magic-symbols` is referenced but missing from the full pack too, so it isn't needed.
- [x] The spell checker needs `data/dictionaries/en_US.*` (a plain folder, not a package)
- [x] Frames decided: M15 + planeswalkers + tokens
- [x] Verify: `tools/slim-test.mse-set` (creature, instant, artifact, land, planeswalker, 2 tokens) renders
  **byte-for-byte identical** to the installed 2.5.6 + full pack, with no errors
- [ ] Fonts: the `Magic - Fonts` folder from the M15 pack must be installed in Windows (already installed on
  this PC). Document this in the Phase 3 package.
- [ ] Optional later: trim unused frame images inside `magic-modules` (145 MB). Riskier, since it's shared.

## Phase 2: Simpler UI

The audit was done on twanvl 2.1.2. Check the line references again on 2.5.6.

- [ ] Remove the Keywords, Statistics, Random Pack and Console tabs (~2,000 lines; tabs are added in `src/gui/set/window.cpp`)
- [ ] Remove the package manager and update checker (~950 lines, drops the network dependency)
- [ ] Remove the symbol editor (~2,900 lines; use an image for the set symbol)
- [ ] Remove the print, HTML export and profiler windows (~530 lines, drops wxHTML)
- [ ] Optional: remove the spell checker (drops Hunspell)
- [ ] Keep Cards, Style, Set Info, the image crop tool and image export
- [ ] Rebuild and test after each removal

## Phase 3: Package

- [ ] Bundle the exe, the MSYS2 runtime DLLs and the slim data into one folder
- [ ] Remove the build tools if they are no longer needed
- [ ] Decide whether to push to GitHub (fork or new repo)
