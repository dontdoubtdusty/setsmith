# File map

The places where MSE Slim differs from upstream. Use `git diff upstream/master..slim --stat` for the full list.

## Tools (fork only)

| File | Purpose |
|---|---|
| `tools/check.ps1` | Build, render test and GUI smoke test |
| `tools/make-slim-data.ps1` | Copies M15, planeswalker, token and blend packages into `build/data` |
| `tools/slim-test.mse-set`, `slim-test.reference.txt` | Seven test cards and their render hashes |
| `tools/JelloBoard/`, `Jello Board.bat` | Local task board (copied from birb-box) |

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
