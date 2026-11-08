# Setsmith (MSE Slim): working notes for Claude

Setsmith is a personal, slimmer fork of Magic Set Editor 2. The exe is `build/setsmith.exe`, and settings live in `%APPDATA%\Setsmith`. It is based on haganbmj/MagicSetEditor2 2.5.6 (Unofficial) and lives on branch `slim`.
- `ROADMAP.md` is the plan and the decisions record (with the why behind each decision). Read it first.
- `claude/` holds the working docs: the file map, session log, QA plan and the task board.

## Commands

Build and test. Run from the repo root in PowerShell.

| What | Command |
|---|---|
| Build + render test + GUI smoke test | `powershell -NoProfile -ExecutionPolicy Bypass -File tools\check.ps1` |
| Rebuild the slim template data | `powershell -NoProfile -ExecutionPolicy Bypass -File tools\make-slim-data.ps1` |
| Task board (http://localhost:5179) | `Jello Board.bat` (needs the .NET 10 SDK) |

- **Toolchain:** MSYS2 UCRT64 at `D:\msys64` (GCC, CMake, Ninja, wxWidgets 3.2, Boost.Regex, Hunspell). `check.ps1` drives it through `D:\msys64\usr\bin\bash.exe`, so you don't need to open an MSYS2 shell.
- **Reconfigure after adding or removing sources.** CMake finds sources with `file(GLOB)`, and `check.ps1` reconfigures every run.
- **Running the exe outside MSYS2:** it needs the 18 MSYS2 DLLs next to it in `build/`, plus `build/resource/` (the post-build step copies it) and `build/data/` (from `make-slim-data.ps1`).
- **Render test:** `tools/slim-test.mse-set` is compared against the hashes in `tools/slim-test.reference.txt`. If you change rendering on purpose, regenerate the reference and say so in the commit.

## Architecture

| Folder | Contents |
|---|---|
| `src/data/` | Sets, cards, games, styles, fields and actions (undoable edits) |
| `src/script/` | The template scripting language |
| `src/render/` | Drawing card values. `render/symbol/filter.*` holds the rarity coloring, including uploaded image symbols |
| `src/gfx/` | Image generation used by templates (`generated_image.cpp`, `SymbolToImage`) |
| `src/gui/` | wxWidgets UI: `set/` holds the tabs (Cards, Style, Set info) and `value/` the field editors |
| `src/cli/` | `setsmith.com` / `setsmith-cli`, used by the render test |

## Working rules

- No emojis in code, docs or UI text.
- Ask multiple-choice questions with at most 4 options, the recommended one first. Push back on ideas that hurt simplicity or speed.
- Build features fully, then verify them with `check.ps1`. For UI changes, also check the real GUI.
- Keep internal MSE identifiers (`.mse-*` extensions, `mse_version`, the `magicseteditor.exe` package ID). Templates depend on them.
- Write plain comments. Mark fork changes with `MSE Slim:` so upstream merges stay readable.
- Reuse existing code (`filter_symbol`, `value_action`, `show_queued_messages`) before writing new code.
- **Commits:** Claude commits on `slim` only when asked. Claude never pushes; the fork is local only.
- **Decisions:** record each decision in the decisions table in `ROADMAP.md`, with the why.
- **After changes:** give a short summary of what changed and what was verified.
- **Wrap-up at the end of a session:**
  - Add a dated entry to `claude/session-log.md`.
  - Update the board cards and `claude/file-map.md` if files moved.
  - Update `ROADMAP.md`.
- **"Instructions:" convention:** a user message starting with "Instructions:" is a standing rule. Add it to this file.

## Related

`custom-uvs` (separate repo) is the UniVersus web app that borrows ideas from this project. Never push its main without asking.
