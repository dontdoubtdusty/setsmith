# Setsmith

Setsmith is a card editor for custom trading cards. It makes cards in two frames:
- **UniVersus**: the 2026 frame, in three print sizes (with bleed, exact size, and MakePlayingCards).
- **Magic**: the M15 frame.

You work in three tabs (Cards, Style, Set info). Set symbols are uploaded as images, and cards export to PNG at print size.

Plans and decisions are in `ROADMAP.md`. How to build and test is in `CLAUDE.md`.

## Built on Magic Set Editor 2

Setsmith started as a fork of Magic Set Editor 2, and most of its engine (card rendering, the template language, the file formats) is still theirs. It builds on the unofficial 2.5.6 release by [haganbmj](https://github.com/haganbmj/MagicSetEditor2), which in turn builds on the work of Twan van Laarhoven, Sean Hunt (coppro), Alissa Rao (Lymia), CaiCai and the other Magic Set Editor developers.

Setsmith keeps Magic Set Editor's file extensions and package names (`.mse-set`, `.mse-style` and so on), so existing sets and templates still open. Its template scripting language is the one documented in `doc/`, which is the Magic Set Editor technical documentation. The changelog of the original project is kept as `UPSTREAM-CHANGES.md`.

## License

Free software under the GNU General Public License, version 2 or later (see `COPYING`), like the program it is built on.

## Building (Windows)

Setsmith builds with MSYS2 (UCRT64) and needs wxWidgets 3.2, Boost.Regex and Hunspell. The one command that builds, runs the render tests and checks that the window opens is:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\check.ps1
```

The Magic template data is copied from the M15 Magic Pack by `tools\make-slim-data.ps1`; the UniVersus templates live in `templates\`. See `CLAUDE.md` for the details.
