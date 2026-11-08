# MSE Slim: roadmap

A personal, slimmed-down build of Magic Set Editor 2, forked from
[twanvl/MagicSetEditor2](https://github.com/twanvl/MagicSetEditor2) (GPL v2; this fork stays GPL).

- **Branch:** `slim` (local only, not pushed yet)
- **Goal:** a smaller install and a simpler UI, focused on making Magic cards in the M15 frame.
- **Related:** `custom-uvs` (the UniVersus web app) borrows feature ideas from MSE. See its `ROADMAP.md`.

## Decisions

| Date | Decision |
|---|---|
| 2026-11-08 | "Slim" means smaller template data plus a simpler UI. The rendering engine is kept as is. |
| 2026-11-08 | Toolchain: MSYS2 (UCRT64) at `D:\msys64`, with CMake, Ninja, wxWidgets 3.2, Boost and Hunspell from pacman. |
| 2026-11-08 | Local only for now. Push to GitHub once it builds. |
| 2026-11-08 | `custom-uvs` stays a web app. The slim MSE stays Magic-only. |

## Phase 0: Baseline

- [x] Clone upstream into `D:\My Documents\rpret\GitHub\MagicSetEditor2`
- [x] Create the `slim` branch
- [x] Install MSYS2 at `D:\msys64` (2026-11-08, installer 20260927). An older 2022 MSYS2 at `C:\msys64` is not used by this project.
- [x] Install the build packages: GCC 16.2, CMake 4.4, Ninja, wxWidgets 3.2, Boost, Hunspell (UCRT64)
- [ ] Build the unmodified source and launch it
- [ ] Render a test card from the CLI (`--export-images`) to confirm the build works

## Phase 1: Slim data (667 MB -> ~176 MB)

The M15 and planeswalker frames need only these 10 packages (from declared dependencies):

`magic.mse-game`, `magic-m15`, `magic-m15-mainframe-planeswalker`, `magic-modules`,
`magic-mainframe-extras`, `magic-default-image`, `magic-mana-large`, `magic-mana-small`,
`magic-pride`, `en.mse-locale`

- [ ] Copy those packages into a slim `data/` folder
- [ ] Check for packages that scripts load at runtime without declaring them (render cards of every type, read the warnings)
- [ ] Decide which other frames to keep (tokens? full art? DFC?)
- [ ] Document that the `Magic - Fonts` folder must be installed

## Phase 2: Simpler UI

Main window tabs are added in `src/gui/set/window.cpp` (lines 148-154).

- [ ] Remove the Keywords, Statistics, Random Pack and Console tabs (~2,000 lines)
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
